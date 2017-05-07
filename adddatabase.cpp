#include "adddatabase.h"
#include "ui_adddatabase.h"

#include <QtSql>
#include <QtGui>

AddDatabase::AddDatabase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddDatabase)
{
    ui->setupUi(this);
    checked = false;
    QObject::connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

AddDatabase::~AddDatabase()
{
    delete ui;
}

void AddDatabase::on_checkButton_clicked()
{
    if(checked){
        QSettings settings("Infoservice", "AgroChemLab");
        settings.beginGroup("Databases");
            settings.beginGroup(ui->NameTxt->text());
            settings.setValue("Server", ui->HostTxt->text());
            settings.endGroup();
        settings.endGroup();
        this->close();
    }
    else{
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "dbcheck");
        if(ui->NameTxt->text()==""){
            QMessageBox::warning(this, tr("Check database"), tr("Database name can not be empty!"));
            return;
        }
        db.setHostName(ui->HostTxt->text());
        db.setDatabaseName(ui->NameTxt->text());
        db.setUserName("root");
        db.setPassword("root");

        if(db.open()){

            checked = true;
            ui->checkButton->setText(tr("Add"));
            QMessageBox::information(this, tr("Check database"), tr("Connection to database %1 succesfully established").arg(ui->NameTxt->text()));
        }
        else{
            QMessageBox::warning(this, tr("Check database"), tr("Can not establish connection to: %1").arg(ui->NameTxt->text()));
            QMessageBox::warning(this, tr("Check database"), tr("Error: %1").arg(db.lastError().databaseText()));
        }
    }
    QSqlDatabase::removeDatabase("dbcheck");

}

void AddDatabase::on_NameTxt_textChanged(const QString &arg1)
{
    ui->checkButton->setText(tr("Check"));
    checked = false;
}
