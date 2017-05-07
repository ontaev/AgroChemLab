#ifndef GRANULLIST_H
#define GRANULLIST_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class GranulList;
}

class GranulList : public QDialog
{
    Q_OBJECT
    
public:
    explicit GranulList(QWidget *parent = 0);
    ~GranulList();
    
private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::GranulList *ui;
    QSqlRelationalTableModel *model;
};

#endif // GRANULLIST_H
