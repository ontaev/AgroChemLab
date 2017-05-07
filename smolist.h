#ifndef SMOLIST_H
#define SMOLIST_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class SMOList;
}

class SMOList : public QDialog
{
    Q_OBJECT
    
public:
    explicit SMOList(QWidget *parent = 0);
    ~SMOList();
    
private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::SMOList *ui;
    QSqlRelationalTableModel *model;
};

#endif // SMOLIST_H
