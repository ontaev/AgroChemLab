#ifndef REGIONLIST_H
#define REGIONLIST_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class RegionList;
}

class RegionList : public QDialog
{
    Q_OBJECT
    
public:
    explicit RegionList(QWidget *parent = 0);
    ~RegionList();
    
private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::RegionList *ui;
    QSqlRelationalTableModel *model;
};

#endif // REGIONLIST_H
