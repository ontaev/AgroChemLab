#ifndef FILTER_H
#define FILTER_H

#include <QDialog>
#include <QMap>
#include <QtSql>

namespace Ui {
class Filter;
}

class Filter : public QDialog
{
    Q_OBJECT
    
public:
    explicit Filter(QWidget *parent = 0, QString str = "", QString sql_str = "");
    ~Filter();

signals:
    void set_filter(QString filter_string, QString filter_sql_string);
    
private slots:

    void on_exprButton_clicked();

    void on_separButton_clicked();

    void on_poleCombo_currentIndexChanged(int index);

    void on_applyButton_clicked();

private:
    Ui::Filter *ui;
    QString filter_str;
    QString filter_sql_str;
    QMap<int, QString> poleMapList;
    QSqlTableModel *comboModel;

};

#endif // FILTER_H
