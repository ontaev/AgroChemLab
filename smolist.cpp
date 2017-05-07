#include "smolist.h"
#include "ui_smolist.h"
#include "noeditabledelegate.h"
#include <QtGui>

SMOList::SMOList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SMOList)
{
    ui->setupUi(this);
    model = new QSqlRelationalTableModel(ui->tableView);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->setTable("smo");
    model->setRelation(model->fieldIndex("region_id"), QSqlRelation("regions", "id_region", "region"));

    model->select();      

    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));
    ui->tableView->setItemDelegateForColumn(model->fieldIndex("id_smo"), new NoEditableDelegate (this));
    ui->tableView->setItemDelegateForRow(0, new NoEditableDelegate (this));
}

SMOList::~SMOList()
{
    delete ui;
    delete model;
}

void SMOList::on_addButton_clicked()
{
    QSqlQuery query;
    query.prepare("INSERT INTO smo (region_id) VALUES (1);");
    if(!query.exec()){
        QMessageBox::warning(this, tr("Smo table"),tr("The database reported an error: %1").arg(query.lastError().databaseText()));
    }
    model->select();
}

void SMOList::on_deleteButton_clicked()
{
    if (QMessageBox::question(this, tr("Removing data"), tr("Data will be removed permanently. Continue?"),
    QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No)    return;

    //Deleting from "smo" table

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
