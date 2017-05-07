#ifndef PREVIEWFORM_H
#define PREVIEWFORM_H

#include <QWidget>

namespace Ui {
class PreviewForm;
}

class PreviewForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit PreviewForm(QWidget *parent = 0, QString string = QString());
    ~PreviewForm();
    
private slots:
    void on_saveButton_clicked();

    void on_printButton_clicked();

private:
    Ui::PreviewForm *ui;
    QString data;
};

#endif // PREVIEWFORM_H
