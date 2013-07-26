#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QString>
#include <QtSql>
#include <QMessageBox>

QString getLanguage();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString lang = getLanguage();
    QTranslator translate;
    translate.load(":/lang/translate_"+lang);
    a.installTranslator(&translate);
    MainWindow w;
    w.show();
    w.setLang(lang);
    
    return a.exec();
}

QString getLanguage(){
    QString res = "";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "local");
    db.setDatabaseName("settings.sqlite");
    bool open = db.open();
    if(open){
        QSqlQuery query = db.exec("select value from 'settings' where property = 'lang'");
        if(query.next()) res = query.value(0).toString();
    }else{
        QMessageBox::critical(NULL, "Error!", "Can not connect to local database!");
        db.close();
        QSqlDatabase::removeDatabase("local");
        quick_exit(0);
    }
    return res;
}
