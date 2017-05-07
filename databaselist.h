#ifndef DATABASELIST_H
#define DATABASELIST_H

#include <QDialog>
#include <QtGui>

namespace Ui {
class DatabaseList;
}

class DatabaseList : public QDialog
{
    Q_OBJECT
    
public:
    explicit DatabaseList(QWidget *parent = 0);
    ~DatabaseList();

private slots:
    void on_tableWidget_doubleClicked(const QModelIndex &index);

    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::DatabaseList *ui;
    void updateList();
};

#endif // DATABASELIST_H
