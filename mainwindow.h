#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class crossword;

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
    crossword *my_crossword, *changes_cr, *crosswords_cr;

    int editor_status, current_operation;

    void writeLog(QString str);
    void writeLogError(QString str);
    void crateTables();
    void connect_to_tb_changes();
    void connect_to_tb_crosswords();
    void add_context_menu();

    enum {
        CREATE, EDIT, REMOVE,
        NOTHING, CREATE_NEW, EDIT_NEW, EDIT_EXIST
    };

public slots:
    void slotLangChanged(QAction *action);
private slots:
    void on_pushButton_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_2_clicked();
    void slotSelectedChChanged();
    void slotSelectedCrChanged();
    void slotCrosswordsRemove();
    void slotCrosswordsEdit();
    void slotChangesEdit();
    void slotChangesRemove();
    void slotChangesRemoveAll();
};

#endif // MAINWINDOW_H
