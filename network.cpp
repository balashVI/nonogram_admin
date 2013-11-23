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

    QNetworkAccessManager *net_manager = new QNetworkAccessManager(this);
    connect(net_manager, SIGNAL(finished(QNetworkReply*)), SLOT(remout_db_finish(QNetworkReply*)));
    QByteArray postData;
    postData.append("mode=export&");
    postData.append("json_str="+QJsonDocument(res_array).toJson());
    QNetworkRequest request(server_api);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    net_manager->post(request, postData);
}

void MainWindow::remout_db_finish(QNetworkReply *reply)
{
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
            if(errors) writeLogError(tr("When updating a remote database errors occurred."));
            else writeLog(tr("Remote database has been successfully updated."));
        }
    }
    else writeLogError(reply->errorString());
}
