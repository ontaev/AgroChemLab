#ifndef ADDDATABASE_H
#define ADDDATABASE_H

#include <QDialog>

namespace Ui {
class AddDatabase;
}

class AddDatabase : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddDatabase(QWidget *parent = 0);
    ~AddDatabase();

private slots:

    void on_checkButton_clicked();

    void on_NameTxt_textChanged(const QString &arg1);

private:
    Ui::AddDatabase *ui;
    bool checked;
};

#endif // ADDDATABASE_H
