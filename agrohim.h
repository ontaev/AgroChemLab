#ifndef AGROHIM_H
#define AGROHIM_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class AgroHim;
}

class AgroHim : public QDialog
{
    Q_OBJECT
    
public:
    explicit AgroHim(QWidget *parent = 0);
    ~AgroHim();
    
private slots:
    void on_calcButton_clicked();

private:
    Ui::AgroHim *ui;
    QSqlTableModel *smoModel;
};

#endif // AGROHIM_H
