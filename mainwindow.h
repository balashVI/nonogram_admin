#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setLang(QString lang);
    
private:
    Ui::MainWindow *ui;
    void writeLog(QString str);
    void writeLogError(QString str);
    void crateTables();

public slots:
    void slotLangChanged(QAction *action);
};

#endif // MAINWINDOW_H
