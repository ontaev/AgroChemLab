#include "kulturalist.h"
#include "ui_kulturalist.h"
#include "noeditabledelegate.h"
#include <QtGui>

KulturaList::KulturaList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KulturaList)
{
    ui->setupUi(this);
    model = new QSqlRelationalTableModel(ui->tableView);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->setTable("kultura");
    model->select();
    ui->tableView->setModel(model);
    ui->tableView->setItemDelegateForColumn(model->fieldIndex("id_kultura"), new NoEditableDelegate (this));
    ui->tableView->setItemDelegateForRow(0, new NoEditableDelegate (this));
}

KulturaList::~KulturaList()
{
    delete ui;
    delete model;
}

void KulturaList::on_addButton_clicked()
{
    QSqlQuery query;
    query.prepare("INSERT INTO kultura (name) VALUES ('');");
    if(!query.exec()){
        QMessageBox::warning(this, tr("Kultura table"),tr("The database reported an error: %1").arg(query.lastError().databaseText()));
    }
    model->select();
}

void KulturaList::on_deleteButton_clicked()
{
    if (QMessageBox::question(this, tr("Removing data"), tr("Data will be removed permanently. Continue?"),
    QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No)    return;

    //Deleting from "kultura" table

    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();

    int start = indexes[0].row();
    int count =  indexes.count();

    if (count > 1){
        QMessageBox::warning(this, tr("Removing data"), tr("Can not remove more than 1 row! "));
        return;
    } else {
        int id = model->data(model->index(indexes[0].row(),0)).toInt();
        if (id == 0){
            QMessageBox::warning(this, tr("Removing data"), tr("Can not remove first row! "));
            return;
        }
        model->removeRows(start, 1);
    }

    //Commit to database
    model->database().transaction();
    if (model->submitAll()){
        model->database().commit();
    }
    else{
        model->database().rollback();
        QMessageBox::warning(this, tr("Removing data"),
                             tr("The database reported an error: %1")
                             .arg(model->lastError().text()));
    }
}
