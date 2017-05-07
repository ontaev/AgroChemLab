#ifndef OWNERLIST_H
#define OWNERLIST_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class OwnerList;
}

class OwnerList : public QDialog
{
    Q_OBJECT
    
public:
    explicit OwnerList(QWidget *parent = 0);
    ~OwnerList();
    
private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::OwnerList *ui;
    QSqlRelationalTableModel *model;
};

#endif // OWNERLIST_H
