#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtSql>
#include <QMessageBox>
#include <QLabel>
#include "crossword.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->log->document()->setHtml("["+QTime::currentTime().toString("hh:mm:ss.zzz")+tr("] Program is running"));

    my_crossword = new crossword();
    ui->scrollArea_crossword->setWidget(my_crossword);
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), my_crossword, SLOT(slot_cell_size(int)));

    changes_cr = new crossword();
    changes_cr->set_can_edit(false);
    ui->scrollArea_changes_cr->setWidget(changes_cr);
    changes_cr->slot_cell_size(ui->horizontalSlider_changes->value());
    connect(ui->horizontalSlider_changes, SIGNAL(valueChanged(int)), changes_cr, SLOT(slot_cell_size(int)));

    crosswords_cr = new crossword();
    crosswords_cr->set_can_edit(false);
    ui->scrollArea_crosswords_cr->setWidget(crosswords_cr);
    crosswords_cr->slot_cell_size(ui->horizontalSlider_changes->value());
    connect(ui->horizontalSlider_crosswords, SIGNAL(valueChanged(int)), crosswords_cr, SLOT(slot_cell_size(int)));

    editor_status = NOTHING;
    crateTables();
}

MainWindow::~MainWindow()
{
    delete my_crossword;
    delete changes_cr;
    delete crosswords_cr;
    delete ui;
}

void MainWindow::setLang(QString lang)
{
    QActionGroup *alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(ui->actionEn);
    alignmentGroup->addAction(ui->actionRu);
    alignmentGroup->addAction(ui->actionUa);
    if(lang=="ua") ui->actionUa->setChecked(true);
    else if(lang=="ru") ui->actionRu->setChecked(true);
    connect(alignmentGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotLangChanged(QAction*)));
}

void MainWindow::writeLog(QString str)
{
    ui->log->document()->setHtml(ui->log->document()->toHtml()+"<p>["+QTime::currentTime().toString("hh:mm:ss.zzz")+"] "+str+"</p>");
}

void MainWindow::writeLogError(QString str)
{
    ui->log->document()->setHtml(ui->log->document()->toHtml()+"<p style=\"color:Red\">["
                                 +QTime::currentTime().toString("hh:mm:ss.zzz")+"] "
                                 +tr("Error querying database:<br/>\"")+str+"\"</p>");
    ui->toolBox->setCurrentIndex(3);
}

void MainWindow::crateTables()
{
    QSqlDatabase db = QSqlDatabase::database("local");
    QSqlQuery query = db.exec("CREATE TABLE IF NOT EXISTS `settings` ("
                              "`property` text PRIMARY KEY,"
                              "`value` text NOT NULL"
                              ")");
    if(!query.isActive()) writeLogError(query.lastError().text());
    query = db.exec("CREATE TABLE IF NOT EXISTS `changes` ("
                    "'id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "'date' text,"
                    "'type' int,"
                    "'name' text,"
                    "'crossword' text,"
                    "'width' int,"
                    "'height' int,"
                    "'cr_number' int"
                    ")");
    if(!query.isActive()) writeLogError(query.lastError().text());
    query = db.exec("CREATE TABLE IF NOT EXISTS `crosswords` ("
                    "'cr_number' INTEGER PRIMARY KEY NOT NULL,"
                    "'crossword' text,"
                    "'width' int,"
                    "'height' int,"
                    "'edited' int"
                    ")");
    if(!query.isActive()) writeLogError(query.lastError().text());
    query = db.exec("INSERT OR IGNORE INTO `settings` ("
                    "'property', 'value') VALUES ("
                    "'lang', 'en')");
    if(!query.isActive()) writeLogError(query.lastError().text());

    connect_to_tb_changes();
    connect_to_tb_crosswords();
}

void MainWindow::connect_to_tb_changes()
{
    QSqlDatabase db = QSqlDatabase::database("local");
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery("select id, date, name, cr_number from changes", db);
    model->setHeaderData(0, Qt::Horizontal, tr("Date"));
    model->setHeaderData(1, Qt::Horizontal, tr("Operation"));
    model->setHeaderData(2, Qt::Horizontal, tr("Crossword"));
    ui->tableView_changes->setModel(model);
    ui->tableView_changes->setColumnWidth(0, 0);
    this->connect(ui->tableView_changes->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                  this, SLOT(slotSelectedChChanged()));
    slotSelectedChChanged();
}

void MainWindow::connect_to_tb_crosswords()
{
    QSqlDatabase db = QSqlDatabase::database("local");
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery("SELECT cr_number, width, height FROM crosswords WHERE NOT edited = 1", db);
    model->setHeaderData(0, Qt::Horizontal, tr("Crossword"));
    model->setHeaderData(1, Qt::Horizontal, tr("Width"));
    model->setHeaderData(2, Qt::Horizontal, tr("Height"));
    ui->tableView_crosswords->setModel(model);
    this->connect(ui->tableView_crosswords->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                  this, SLOT(slotSelectedCrChanged()));
    slotSelectedCrChanged();
}

void MainWindow::slotLangChanged(QAction *action)
{
    QString lang;
    if(action->text() == "English") lang="en";
    else if (action->text() == "Русский") lang="ru";
    else lang="ua";
    QSqlQuery query = QSqlDatabase::database("local").exec("update settings set value='"+lang+"' where property='lang'");
    if(query.isActive()){
        QMessageBox::about(this, "Nonogram Admin", tr("The language change will take effect after a restart of program."));
        writeLog(tr("Language successfully changed"));
    }else{
        writeLogError(query.lastError().text());
    }
}

void MainWindow::on_pushButton_clicked()
{
    if(editor_status == NOTHING){
        ui->horizontalSlider->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_3->setEnabled(true);
    }
    ui->label_editor_status->setText(tr("Creating a new crossword."));
    ui->pushButton_2->setText(tr("Clear"));
    my_crossword->setSize(ui->spinBox_width->value(), ui->spinBox_height->value());
    editor_status = CREATE_NEW;
    current_operation = -1;
}

void MainWindow::on_pushButton_3_clicked()
{
    QSqlDatabase db = QSqlDatabase::database("local");
    QSqlQuery query(db);
    if(editor_status == CREATE_NEW){
        query.prepare("insert into 'changes' ('date', 'type', 'name', 'crossword', 'width', 'height')"
                      "values (?, ?, ?, ?, ?, ?)");
        query.addBindValue(QDate::currentDate().toString("dd.MM.yyyy"));
        query.addBindValue(CREATE);
        query.addBindValue(tr("Create"));
        query.addBindValue(my_crossword->get_crossword());
        query.addBindValue(my_crossword->get_width());
        query.addBindValue(my_crossword->get_height());
        query.exec();
        if(!query.isActive()) writeLogError(query.lastError().text());
        else {
            ui->toolBox->setCurrentIndex(0);
            on_pushButton_2_clicked();
            connect_to_tb_changes();
            writeLog(tr("Crossword was created"));
        }
    } else if(editor_status == EDIT_NEW){
        query.prepare("update 'changes' set 'date'=?, 'crossword'=? where 'id'=?");
        query.addBindValue(QDate::currentDate().toString("dd.MM.yyyy"));
        query.addBindValue(my_crossword->get_crossword());
        query.addBindValue(current_operation);
        query.exec();
        if(!query.isActive()) writeLogError(query.lastError().text());
        else {
            ui->toolBox->setCurrentIndex(0);
            on_pushButton_2_clicked();
            connect_to_tb_changes();
            writeLog(tr("Crossword was edited"));
            current_operation = -1;
        }
    } else if(editor_status == EDIT_EXIST){
        query.prepare("insert into 'changes' ('date', 'type', 'name', 'crossword', 'width', 'height', 'cr_number')"
                      "values (?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(QDate::currentDate().toString("dd.MM.yyyy"));
        query.addBindValue(EDIT);
        query.addBindValue(tr("Edit"));
        query.addBindValue(my_crossword->get_crossword());
        query.addBindValue(my_crossword->get_width());
        query.addBindValue(my_crossword->get_height());
        query.addBindValue(current_operation);
        query.exec();
        if(!query.isActive()) writeLogError(query.lastError().text());
        else {
            query.prepare("update 'crosswords' set 'edited'=? where 'cr_number'=?");
            query.addBindValue(1);
            query.addBindValue(current_operation);
            query.exec();
            if(!query.isActive()) writeLogError(query.lastError().text());
            else {
                ui->toolBox->setCurrentIndex(0);
                on_pushButton_2_clicked();
                connect_to_tb_changes();
                writeLog(tr("Crossword was edited"));
                current_operation = -1;
            }
        }
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->label_editor_status->setText(tr("Creating a new crossword."));
    ui->pushButton_2->setText(tr("Clear"));
    my_crossword->setSize(my_crossword->get_width(), my_crossword->get_height());
    editor_status = CREATE_NEW;
    current_operation = -1;
}

void MainWindow::slotSelectedChChanged()
{
    if(ui->tableView_changes->selectionModel()->selectedRows().size()==0){
        ui->horizontalSlider_changes->setEnabled(false);
        changes_cr->clear();
    } else {
        QSqlQuery query = QSqlDatabase::database("local").exec("select type, crossword, width, height, cr_number from"
                                                               " changes where id=" + ui->tableView_changes->selectionModel()->selectedRows().at(0).data().toString());
        if(!query.isActive()){
            writeLogError(query.lastError().text());
            return;
        } else if (query.next()){
            int mode = query.value(0).toInt();
            if (mode == CREATE || mode == EDIT){
                changes_cr->setCrossword(query.value(2).toInt(), query.value(3).toInt(), query.value(1).toString());
                ui->horizontalSlider_changes->setEnabled(true);
            } else if (mode == REMOVE){
                query = QSqlDatabase::database("local").exec("select crossword, width, height from crosswords"
                                                             " where cr_number=" + query.value(4).toString());
                if(!query.isActive()){
                    writeLogError(query.lastError().text());
                    return;
                } else if (query.next()){
                    changes_cr->setCrossword(query.value(1).toInt(), query.value(2).toInt(), query.value(0).toString());
                    ui->horizontalSlider_changes->setEnabled(true);
                }
            }
        }
    }
}

void MainWindow::slotSelectedCrChanged()
{
    if(ui->tableView_crosswords->selectionModel()->selectedRows().size()==0){
        ui->horizontalSlider_crosswords->setEnabled(false);
        crosswords_cr->clear();
    } else {
        QSqlQuery query = QSqlDatabase::database("local").exec("select crossword, width, height from crosswords"
                                                               " where cr_number=" + ui->tableView_crosswords->selectionModel()->selectedRows().at(0).data().toString());
        if(!query.isActive()){
            writeLogError(query.lastError().text());
            return;
        } else if (query.next()){
            crosswords_cr->setCrossword(query.value(1).toInt(), query.value(2).toInt(), query.value(0).toString());
            ui->horizontalSlider_crosswords->setEnabled(true);
        }
    }
}
