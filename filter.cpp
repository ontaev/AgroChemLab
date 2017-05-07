#include "filter.h"
#include "ui_filter.h"

Filter::Filter(QWidget *parent, QString str, QString sql_str) :
    QDialog(parent),
    ui(new Ui::Filter)
{
    ui->setupUi(this);
    QObject::connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    filter_str = str;
    filter_sql_str = sql_str;
    ui->textEdit->setText(filter_str);
    ui->textEdit_2->setText(filter_sql_str);

    comboModel = new QSqlTableModel(this);

    poleMapList.insert(1, "zemlepolz");
    poleMapList.insert(2, "smo");
    poleMapList.insert(3, "tip");
    poleMapList.insert(4, "nomer");
    poleMapList.insert(5, "s_polya");
    poleMapList.insert(6, "kultura");
    poleMapList.insert(7, "nomer_rab_uch");
    poleMapList.insert(8, "s_rab_uch");
    poleMapList.insert(9, "s_pochv_kont");
    poleMapList.insert(10, "index_p");
    poleMapList.insert(11, "granul");
    poleMapList.insert(12, "zasor");
    poleMapList.insert(13, "gamma");
    poleMapList.insert(14, "N");
    poleMapList.insert(15, "P");
    poleMapList.insert(16, "K");
    poleMapList.insert(17, "Org");
    poleMapList.insert(18, "S");
    poleMapList.insert(19, "ph");
    poleMapList.insert(20, "Ca");
    poleMapList.insert(21, "Mg");
    poleMapList.insert(22, "Na");
    poleMapList.insert(23, "Reko");
    poleMapList.insert(24, "Cd");
    poleMapList.insert(25, "Pb");
    poleMapList.insert(26, "As");
    poleMapList.insert(27, "Hg");
    poleMapList.insert(28, "Zn");
    poleMapList.insert(29, "Cu");
    poleMapList.insert(30, "Mo");
    poleMapList.insert(31, "B");

}

Filter::~Filter()
{
    delete ui;
}

void Filter::on_exprButton_clicked()
{
    if((ui->poleCombo->currentIndex()!= 0)&&(ui->exprCombo->currentIndex()!= 0)){

        switch (ui->poleCombo->currentIndex()){
            case 1:
            case 2:
            case 3:
            case 6:
            case 10:
            case 11:
                filter_str.append(" "+ui->poleCombo->currentText()+" "+ui->exprCombo->currentText()+" '"+ui->valueCombo->currentText()+"' ");
                ui->textEdit->setText(filter_str);
                filter_sql_str.append(" "+poleMapList.value(ui->poleCombo->currentIndex())+" "+ui->exprCombo->currentText()+" "+comboModel->data(comboModel->index(ui->valueCombo->currentIndex(),0)).toString());
                ui->textEdit_2->setText(filter_sql_str);
            break;
            case 4:
                filter_str.append(" "+ui->poleCombo->currentText()+" "+ui->exprCombo->currentText()+" '"+ui->valueCombo->currentText()+"' ");
                ui->textEdit->setText(filter_str);
                filter_sql_str.append(" "+poleMapList.value(ui->poleCombo->currentIndex())+" "+ui->exprCombo->currentText()+" '"+ui->valueCombo->currentText()+"' ");
                ui->textEdit_2->setText(filter_sql_str);
            break;
            default:
                filter_str.append(" "+ui->poleCombo->currentText()+" "+ui->exprCombo->currentText()+" "+ui->valueCombo->currentText());
                ui->textEdit->setText(filter_str);
                filter_sql_str.append(" "+poleMapList.value(ui->poleCombo->currentIndex())+" "+ui->exprCombo->currentText()+" "+ui->valueCombo->currentText());
                ui->textEdit_2->setText(filter_sql_str);
            break;
        }
    }
}

void Filter::on_separButton_clicked()
{
    filter_str.append(" "+ui->separCombo->currentText());
    filter_sql_str.append(" "+ui->separCombo->currentText());
    ui->textEdit->setText(filter_str);
    ui->textEdit_2->setText(filter_sql_str);

}

void Filter::on_poleCombo_currentIndexChanged(int index)
{
    switch (index){

        case 1:
            comboModel->setTable("owner");
            comboModel->select();
            ui->valueCombo->setModel(comboModel);
            ui->valueCombo->setModelColumn(comboModel->fieldIndex("owner"));
            ui->valueCombo->setCurrentIndex(0);
            break;
        case 2:
            comboModel->setTable("smo");
            comboModel->select();
            ui->valueCombo->setModel(comboModel);
            ui->valueCombo->setModelColumn(comboModel->fieldIndex("name"));
            ui->valueCombo->setCurrentIndex(0);
            break;
        case 3:
            comboModel->setTable("tip");
            comboModel->select();
            ui->valueCombo->setModel(comboModel);
            ui->valueCombo->setModelColumn(comboModel->fieldIndex("name"));
            ui->valueCombo->setCurrentIndex(0);
            break;
        case 6:
            comboModel->setTable("kultura");
            comboModel->select();
            ui->valueCombo->setModel(comboModel);
            ui->valueCombo->setModelColumn(comboModel->fieldIndex("name"));
            ui->valueCombo->setCurrentIndex(0);
            break;
        case 10:
            comboModel->setTable("indexes");
            comboModel->select();
            ui->valueCombo->setModel(comboModel);
            ui->valueCombo->setModelColumn(comboModel->fieldIndex("name"));
            ui->valueCombo->setCurrentIndex(0);
            break;
        case 11:
            comboModel->setTable("granul");
            comboModel->select();
            ui->valueCombo->setModel(comboModel);
            ui->valueCombo->setModelColumn(comboModel->fieldIndex("name"));
            ui->valueCombo->setCurrentIndex(0);
            break;
        default:
            comboModel->setTable("emptytable");
            comboModel->select();
            ui->valueCombo->setModel(comboModel);
            ui->valueCombo->setModelColumn(comboModel->fieldIndex("emptycolumn"));
            break;
    }
    return;

}

void Filter::on_applyButton_clicked()
{
    QString text = ui->textEdit_2->toPlainText();
    emit(set_filter(filter_str, text));
    close();
}
