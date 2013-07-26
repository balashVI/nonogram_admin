#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtSql>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->log->document()->setHtml("["+QTime::currentTime().toString("hh:mm:ss.zzz")+tr("] Program is running"));
    crateTables();
}

MainWindow::~MainWindow()
{
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
    ui->log->document()->setHtml(ui->log->document()->toHtml()+"["+QTime::currentTime().toString("hh:mm:ss.zzz")+"] "+str);
}

void MainWindow::writeLogError(QString str)
{
    ui->log->document()->setHtml(ui->log->document()->toHtml()+"<p style=\"color:Red\">["
                                 +QTime::currentTime().toString("hh:mm:ss.zzz")+"] "+str+"</p>");
}

void MainWindow::crateTables()
{
    QSqlDatabase db = QSqlDatabase::database("local");
    QSqlQuery query = db.exec("CREATE TABLE IF NOT EXISTS `settings` ("
                              "`property` text PRIMARY KEY,"
                              "`value` text NOT NULL"
                              ")");
    if(!query.isActive()) writeLogError(tr("Error querying database. Error:<br/>")+query.lastError().text());
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
    if(!query.isActive()) writeLogError(tr("Error querying database. Error:<br/>")+query.lastError().text());
    query = db.exec("CREATE TABLE IF NOT EXISTS `crosswords` ("
                    "'id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "'cr_number' int,"
                    "'crossword' text,"
                    "'width' int,"
                    "'height' int"
                    ")");
    if(!query.isActive()) writeLogError(tr("Error querying database. Error:<br/>")+query.lastError().text());

    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery("select date, name, cr_number from changes", db);
    model->setHeaderData(0, Qt::Horizontal, tr("Date"));
    model->setHeaderData(1, Qt::Horizontal, tr("Operation"));
    model->setHeaderData(2, Qt::Horizontal, tr("Crossword"));
    ui->tableView_changes->setModel(model);
    model = new QSqlQueryModel(this);
    model->setQuery("select cr_number, width, height from crosswords", db);
    model->setHeaderData(0, Qt::Horizontal, tr("Crossword"));
    model->setHeaderData(1, Qt::Horizontal, tr("Width"));
    model->setHeaderData(2, Qt::Horizontal, tr("Height"));
    ui->tableView_crosswords->setModel(model);
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
        writeLogError(tr("Failed to change the language. Error:<br/>")+query.lastError().text());
    }
}
