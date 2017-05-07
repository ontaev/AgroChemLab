#include "previewform.h"
#include "ui_previewform.h"
#include <QTextCodec>
#include <QFileDialog>
#include <QTextStream>
#include <QIODevice>

PreviewForm::PreviewForm(QWidget *parent, QString string) :
    QWidget(parent),
    ui(new Ui::PreviewForm)
{
    ui->setupUi(this);
    data = string;

    ui->webView->setHtml(string);
    ui->webView->adjustSize();
}

PreviewForm::~PreviewForm()
{
    delete ui;
}

void PreviewForm::on_saveButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Document"), QDir::currentPath(), tr("Excel (*.xls)") );
    QFile file(filename);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    file.open(QFile::WriteOnly | QFile::Text);
    stream << data;
    file.close();
}

void PreviewForm::on_printButton_clicked()
{

}
