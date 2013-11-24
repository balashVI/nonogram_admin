#include "mainwindow.h"

#include <QJsonArray>
#include <QtSql>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrl>

void MainWindow::on_actionUpdate_remote_database_triggered()
{
    QJsonArray res_array;
    QVariantMap variant_map;

    QSqlDatabase db = QSqlDatabase::database("local");
    QSqlQuery query = db.exec("select count(*) from changes");
    if(!query.isActive()){
        writeLogError(query.lastError().text());
        return;
    } else if (query.next())
        if(query.value(0).toInt()==0){
            writeLog(tr("List of changes is empty."));
            return;
        }
    query = db.exec("select id, type, crossword, width, height, cr_number from changes");
    if(!query.isActive()){
        writeLogError(query.lastError().text());
        return;
    } else while (query.next()) {
        variant_map.clear();
        variant_map.insert("id", query.value(0));
        variant_map.insert("type", query.value(1));
        variant_map.insert("crossword", query.value(2));
        variant_map.insert("width", query.value(3));
        variant_map.insert("height", query.value(4));
        variant_map.insert("cr_number", query.value(5));
        res_array.insert(0, QJsonValue::fromVariant(variant_map));
    }

    connect(net_manager, SIGNAL(finished(QNetworkReply*)), SLOT(server_remout_db(QNetworkReply*)));
    QByteArray postData;
    postData.append("mode=export&");
    postData.append("json_str="+QJsonDocument(res_array).toJson());
    QNetworkRequest request(server_api);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    net_manager->post(request, postData);
}

void MainWindow::server_remout_db(QNetworkReply *reply)
{
    net_manager->disconnect(net_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(server_remout_db(QNetworkReply*)));
    if (reply->error() == QNetworkReply::NoError)
    {
        QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
        if(json.object().find("error").value().toString()=="true"){
            writeLogError(json.object().find("error_code").value().toString());
            return;
        } else {
            QSqlDatabase db = QSqlDatabase::database("local");
            QSqlQuery query;
            bool errors=false;
            QString error;
            QJsonArray arr = json.object().find("data").value().toArray();
            for(int i=0;i<arr.count();i++){
                error = arr.at(i).toObject().find("error").value().toString();
                if(error=="") {
                    query = db.exec("delete from changes where id='"+QString::number(arr.at(i).toObject().find("id").value().toDouble())+"'");
                    if(!query.isActive()){
                        writeLogError(query.lastError().text());
                        errors=true;
                    }
                } else {
                    errors=true;
                    writeLogError(error);
                }
            }
            connect_to_tb_changes();
            if(errors) writeLogError(tr("Errors have occurred while updating remote database."));
            else writeLog(tr("Remote database has been successfully updated."));
            on_actionUpdate_local_database_triggered();
        }
    }
    else writeLogError(reply->errorString());
}

void MainWindow::on_actionUpdate_local_database_triggered()
{
    QString db_version;
    QSqlDatabase db = QSqlDatabase::database("local");
    QSqlQuery query = db.exec("select value from settings where property='db_version'");
    if(!query.isActive()){
        writeLogError(query.lastError().text());
        return;
    } else if (query.next()) db_version = query.value(0).toString();

    connect(net_manager, SIGNAL(finished(QNetworkReply*)), SLOT(server_local_db(QNetworkReply*)));
    QByteArray postData;
    postData.append("mode=import&");
    postData.append("db_version="+db_version);
    QNetworkRequest request(server_api);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    net_manager->post(request, postData);
}

void MainWindow::server_local_db(QNetworkReply *reply)
{
    net_manager->disconnect(net_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(server_local_db(QNetworkReply*)));
    if (reply->error() == QNetworkReply::NoError)
    {
        bool errors=false;
        QByteArray readed = reply->readAll();
        if(readed==""){
            writeLog(tr("The latest database is installed on your computer."));
            return;
        }
        QJsonDocument json = QJsonDocument::fromJson(readed);
        if(json.object().find("error").value().toString()=="true"){
            writeLogError(json.object().find("error_code").value().toString());
            errors=true;
        } else {
            QSqlDatabase db = QSqlDatabase::database("local");
            QSqlQuery query = db.exec("update settings set value='"+json.object().find("db_version").value().toString()+"' where property='db_version'");
            if(!query.isActive()){
                writeLogError(query.lastError().text());
                errors=true;
            } else {
                query = db.exec("delete from crosswords");
                if(!query.isActive()){
                    writeLogError(query.lastError().text());
                    errors=true;
                } else {
                    QJsonArray arr = json.object().find("data").value().toArray();
                    QJsonObject obj;
                    for(int i=0;i<arr.count();i++){
                        obj=arr.at(i).toObject();

                        query.prepare("insert into crosswords (cr_number, crossword, width, height, edited) "
                                 "values (?, ?, ?, ?, ?)");
                        query.addBindValue(obj.find("id").value().toString());
                        query.addBindValue(obj.find("crossword").value().toString());
                        query.addBindValue(obj.find("width").value().toString());
                        query.addBindValue(obj.find("height").value().toString());
                        query.addBindValue("0");
                        query.exec();
                        if(!query.isActive()){
                            writeLogError(query.lastError().text());
                            errors=true;
                        }
                    }
                }
            }
        }
        connect_to_tb_crosswords();
        if(errors) writeLogError(tr("Errors have occurred while updating local database."));
        else writeLog(tr("Local database has been successfully updated."));
    }
    else writeLogError(reply->errorString());
}
