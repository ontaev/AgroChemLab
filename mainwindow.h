#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtSql>
#include "treemodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setStatus(QString);
    void updateTable();
    
private slots:
    void on_openDatabase_triggered();


    void on_open_SMOList_triggered();

    void on_open_OwnerList_triggered();

    void on_open_IndexList_triggered();

    void on_open_GranulList_triggered();

    void on_agroButton_clicked();

    void on_ecoButton_clicked();

    void on_addButton_clicked();

    void on_delButton_clicked();

    void on_mainTable_doubleClicked(const QModelIndex &index);

    void on_open_RegionList_triggered();

    void on_open_TipList_triggered();

    void on_open_KulturaList_triggered();

    void on_treeView_clicked(const QModelIndex &index);

    void on_open_Tables_triggered();

    void on_newDb_triggered();

    void on_filterButton_clicked();

    void on_resetButton_clicked();

    void set_filter(QString str, QString sql_str);

    void on_agroHim_triggered();

    void on_raschUdob_triggered();

private:
    Ui::MainWindow *ui;
    QSqlRelationalTableModel *model;
    TreeModel *treeModel;
    QLabel *status;
    QString filter_str;
    QString filter_sql_str;
    QString smo_filter;
};

#endif // MAINWINDOW_H
