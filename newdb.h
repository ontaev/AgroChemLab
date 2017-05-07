#ifndef NEWDB_H
#define NEWDB_H

#include <QDialog>

namespace Ui {
class NewDb;
}

class NewDb : public QDialog
{
    Q_OBJECT
    
public:
    explicit NewDb(QWidget *parent = 0);
    ~NewDb();
    
private slots:
    void on_createButton_clicked();

private:
    Ui::NewDb *ui;
};

#endif // NEWDB_H
