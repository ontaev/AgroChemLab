#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class ReportDialog;
}

class ReportDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ReportDialog(QWidget *parent = 0);
    ~ReportDialog();
    
private slots:
    void on_pushButton_clicked();
    QString N_report(QString id);
    QString P_report(QString id);
    QString K_report(QString id);
    QString S_report(QString id);
    QString Org_report(QString id);
    QString PH_report(QString id);
    QString Ca_report(QString id);
    QString Mg_report(QString id);
    QString Zn_report(QString id);
    QString Cu_report(QString id);
    QString Mo_report(QString id);
    QString B_report(QString id);
    QString REKO_report(QString id);
    QString granul_report(QString id);
    QString Cd_report(QString id);
    QString Pb_report(QString id);
    QString Hg_report(QString id);
    QString As_report(QString id);
    QString report(QModelIndexList index, QString ids);

    QString N_report_resp(QString id);
    QString P_report_resp(QString id);
    QString K_report_resp(QString id);
    QString S_report_resp(QString id);
    QString Org_report_resp(QString id);
    QString PH_report_resp(QString id);
    QString Ca_report_resp(QString id);
    QString Mg_report_resp(QString id);
    QString Zn_report_resp(QString id);
    QString Cu_report_resp(QString id);
    QString Mo_report_resp(QString id);
    QString B_report_resp(QString id);
    QString REKO_report_resp(QString id);
    QString granul_report_resp(QString id);
    QString Cd_report_resp(QString id);
    QString Pb_report_resp(QString id);
    QString Hg_report_resp(QString id);
    QString As_report_resp(QString id);
    QString report_resp(QModelIndexList index, QString ids);

    void on_tipCombo_currentIndexChanged(int index);

private:
    Ui::ReportDialog *ui;
    QSqlTableModel *tipModel;
};

#endif // REPORTDIALOG_H
