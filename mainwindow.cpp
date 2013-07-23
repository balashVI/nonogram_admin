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
    ui->log->document()->setHtml(ui->log->document()->toHtml()+"Запуск");
    ui->log->document()->setHtml(ui->log->document()->toHtml()+"Запуск");
    ui->log->document()->setHtml(ui->log->document()->toHtml()+"Запуск");
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

void MainWindow::slotLangChanged(QAction *action)
{
    QString lang;
    if(action->text() == "English") lang="en";
    else if (action->text() == "Русский") lang="ru";
    else lang="ua";
    QSqlQuery query = QSqlDatabase::database("local").exec("update settings set value='"+lang+"' where property='lang'");
    if(query.isActive()) QMessageBox::about(this, "Nonogram Admin", tr("The language change will take effect after a restart of program."));
}
