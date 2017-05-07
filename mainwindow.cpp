#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databaselist.h"
#include "smolist.h"
#include "ownerlist.h"
#include "indexlist.h"
#include "granullist.h"
#include "regionlist.h"
#include "tiplist.h"
#include "kulturalist.h"
#include "opencard.h"
#include "reportdialog.h"
#include "newdb.h"
#include "filter.h"
#include "raschudob.h"
#include "agrohim.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    status = new QLabel(this);
    filter_str = "";
    filter_sql_str = "";
    smo_filter = "";
    setStatus("Ready");
    connect(ui->exitApplication, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setStatus(QString message){
    status->setText(message);
    ui->statusBar->addWidget(status);
}

void MainWindow::updateTable(){

    model = new QSqlRelationalTableModel();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setTable("main");


    ui->mainTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->mainTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    int smo, zemlepolz, tip, kultura, index, granul;

    smo = model->fieldIndex("smo");
    zemlepolz = model->fieldIndex("zemlepolz");
    tip = model->fieldIndex("tip");
    kultura = model->fieldIndex("kultura");
    index = model->fieldIndex("index_p");
    granul = model->fieldIndex("granul");

    model->setHeaderData(smo, Qt::Horizontal, tr("smo"));
    model->setHeaderData(zemlepolz, Qt::Horizontal, tr("owner"));
    model->setHeaderData(tip, Qt::Horizontal, tr("tip"));
    model->setHeaderData(kultura, Qt::Horizontal, tr("kultura"));
    model->setHeaderData(index, Qt::Horizontal, tr("index"));
    model->setHeaderData(granul, Qt::Horizontal, tr("granul"));

    model->setHeaderData(model->fieldIndex("nomer"), Qt::Horizontal, tr("n_polya"));
    model->setHeaderData(model->fieldIndex("s_polya"), Qt::Horizontal, tr("s_polya"));
    model->setHeaderData(model->fieldIndex("nomer_rab_uch"), Qt::Horizontal, tr("n_rab_uch"));
    model->setHeaderData(model->fieldIndex("s_rab_uch"), Qt::Horizontal, tr("s_rab_uch"));
    model->setHeaderData(model->fieldIndex("s_pochv_kont"), Qt::Horizontal, tr("s_pochv_kont"));
    model->setHeaderData(model->fieldIndex("zasor"), Qt::Horizontal, tr("zasor"));
    model->setHeaderData(model->fieldIndex("gamma"), Qt::Horizontal, tr("gamma"));
    model->setHeaderData(model->fieldIndex("Reko"), Qt::Horizontal, tr("Reko"));

    model->setRelation(smo, QSqlRelation("smo", "id_smo", "name"));
    model->setRelation(zemlepolz, QSqlRelation("owner", "id_owner", "owner"));
    model->setRelation(tip, QSqlRelation("tip", "id_tip", "name"));
    model->setRelation(kultura, QSqlRelation("kultura", "id_kultura", "name"));
    model->setRelation(index, QSqlRelation("indexes", "id_index", "name"));
    model->setRelation(granul, QSqlRelation("granul", "id_granul", "name"));

    //model->setFilter();

    model->select();
//    qDebug() << model->rowCount();
//    model->fetchMore();
//    qDebug() << model->canFetchMore();
//    while(model->canFetchMore()){
//        model->fetchMore();
//        QCoreApplication::processEvents();
//    }
    ui->mainTable->setModel(model);
    ui->mainTable->resizeColumnsToContents();

    ui->mainTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);


    ui->mainTable->setColumnHidden(model->fieldIndex("N"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("P"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("K"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Org"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("S"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("ph"), true);

    ui->mainTable->setColumnHidden(model->fieldIndex("Ca"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Mg"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Na"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Reko"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Cd"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Pb"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("As"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Hg"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Zn"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Cu"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("Mo"), true);
    ui->mainTable->setColumnHidden(model->fieldIndex("B"), true);

    QStringList headers;
    headers << tr("Region") << tr("ID");

    treeModel = new TreeModel(headers);
    ui->treeView->setModel(treeModel);
    ui->treeView->setColumnHidden(1, true);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);


}

void MainWindow::on_openDatabase_triggered()
{
    DatabaseList dbListDlg(this);
    dbListDlg.exec();
}

void MainWindow::on_open_SMOList_triggered()
{
    SMOList SMOListDlg(this);
    SMOListDlg.exec();
}

void MainWindow::on_open_OwnerList_triggered()
{
    OwnerList OwnerListDlg(this);
    OwnerListDlg.exec();
}

void MainWindow::on_open_IndexList_triggered()
{
    IndexList IndexListDlg(this);
    IndexListDlg.exec();
}

void MainWindow::on_open_GranulList_triggered()
{
    GranulList GranulListDlg(this);
    GranulListDlg.exec();
}

void MainWindow::on_open_RegionList_triggered()
{
    RegionList RegionListDlg(this);
    RegionListDlg.exec();
}

void MainWindow::on_open_TipList_triggered()
{
    TipList TipListDlg(this);
    TipListDlg.exec();
}

void MainWindow::on_open_KulturaList_triggered()
{
    KulturaList KulturaListDlg(this);
    KulturaListDlg.exec();
}
void MainWindow::on_agroButton_clicked()
{
    if(status->text()=="Ready") return;
    if (ui->agroButton->text()=="Agro >>"){
        ui->mainTable->setColumnHidden(model->fieldIndex("N"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("P"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("K"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Org"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("S"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("ph"), false);
        ui->agroButton->setText("Agro <<");


    } else {
        ui->mainTable->setColumnHidden(model->fieldIndex("N"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("P"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("K"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Org"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("S"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("ph"), true);
        ui->agroButton->setText("Agro >>");
    }

}

void MainWindow::on_ecoButton_clicked()
{
    if(status->text()=="Ready") return;
    if (ui->ecoButton->text()=="Eco >>"){
        ui->mainTable->setColumnHidden(model->fieldIndex("Ca"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Mg"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Na"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Reko"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Cd"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Pb"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("As"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Hg"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Zn"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Cu"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("Mo"), false);
        ui->mainTable->setColumnHidden(model->fieldIndex("B"), false);
        ui->ecoButton->setText("Eco <<");


    } else {
        ui->mainTable->setColumnHidden(model->fieldIndex("Ca"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Mg"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Na"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Reko"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Cd"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Pb"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("As"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Hg"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Zn"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Cu"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("Mo"), true);
        ui->mainTable->setColumnHidden(model->fieldIndex("B"), true);
        ui->ecoButton->setText("Eco >>");
    }
}

void MainWindow::on_addButton_clicked()
{
    if(status->text()=="Ready") return;
    QSqlQuery query;
    query.prepare("INSERT INTO main (zemlepolz, smo, tip, kultura, index_p, granul) VALUES (1, 1, 1, 1, 1, 1);");
    if(!query.exec()){
        QMessageBox::warning(this, tr("Main table"),tr("The database reported an error: %1").arg(query.lastError().databaseText()));

    } else {
        int new_id;
        new_id = query.lastInsertId().toInt();
        OpenCard oCard(this, new_id);
        oCard.exec();
    }
    model->select();
    ui->mainTable->resizeColumnsToContents();
}

void MainWindow::on_delButton_clicked()
{
    if(status->text()=="Ready") return;
    if (QMessageBox::question(this, tr("Removing data"), tr("Data will be removed permanently. Continue?"),
    QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No)    return;

    //Deleting from "main" table
    int count = ui->mainTable->selectionModel()->selectedRows().count();
    QModelIndexList indexes = ui->mainTable->selectionModel()->selectedRows();

    int toDel = 1;
    int start = indexes[0].row();
    count =  indexes.count();

    for (int i = 0; i < count-1; i++){
        if(indexes.at(i).row() != indexes.at(i+1).row()-1){
            model->removeRows(start, toDel);
            start = indexes.at(i+1).row();
            toDel = 1;
        }
        else{
            toDel++;
        }

    }
    model->removeRows(start, toDel);

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

void MainWindow::on_mainTable_doubleClicked(const QModelIndex &index)
{
    if(status->text()=="Ready") return;
    int id = model->data(model->index(index.row(),0)).toInt();
    OpenCard oCard (this, id);
    oCard.exec();
    model->select();
    ui->mainTable->resizeColumnsToContents();
}


void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    if(status->text()=="Ready") return;
    if(treeModel->parent(index) == QModelIndex()){
        qDebug() <<"1";
        int region = treeModel->dataColumn(index, 1).toInt();
        qDebug() << "region" << region;
        QSqlQuery query;
        QString numbers;
        query.prepare("SELECT id_smo FROM smo WHERE region_id = :reg");
        query.bindValue(":reg", region);
        if(query.exec()){
            if (query.size()>0) {
                while (query.next()){
                    numbers.append(query.value(0).toString()+",");
                }
                qDebug() << numbers;
                numbers.chop(1);
                smo_filter = QString("smo IN(%1)").arg(numbers);
                QString modelFilter;
                if (filter_sql_str != ""){
                    modelFilter = smo_filter + " AND (" + filter_sql_str + ")";
                } else {
                    modelFilter = smo_filter;
                }
                qDebug() << "Model filt " << modelFilter;
                model->setFilter(modelFilter);
                model->select();

            } else {
                smo_filter = "smo = 1";
                QString modelFilter;
                if (filter_sql_str != ""){
                    modelFilter = smo_filter + " AND (" + filter_sql_str + ")";
                } else {
                    modelFilter = smo_filter;
                }
                qDebug() << "Model filt " << modelFilter;
                model->setFilter(modelFilter);
                model->select();
            }
        }

    } else {
        qDebug() <<"2";
        smo_filter = QString("smo=%1").arg(treeModel->dataColumn(index, 1).toString());
        QString modelFilter;
        if (filter_sql_str != ""){
            modelFilter = smo_filter + " AND (" + filter_sql_str + ")";
        } else {
            modelFilter = smo_filter;
        }
        qDebug() << "Model filt " << modelFilter;
        model->setFilter(modelFilter);
        model->select();
    }
    return;
}

void MainWindow::on_open_Tables_triggered()
{
    ReportDialog RepDlg(this);
    RepDlg.exec();
}

void MainWindow::on_newDb_triggered()
{
    NewDb NewDbDlg(this);
    NewDbDlg.exec();
}

void MainWindow::on_filterButton_clicked()
{
    if(status->text()=="Ready") return;
    Filter FilterDlg(this, filter_str, filter_sql_str);
    QObject::connect (&FilterDlg, SIGNAL(set_filter(QString, QString)),this, SLOT(set_filter(QString, QString)));
    FilterDlg.exec();
}

void MainWindow::on_resetButton_clicked()
{
    if(status->text()=="Ready") return;
    model->setFilter("");
    smo_filter = "";
    filter_str = "";
    filter_sql_str = "";
    ui->treeView->clearSelection();
    model->select();
}

void MainWindow::set_filter(QString str, QString sql_str)
{
    filter_str = str;
    filter_sql_str = sql_str;

    //if (sql_str == "") return;
    QString modelFilter("");
    if (smo_filter != ""){
        modelFilter = smo_filter + " AND (" + sql_str + ")";
    } else {
        modelFilter = sql_str;
    }


    model->setFilter(modelFilter);
    qDebug() << "filt" << model->filter();
    qDebug() << "Model filterr!!! " << modelFilter << " str " << str << "sql_str  " << sql_str;
    model->select();
    return;
}

void MainWindow::on_agroHim_triggered()
{
    AgroHim AgroHimDlg(this);
    AgroHimDlg.exec();
}

void MainWindow::on_raschUdob_triggered()
{
    RaschUdob UdobrDlg(this);
    UdobrDlg.exec();
}
