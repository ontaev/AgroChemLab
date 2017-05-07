#include "indexlist.h"
#include "ui_indexlist.h"
#include "noeditabledelegate.h"
#include <QtGui>

IndexList::IndexList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndexList)
{
    ui->setupUi(this);
    model = new QSqlRelationalTableModel(ui->tableView);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->setTable("indexes");
    model->select();
    ui->tableView->setModel(model);
    ui->tableView->setItemDelegateForColumn(model->fieldIndex("id_index"), new NoEditableDelegate (this));
    ui->tableView->setItemDelegateForRow(0, new NoEditableDelegate (this));
}

IndexList::~IndexList()
{
    delete ui;
    delete model;
}


void IndexList::on_addButton_clicked()
{
    QSqlQuery query;
    query.prepare("INSERT INTO indexes (name) VALUES ('');");
    if(!query.exec()){
        QMessageBox::warning(this, tr("Index table"),tr("The database reported an error: %1").arg(query.lastError().databaseText()));
    }
    model->select();
}

void IndexList::on_deleteButton_clicked()
{
    if (QMessageBox::question(this, tr("Removing data"), tr("Data will be removed permanently. Continue?"),
    QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No)    return;

    //Deleting from "index" table

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
