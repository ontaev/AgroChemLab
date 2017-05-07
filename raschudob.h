#ifndef RASCHUDOB_H
#define RASCHUDOB_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class RaschUdob;
}

class RaschUdob : public QDialog
{
    Q_OBJECT
    
public:
    explicit RaschUdob(QWidget *parent = 0);
    ~RaschUdob();
    
private slots:
    void on_calcButton_clicked();
    QString report(QString smo_id, QString kultura_id);

private:
    Ui::RaschUdob *ui;
    QSqlTableModel *smoModel;
};

#endif // RASCHUDOB_H
