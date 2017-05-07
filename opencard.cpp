#include "opencard.h"
#include "ui_opencard.h"

OpenCard::OpenCard(QWidget *parent, int id) :
    QDialog(parent),
    ui(new Ui::OpenCard)
{
    ui->setupUi(this);
    OpenCard::id = id;
    QString str;

    //Create model and set filter
    model = new QSqlRelationalTableModel(this);
    model->setTable("main");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setFilter(QString("id=%1").arg(str.setNum(id)));


    int smo, zemlepolz, tip, kultura, index, granul;

    smo = model->fieldIndex("smo");
    zemlepolz = model->fieldIndex("zemlepolz");
    tip = model->fieldIndex("tip");
    kultura = model->fieldIndex("kultura");
    index = model->fieldIndex("index_p");
    granul = model->fieldIndex("granul");


    model->setRelation(smo, QSqlRelation("smo", "id_smo", "name"));
    model->setRelation(zemlepolz, QSqlRelation("owner", "id_owner", "owner"));
    model->setRelation(tip, QSqlRelation("tip", "id_tip", "name"));
    model->setRelation(kultura, QSqlRelation("kultura", "id_kultura", "name"));
    model->setRelation(index, QSqlRelation("indexes", "id_index", "name"));
    model->setRelation(granul, QSqlRelation("granul", "id_granul", "name"));


    QSqlTableModel *smoRelModel = model->relationModel(smo);
    ui->smoCombo->setModel(smoRelModel);
    ui->smoCombo->setModelColumn(smoRelModel->fieldIndex("name"));

    QSqlTableModel *zemlepolzRelModel = model->relationModel(zemlepolz);
    ui->zemlepolzCombo->setModel(zemlepolzRelModel);
    ui->zemlepolzCombo->setModelColumn(zemlepolzRelModel->fieldIndex("owner"));

    QSqlTableModel *tipRelModel = model->relationModel(tip);
    ui->tipCombo->setModel(tipRelModel);
    ui->tipCombo->setModelColumn(tipRelModel->fieldIndex("name"));

    QSqlTableModel *kulturaRelModel = model->relationModel(kultura);
    ui->kulturaCombo->setModel(kulturaRelModel);
    ui->kulturaCombo->setModelColumn(kulturaRelModel->fieldIndex("name"));

    QSqlTableModel *indexRelModel = model->relationModel(index);
    ui->indexCombo->setModel(indexRelModel);
    ui->indexCombo->setModelColumn(indexRelModel->fieldIndex("name"));

    QSqlTableModel *granulRelModel = model->relationModel(granul);
    ui->granulCombo->setModel(granulRelModel);
    ui->granulCombo->setModelColumn(granulRelModel->fieldIndex("name"));

    model->select();


    //Add data mapper on OpenCard form
    mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
    mapper->setItemDelegate(new QSqlRelationalDelegate(this));
    //General ui elements
    mapper->addMapping(ui->idTxt, model->fieldIndex("id"));
    mapper->addMapping(ui->n_polyaTxt, model->fieldIndex("nomer"));
    mapper->addMapping(ui->s_polyaTxt, model->fieldIndex("s_polya"));
    mapper->addMapping(ui->n_uchastTxt, model->fieldIndex("nomer_rab_uch"));
    mapper->addMapping(ui->s_uchastTxt, model->fieldIndex("s_rab_uch"));
    mapper->addMapping(ui->s_konturaTxt, model->fieldIndex("s_pochv_kont"));
    mapper->addMapping(ui->zasorTxt, model->fieldIndex("zasor"));
    mapper->addMapping(ui->gammaTxt, model->fieldIndex("gamma"));
    mapper->addMapping(ui->NTxt, model->fieldIndex("N"));
    mapper->addMapping(ui->PTxt, model->fieldIndex("P"));
    mapper->addMapping(ui->KTxt, model->fieldIndex("K"));
    mapper->addMapping(ui->OrgTxt, model->fieldIndex("Org"));
    mapper->addMapping(ui->STxt, model->fieldIndex("S"));
    mapper->addMapping(ui->pHTxt, model->fieldIndex("pH"));
    mapper->addMapping(ui->CaTxt, model->fieldIndex("Ca"));
    mapper->addMapping(ui->MgTxt, model->fieldIndex("Mg"));
    mapper->addMapping(ui->NaTxt, model->fieldIndex("Na"));
    mapper->addMapping(ui->rekoTxt, model->fieldIndex("Reko"));
    mapper->addMapping(ui->CdTxt, model->fieldIndex("Cd"));
    mapper->addMapping(ui->PbTxt, model->fieldIndex("Pb"));
    mapper->addMapping(ui->AsTxt, model->fieldIndex("As"));
    mapper->addMapping(ui->HgTxt, model->fieldIndex("Hg"));
    mapper->addMapping(ui->ZnTxt, model->fieldIndex("Zn"));
    mapper->addMapping(ui->CuTxt, model->fieldIndex("Cu"));
    mapper->addMapping(ui->MoTxt, model->fieldIndex("Mo"));
    mapper->addMapping(ui->BTxt, model->fieldIndex("B"));

    //Combobox ui elements
    mapper->addMapping(ui->smoCombo, smo);
    mapper->addMapping(ui->zemlepolzCombo, zemlepolz);
    mapper->addMapping(ui->tipCombo, tip);
    mapper->addMapping(ui->kulturaCombo, kultura);
    mapper->addMapping(ui->indexCombo, index);
    mapper->addMapping(ui->granulCombo, granul);

    mapper->setCurrentIndex(0);

    connect(ui->closeButton, SIGNAL(clicked()),this, SLOT(close()));
    connect(ui->submitButton, SIGNAL(clicked()), this, SLOT(submit()));
    connect(ui->revertButton, SIGNAL(clicked()), model, SLOT(revertAll()));
}

OpenCard::~OpenCard()
{
    delete ui;
    delete model;
    delete mapper;
}

void OpenCard::submit()
{
    model->database().transaction();
    if (model->submitAll()) {
        model->database().commit();
    } else {
        model->database().rollback();
        QMessageBox::warning(this, tr("Submit data"),
                             tr("The database reported an error: %1")
                             .arg(model->lastError().text()));
    }
}
