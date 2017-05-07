#include "reportdialog.h"
#include "ui_reportdialog.h"
//#include "previewer.h"
#include "previewform.h"


ReportDialog::ReportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportDialog)
{
    ui->setupUi(this);
    QObject::connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tipModel = new QSqlTableModel(this);
}

ReportDialog::~ReportDialog()
{
    delete ui;
}

void ReportDialog::on_pushButton_clicked()
{
    QString report_string("");
    if (ui->tipCombo->currentIndex() == 0) return;
    if (ui->tipCombo->currentIndex() == 1){
        report_string = report(ui->treeWidget->selectionModel()->selectedRows(), (tipModel->data(tipModel->index(ui->subjCombo->currentIndex(),0)).toString()));
    } else {
        report_string = report_resp(ui->treeWidget->selectionModel()->selectedRows(), (tipModel->data(tipModel->index(ui->subjCombo->currentIndex(),0)).toString()));
    }

    PreviewForm *PF = new PreviewForm (this, report_string);
    PF->setAttribute(Qt::WA_DeleteOnClose);
    PF->setWindowFlags(Qt::Window);
    PF->show();

}

void ReportDialog::on_tipCombo_currentIndexChanged(int index)
{
    if (index == 0){
        tipModel->setTable("emptytable");
        tipModel->select();
        ui->subjCombo->setModel(tipModel);
        ui->subjCombo->setModelColumn(tipModel->fieldIndex("emptycolumn"));
        return;
    }
    if (index == 1){
        tipModel->setTable("smo");
        tipModel->select();
        ui->subjCombo->setModel(tipModel);
        ui->subjCombo->setModelColumn(tipModel->fieldIndex("name"));
        ui->subjCombo->setCurrentIndex(0);
    } else {
        tipModel->setTable("regions");
        tipModel->select();
        ui->subjCombo->setModel(tipModel);
        ui->subjCombo->setModelColumn(tipModel->fieldIndex("region"));
        ui->subjCombo->setCurrentIndex(0);
    }
    return;
}

QString ReportDialog::N_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 1. Группировка почв по содержанию щелочногидролизуемого азота </b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Степень<br>обеспеченности</td><td align=center rowspan=3>Содержание,<br>мг/кг </td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Очень низкая</td><td align=center>&#8804 100</td>%4</tr>"
            "<tr><td align=center>2</td><td>Низкая</td><td align=center>101-150</td>%5</tr>"
            "<tr><td align=center>3</td><td>Средняя</td><td align=center>152-200</td>%6</tr>"
            "<tr><td align=center>4</td><td>Повышенная</td><td align=center>&#8805 200</td>%7</tr>"
            "<tr><td align=center colspan=3><b>Итого:</b></td>%8</tr>"
            "<tr><td align=center colspan=3><b>Средневзвешенное, мг/кг</b></td>%9</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(N*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.N<=100 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.N>100 and main.N<=150 and (main.smo in (%1)) "
                   " group by zemlepolz2, tip2) as table2 "
                   " on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.N>150 and main.N<=200 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.N>200 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();

        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg9.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg9.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg9.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");
    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
}

QString ReportDialog::P_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 2. Группировка почв по содержанию подвижного фосфора</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Степень<br>обеспеченности</td><td align=center rowspan=3>Содержание,<br>мг/кг </td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Очень низкая</td><td align=center>&#8804 10</td>%4</tr>"
            "<tr><td align=center>2</td><td>Низкая</td><td align=center>11-15</td>%5</tr>"
            "<tr><td align=center>3</td><td>Средняя</td><td align=center>16-30</td>%6</tr>"
            "<tr><td align=center>4</td><td>Повышенная</td><td align=center>31-45</td>%7</tr>"
            "<tr><td align=center>5</td><td>Высокая</td><td align=center>46-60</td>%8</tr>"
            "<tr><td align=center>6</td><td>Очень высокая</td><td align=center> > 60</td>%9</tr>");
    QByteArray russ2("<tr><td align=center colspan=3><b>Итого:</b></td>%1</tr>"
                     "<tr><td align=center colspan=3><b>Средневзвешенное, мг/кг</b></td>%2</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string2 = codec->toUnicode(russ2);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(P*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.P<=10 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.P>10 and main.P<=15 and (main.smo in (%1)) "
                   "group by zemlepolz2, tip2) as table2 "
                   "on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.P>15 and main.P<=30 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.P>30 and main.P<=45 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join (select tip as tip5, zemlepolz as zemlepolz5, sum(s_pochv_kont) as partsum from main where  main.P>45 and main.P<=60 and (main.smo in (%1)) "
                   "group by zemlepolz5, tip5) as table5 "
                   "on (main.tip=table5.tip5 and main.zemlepolz=table5.zemlepolz5) "
                   "left join (select tip as tip6, zemlepolz as zemlepolz6, sum(s_pochv_kont) as partsum from main where  main.P>60 and (main.smo in (%1)) "
                   "group by zemlepolz6, tip6) as table6 "
                   "on (main.tip=table6.tip6 and main.zemlepolz=table6.zemlepolz6) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9(""), arg10(""), arg11("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float partsum5_total = 0;
    float partsum6_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int partsum5_index = rec.indexOf("partsum5");
        int partsum6_index = rec.indexOf("partsum6");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant partsum5 = main_query.value(partsum5_index);
        QVariant partsum6 = main_query.value(partsum6_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        partsum5_total = partsum5.toFloat();
        partsum6_total = partsum6.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();

        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant partsum5 = main_query.value(partsum5_index);
            QVariant partsum6 = main_query.value(partsum6_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            partsum5_total = partsum5_total + partsum5.toFloat();
            partsum6_total = partsum6_total + partsum6.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(partsum5_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum5_total/allsum_total*100), 'f', 2)+"</td>");
    arg9.append("<td align=right>"+russian.toString(partsum6_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum6_total/allsum_total*100), 'f', 2)+"</td>");
    arg10.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg11.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)+ string2.arg(arg10, arg11);
}

QString ReportDialog::K_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 3. Группировка почв по содержанию обменного калия</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Степень<br>обеспеченности</td><td align=center rowspan=3>Содержание,<br>мг/кг </td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Очень низкая</td><td align=center>&#8804 100</td>%4</tr>"
            "<tr><td align=center>2</td><td>Низкая</td><td align=center>101-200</td>%5</tr>"
            "<tr><td align=center>3</td><td>Средняя</td><td align=center>201-300</td>%6</tr>"
            "<tr><td align=center>4</td><td>Повышенная</td><td align=center>301-400</td>%7</tr>"
            "<tr><td align=center>5</td><td>Высокая</td><td align=center>401-600</td>%8</tr>"
            "<tr><td align=center>6</td><td>Очень высокая</td><td align=center> > 600</td>%9</tr>");
    QByteArray russ2("<tr><td align=center colspan=3><b>Итого:</b></td>%1</tr>"
                     "<tr><td align=center colspan=3><b>Средневзвешенное, мг/кг</b></td>%2</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string2 = codec->toUnicode(russ2);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(K*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.K<=100 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.K>100 and main.K<=200 and (main.smo in (%1)) "
                   "group by zemlepolz2, tip2) as table2 "
                   "on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.K>200 and main.K<=300 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.K>300 and main.K<=400 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join (select tip as tip5, zemlepolz as zemlepolz5, sum(s_pochv_kont) as partsum from main where  main.K>400 and main.K<=600 and (main.smo in (%1)) "
                   "group by zemlepolz5, tip5) as table5 "
                   "on (main.tip=table5.tip5 and main.zemlepolz=table5.zemlepolz5) "
                   "left join (select tip as tip6, zemlepolz as zemlepolz6, sum(s_pochv_kont) as partsum from main where  main.K>600 and (main.smo in (%1)) "
                   "group by zemlepolz6, tip6) as table6 "
                   "on (main.tip=table6.tip6 and main.zemlepolz=table6.zemlepolz6) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9(""), arg10(""), arg11("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float partsum5_total = 0;
    float partsum6_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int partsum5_index = rec.indexOf("partsum5");
        int partsum6_index = rec.indexOf("partsum6");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant partsum5 = main_query.value(partsum5_index);
        QVariant partsum6 = main_query.value(partsum6_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        partsum5_total = partsum5.toFloat();
        partsum6_total = partsum6.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant partsum5 = main_query.value(partsum5_index);
            QVariant partsum6 = main_query.value(partsum6_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            partsum5_total = partsum5_total + partsum5.toFloat();
            partsum6_total = partsum6_total + partsum6.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(partsum5_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum5_total/allsum_total*100), 'f', 2)+"</td>");
    arg9.append("<td align=right>"+russian.toString(partsum6_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum6_total/allsum_total*100), 'f', 2)+"</td>");
    arg10.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg11.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)+ string2.arg(arg10, arg11);
}

QString ReportDialog::S_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 5. Группировка почв по содержанию подвижной серы</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Степень<br>обеспеченности</td><td align=center rowspan=3>Содержание,<br>мг/кг </td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Низкая</td><td align=center>&#8804 6</td>%4</tr>"
            "<tr><td align=center>2</td><td>Средняя</td><td align=center>6-12</td>%5</tr>"
            "<tr><td align=center>3</td><td>Высокая</td><td align=center> >12</td>%6</tr>"
            "<tr><td align=center colspan=3><b>Итого:</b></td>%7</tr>"
            "<tr><td align=center colspan=3><b>Средневзвешенное, мг/кг</b></td>%8</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(S*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.S<=6 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.S>6 and main.S<=12 and (main.smo in (%1)) "
                   " group by zemlepolz2, tip2) as table2 "
                   " on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.S>12 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg8.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

QString ReportDialog::Org_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 4. Группировка почв по содержанию органического вещества</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Содержание<br>органического<br>вещества, %</td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td align=center>&#8804 1,0</td>%4</tr>"
            "<tr><td align=center>2</td><td align=center>1,01-1,5</td>%5</tr>"
            "<tr><td align=center>3</td><td align=center>1,51-2,0</td>%6</tr>"
            "<tr><td align=center>4</td><td align=center>2,01-2,5</td>%7</tr>"
            "<tr><td align=center>5</td><td align=center>2,51-3,0</td>%8</tr>"
            "<tr><td align=center>6</td><td align=center>3,01-4,0</td>%9</tr>");
    QByteArray russ2("<tr><td align=center>7</td><td align=center>4,01-6,0</td>%1</tr>"
            "<tr><td align=center>8</td><td align=center>6,01-8,0</td>%2</tr>"
            "<tr><td align=center>9</td><td align=center>8,01-10</td>%3</tr>"
            "<tr><td align=center colspan=2><b>Итого:</b></td>%4</tr>"
            "<tr><td align=center colspan=2><b>Средневзвешеное,  %</b></td>%5</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string2 = codec->toUnicode(russ2);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(Org*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, "
                   "table5.partsum as partsum5, table6.partsum as partsum6, table7.partsum as partsum7, table8.partsum as partsum8, table9.partsum as partsum9 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.Org<=1 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.Org>1 and main.Org<=1.5 and (main.smo in (%1)) "
                   "group by zemlepolz2, tip2) as table2 "
                   "on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.Org>1.5 and main.Org<=2 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.Org>2 and main.Org<=2.5 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join (select tip as tip5, zemlepolz as zemlepolz5, sum(s_pochv_kont) as partsum from main where  main.Org>2.5 and main.Org<=3 and(main.smo in (%1)) "
                   "group by zemlepolz5, tip5) as table5 "
                   "on (main.tip=table5.tip5 and main.zemlepolz=table5.zemlepolz5) "
                   "left join (select tip as tip6, zemlepolz as zemlepolz6, sum(s_pochv_kont) as partsum from main where  main.Org>3 and main.Org<=4 and (main.smo in (%1)) "
                   "group by zemlepolz6, tip6) as table6 "
                   "on (main.tip=table6.tip6 and main.zemlepolz=table6.zemlepolz6) "
                   "left join (select tip as tip7, zemlepolz as zemlepolz7, sum(s_pochv_kont) as partsum from main where  main.Org>4 and main.Org<=6 and (main.smo in (%1)) "
                   "group by zemlepolz7, tip7) as table7 "
                   "on (main.tip=table7.tip7 and main.zemlepolz=table7.zemlepolz7) "
                   "left join (select tip as tip8, zemlepolz as zemlepolz8, sum(s_pochv_kont) as partsum from main where  main.Org>6 and main.Org<=8 and (main.smo in (%1)) "
                   "group by zemlepolz8, tip8) as table8 "
                   "on (main.tip=table8.tip8 and main.zemlepolz=table8.zemlepolz8) "
                   "left join (select tip as tip9, zemlepolz as zemlepolz9, sum(s_pochv_kont) as partsum from main where  main.Org>8 and main.Org<=10 and(main.smo in (%1)) "
                   "group by zemlepolz9, tip9) as table9 "
                   "on (main.tip=table9.tip9 and main.zemlepolz=table9.zemlepolz9) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9(""), arg10(""), arg11(""), arg12(""), arg13(""), arg14("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float partsum5_total = 0;
    float partsum6_total = 0;
    float partsum7_total = 0;
    float partsum8_total = 0;
    float partsum9_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int partsum5_index = rec.indexOf("partsum5");
        int partsum6_index = rec.indexOf("partsum6");
        int partsum7_index = rec.indexOf("partsum7");
        int partsum8_index = rec.indexOf("partsum8");
        int partsum9_index = rec.indexOf("partsum9");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant partsum5 = main_query.value(partsum5_index);
        QVariant partsum6 = main_query.value(partsum6_index);
        QVariant partsum7 = main_query.value(partsum7_index);
        QVariant partsum8 = main_query.value(partsum8_index);
        QVariant partsum9 = main_query.value(partsum9_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        partsum5_total = partsum5.toFloat();
        partsum6_total = partsum6.toFloat();
        partsum7_total = partsum7.toFloat();
        partsum8_total = partsum8.toFloat();
        partsum9_total = partsum9.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg10.append("<td align=right>"+russian.toString(partsum7.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum7.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg11.append("<td align=right>"+russian.toString(partsum8.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum8.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg12.append("<td align=right>"+russian.toString(partsum9.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum9.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg13.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg14.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant partsum5 = main_query.value(partsum5_index);
            QVariant partsum6 = main_query.value(partsum6_index);
            QVariant partsum7 = main_query.value(partsum7_index);
            QVariant partsum8 = main_query.value(partsum8_index);
            QVariant partsum9 = main_query.value(partsum9_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            partsum5_total = partsum5_total + partsum5.toFloat();
            partsum6_total = partsum6_total + partsum6.toFloat();
            partsum7_total = partsum7_total + partsum7.toFloat();
            partsum8_total = partsum8_total + partsum8.toFloat();
            partsum9_total = partsum9_total + partsum9.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg10.append("<td align=right>"+russian.toString(partsum7.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum7.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg11.append("<td align=right>"+russian.toString(partsum8.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum8.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg12.append("<td align=right>"+russian.toString(partsum9.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum9.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg13.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg14.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");
            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(partsum5_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum5_total/allsum_total*100), 'f', 2)+"</td>");
    arg9.append("<td align=right>"+russian.toString(partsum6_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum6_total/allsum_total*100), 'f', 2)+"</td>");
    arg10.append("<td align=right>"+russian.toString(partsum7_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum7_total/allsum_total*100), 'f', 2)+"</td>");
    arg11.append("<td align=right>"+russian.toString(partsum8_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum8_total/allsum_total*100), 'f', 2)+"</td>");
    arg12.append("<td align=right>"+russian.toString(partsum9_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum9_total/allsum_total*100), 'f', 2)+"</td>");
    arg13.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg14.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");
    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)+ string2.arg(arg10, arg11, arg12, arg13, arg14);
}

QString ReportDialog::PH_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 6. Группировка почв по степени щелочности (кислотности) </b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Реакция<br>среды</td><td align=center rowspan=3>Показатель<br>рН(водный)</td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Слабокислая</td><td align=center>4,1-5,5</td>%4</tr>"
            "<tr><td align=center>2</td><td>Близкая к нейтр.</td><td align=center>5,6-6,5</td>%5</tr>"
            "<tr><td align=center>3</td><td>Нейтральная</td><td align=center>6,6-7,5</td>%6</tr>"
            "<tr><td align=center>4</td><td>Слабощелочная</td><td align=center>7,6-8,5</td>%7</tr>"
            "<tr><td align=center>5</td><td>Щелочная</td><td align=center>8,6-9,0</td>%8</tr>"
            "<tr><td align=center>6</td><td>Сильнощелочная</td><td align=center>9,1-11,0</td>%9</tr>");
    QByteArray russ2("<tr><td align=center colspan=3><b>Итого:</b></td>%1</tr>"
                     "<tr><td align=center colspan=3><b>Средневзвешенное</b></td>%2</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string2 = codec->toUnicode(russ2);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(ph*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.ph > 4 and main.ph <= 5.5 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.ph > 5.5 and main.ph <= 6.5 and (main.smo in (%1)) "
                   "group by zemlepolz2, tip2) as table2 "
                   "on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.ph > 6.5 and main.ph <= 7.5 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.ph > 7.5 and main.ph <= 8.5 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join (select tip as tip5, zemlepolz as zemlepolz5, sum(s_pochv_kont) as partsum from main where  main.ph > 8.5 and main.ph <= 9 and (main.smo in (%1)) "
                   "group by zemlepolz5, tip5) as table5 "
                   "on (main.tip=table5.tip5 and main.zemlepolz=table5.zemlepolz5) "
                   "left join (select tip as tip6, zemlepolz as zemlepolz6, sum(s_pochv_kont) as partsum from main where  main.ph > 9 and main.ph <= 11 and (main.smo in (%1)) "
                   "group by zemlepolz6, tip6) as table6 "
                   "on (main.tip=table6.tip6 and main.zemlepolz=table6.zemlepolz6) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9(""), arg10(""), arg11("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float partsum5_total = 0;
    float partsum6_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int partsum5_index = rec.indexOf("partsum5");
        int partsum6_index = rec.indexOf("partsum6");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant partsum5 = main_query.value(partsum5_index);
        QVariant partsum6 = main_query.value(partsum6_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        partsum5_total = partsum5.toFloat();
        partsum6_total = partsum6.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant partsum5 = main_query.value(partsum5_index);
            QVariant partsum6 = main_query.value(partsum6_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            partsum5_total = partsum5_total + partsum5.toFloat();
            partsum6_total = partsum6_total + partsum6.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(partsum5_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum5_total/allsum_total*100), 'f', 2)+"</td>");
    arg9.append("<td align=right>"+russian.toString(partsum6_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum6_total/allsum_total*100), 'f', 2)+"</td>");
    arg10.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg11.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)+ string2.arg(arg10, arg11);
}

QString ReportDialog::Ca_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 7. Группировка почв по содержанию обменного кальция</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Содержание в почве<br>обменного кальция</td><td align=center rowspan=3>мг-экв/100г<br>почвы</td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Очень низкое</td><td align=center>&#8804 2,5</td>%4</tr>"
            "<tr><td align=center>2</td><td>Низкое</td><td align=center>2,6-5,0</td>%5</tr>"
            "<tr><td align=center>3</td><td>Среднее</td><td align=center>5,1-10,0</td>%6</tr>"
            "<tr><td align=center>4</td><td>Повышенное</td><td align=center>10,1-15,0</td>%7</tr>"
            "<tr><td align=center>5</td><td>Высокое</td><td align=center>15,1-20,0</td>%8</tr>"
            "<tr><td align=center>6</td><td>Очень высокое</td><td align=center> > 20,0</td>%9</tr>");
    QByteArray russ2("<tr><td align=center colspan=3><b>Итого:</b></td>%1</tr>"
                     "<tr><td align=center colspan=3><b>Средневзвешенное</b></td>%2</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string2 = codec->toUnicode(russ2);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(Ca*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.Ca <= 2.5 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.Ca > 2.5 and main.Ca <= 5 and (main.smo in (%1)) "
                   "group by zemlepolz2, tip2) as table2 "
                   "on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.Ca > 5 and main.Ca <= 10 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.Ca > 10 and main.Ca <= 15 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join (select tip as tip5, zemlepolz as zemlepolz5, sum(s_pochv_kont) as partsum from main where  main.Ca > 15 and main.Ca <= 20 and (main.smo in (%1)) "
                   "group by zemlepolz5, tip5) as table5 "
                   "on (main.tip=table5.tip5 and main.zemlepolz=table5.zemlepolz5) "
                   "left join (select tip as tip6, zemlepolz as zemlepolz6, sum(s_pochv_kont) as partsum from main where  main.Ca > 20 and (main.smo in (%1)) "
                   "group by zemlepolz6, tip6) as table6 "
                   "on (main.tip=table6.tip6 and main.zemlepolz=table6.zemlepolz6) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9(""), arg10(""), arg11("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float partsum5_total = 0;
    float partsum6_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int partsum5_index = rec.indexOf("partsum5");
        int partsum6_index = rec.indexOf("partsum6");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant partsum5 = main_query.value(partsum5_index);
        QVariant partsum6 = main_query.value(partsum6_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        partsum5_total = partsum5.toFloat();
        partsum6_total = partsum6.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant partsum5 = main_query.value(partsum5_index);
            QVariant partsum6 = main_query.value(partsum6_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            partsum5_total = partsum5_total + partsum5.toFloat();
            partsum6_total = partsum6_total + partsum6.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(partsum5_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum5_total/allsum_total*100), 'f', 2)+"</td>");
    arg9.append("<td align=right>"+russian.toString(partsum6_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum6_total/allsum_total*100), 'f', 2)+"</td>");
    arg10.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg11.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)+ string2.arg(arg10, arg11);
}

QString ReportDialog::Mg_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 8. Группировка почв по содержанию обменного магния</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Содержание в почве<br>обменного магния</td><td align=center rowspan=3>мг-экв/100г<br>почвы</td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Очень низкое</td><td align=center>&#8804 0,5</td>%4</tr>"
            "<tr><td align=center>2</td><td>Низкое</td><td align=center>0,6-1,0</td>%5</tr>"
            "<tr><td align=center>3</td><td>Среднее</td><td align=center>1,1-2,0</td>%6</tr>"
            "<tr><td align=center>4</td><td>Повышенное</td><td align=center>2,1-3,0</td>%7</tr>"
            "<tr><td align=center>5</td><td>Высокое</td><td align=center>3,1-4,0</td>%8</tr>"
            "<tr><td align=center>6</td><td>Очень высокое</td><td align=center> > 4,0</td>%9</tr>");
    QByteArray russ2("<tr><td align=center colspan=3><b>Итого:</b></td>%1</tr>"
                     "<tr><td align=center colspan=3><b>Средневзвешенное</b></td>%2</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string2 = codec->toUnicode(russ2);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(Mg*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.Mg <= 0.5 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.Mg > 0.5 and main.Mg <= 1 and (main.smo in (%1)) "
                   "group by zemlepolz2, tip2) as table2 "
                   "on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.Mg > 1 and main.Mg <= 2 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.Mg > 2 and main.Mg <= 3 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join (select tip as tip5, zemlepolz as zemlepolz5, sum(s_pochv_kont) as partsum from main where  main.Mg > 3 and main.Mg <= 4 and (main.smo in (%1)) "
                   "group by zemlepolz5, tip5) as table5 "
                   "on (main.tip=table5.tip5 and main.zemlepolz=table5.zemlepolz5) "
                   "left join (select tip as tip6, zemlepolz as zemlepolz6, sum(s_pochv_kont) as partsum from main where  main.Mg > 4 and (main.smo in (%1)) "
                   "group by zemlepolz6, tip6) as table6 "
                   "on (main.tip=table6.tip6 and main.zemlepolz=table6.zemlepolz6) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9(""), arg10(""), arg11("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float partsum5_total = 0;
    float partsum6_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int partsum5_index = rec.indexOf("partsum5");
        int partsum6_index = rec.indexOf("partsum6");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant partsum5 = main_query.value(partsum5_index);
        QVariant partsum6 = main_query.value(partsum6_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        partsum5_total = partsum5.toFloat();
        partsum6_total = partsum6.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant partsum5 = main_query.value(partsum5_index);
            QVariant partsum6 = main_query.value(partsum6_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            partsum5_total = partsum5_total + partsum5.toFloat();
            partsum6_total = partsum6_total + partsum6.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(partsum5_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum5_total/allsum_total*100), 'f', 2)+"</td>");
    arg9.append("<td align=right>"+russian.toString(partsum6_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum6_total/allsum_total*100), 'f', 2)+"</td>");
    arg10.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg11.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)+ string2.arg(arg10, arg11);
}

QString ReportDialog::REKO_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 9. Группировка почв по ёмкости катионного обмена</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Ёмкость<br>катионного<br>обмена</td><td align=center rowspan=3>мг-экв/100г<br>почвы</td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Очень низкая</td><td align=center>&#8804 5,0</td>%4</tr>"
            "<tr><td align=center>2</td><td>Низкая</td><td align=center>5,1-10,0</td>%5</tr>"
            "<tr><td align=center>3</td><td>Средняя</td><td align=center>10,1-15,0</td>%6</tr>"
            "<tr><td align=center>4</td><td>Повышенная</td><td align=center>15,1-20,0</td>%7</tr>"
            "<tr><td align=center>5</td><td>Высокая</td><td align=center>20,1-30,0</td>%8</tr>"
            "<tr><td align=center>6</td><td>Очень высокая</td><td align=center> >30,0 </td>%9</tr>");
    QByteArray russ2("<tr><td align=center colspan=3><b>Итого:</b></td>%1</tr>"
                     "<tr><td align=center colspan=3><b>Средневзвешенное</b></td>%2</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string2 = codec->toUnicode(russ2);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(Reko*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.Reko <= 5 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.Reko > 5 and main.Reko <= 10 and (main.smo in (%1)) "
                   "group by zemlepolz2, tip2) as table2 "
                   "on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.Reko > 10 and main.Reko <= 15 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.Reko > 15 and main.Reko <= 20 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join (select tip as tip5, zemlepolz as zemlepolz5, sum(s_pochv_kont) as partsum from main where  main.Reko > 20 and main.Reko <= 30 and (main.smo in (%1)) "
                   "group by zemlepolz5, tip5) as table5 "
                   "on (main.tip=table5.tip5 and main.zemlepolz=table5.zemlepolz5) "
                   "left join (select tip as tip6, zemlepolz as zemlepolz6, sum(s_pochv_kont) as partsum from main where  main.Reko > 30 and (main.smo in (%1)) "
                   "group by zemlepolz6, tip6) as table6 "
                   "on (main.tip=table6.tip6 and main.zemlepolz=table6.zemlepolz6) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9(""), arg10(""), arg11("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float partsum5_total = 0;
    float partsum6_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int partsum5_index = rec.indexOf("partsum5");
        int partsum6_index = rec.indexOf("partsum6");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant partsum5 = main_query.value(partsum5_index);
        QVariant partsum6 = main_query.value(partsum6_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        partsum5_total = partsum5.toFloat();
        partsum6_total = partsum6.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant partsum5 = main_query.value(partsum5_index);
            QVariant partsum6 = main_query.value(partsum6_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            partsum5_total = partsum5_total + partsum5.toFloat();
            partsum6_total = partsum6_total + partsum6.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg11.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(partsum5_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum5_total/allsum_total*100), 'f', 2)+"</td>");
    arg9.append("<td align=right>"+russian.toString(partsum6_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum6_total/allsum_total*100), 'f', 2)+"</td>");
    arg10.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg11.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)+ string2.arg(arg10, arg11);
}

QString ReportDialog::Zn_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 11. Группировка почв по содержанию микроэлементов (цинк)</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Степень<br>обеспеченности</td><td align=center rowspan=3>Содержание,<br>мг/кг </td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Низкая</td><td align=center>&#8804 2,0</td>%4</tr>"
            "<tr><td align=center>2</td><td>Средняя</td><td align=center>2,1-5,0</td>%5</tr>"
            "<tr><td align=center>3</td><td>Высокая</td><td align=center> >5,0</td>%6</tr>"
            "<tr><td align=center colspan=3><b>Итого:</b></td>%7</tr>"
            "<tr><td align=center colspan=3><b>Средневзвешенное, мг/кг</b></td>%8</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(Zn*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.Zn <=2 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.Zn > 2 and main.Zn <= 5 and (main.smo in (%1)) "
                   " group by zemlepolz2, tip2) as table2 "
                   " on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.Zn > 5 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg8.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

QString ReportDialog::Cu_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 11. Группировка почв по содержанию микроэлементов (медь)</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Степень<br>обеспеченности</td><td align=center rowspan=3>Содержание,<br>мг/кг </td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Низкая</td><td align=center>&#8804  0,20</td>%4</tr>"
            "<tr><td align=center>2</td><td>Средняя</td><td align=center>0,21-0,50</td>%5</tr>"
            "<tr><td align=center>3</td><td>Высокая</td><td align=center> >0,50</td>%6</tr>"
            "<tr><td align=center colspan=3><b>Итого:</b></td>%7</tr>"
            "<tr><td align=center colspan=3><b>Средневзвешенное, мг/кг</b></td>%8</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(Cu*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.Cu <= 0.2 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.Cu > 0.2 and main.Cu <= 0.5 and (main.smo in (%1)) "
                   " group by zemlepolz2, tip2) as table2 "
                   " on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.Cu > 0.5 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg8.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

QString ReportDialog::Mo_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 11. Группировка почв по содержанию микроэлементов (молибден)</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Степень<br>обеспеченности</td><td align=center rowspan=3>Содержание,<br>мг/кг </td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Низкая</td><td align=center>&#8804  0,10</td>%4</tr>"
            "<tr><td align=center>2</td><td>Средняя</td><td align=center>0,11-0,22</td>%5</tr>"
            "<tr><td align=center>3</td><td>Высокая</td><td align=center> >0,22</td>%6</tr>"
            "<tr><td align=center colspan=3><b>Итого:</b></td>%7</tr>"
            "<tr><td align=center colspan=3><b>Средневзвешенное, мг/кг</b></td>%8</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(Mo*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.Mo <= 0.1 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.Mo > 0.1 and main.Mo <= 0.22 and (main.smo in (%1)) "
                   " group by zemlepolz2, tip2) as table2 "
                   " on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.Mo > 0.22 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg8.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

QString ReportDialog::B_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 11. Группировка почв по содержанию микроэлементов (бор)</b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Степень<br>обеспеченности</td><td align=center rowspan=3>Содержание,<br>мг/кг </td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Низкая</td><td align=center>&#8804 0,33</td>%4</tr>"
            "<tr><td align=center>2</td><td>Средняя</td><td align=center>0,34-0,70</td>%5</tr>"
            "<tr><td align=center>3</td><td>Высокая</td><td align=center> >0,70</td>%6</tr>"
            "<tr><td align=center colspan=3><b>Итого:</b></td>%7</tr>"
            "<tr><td align=center colspan=3><b>Средневзвешенное, мг/кг</b></td>%8</tr>"
            "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(B*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.B <= 0.33 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.B > 0.33 and main.B <= 0.7 and (main.smo in (%1)) "
                   " group by zemlepolz2, tip2) as table2 "
                   " on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.B > 0.7 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float allsum_total = 0;
    float ves_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int allsum_index = rec.indexOf("allsum");
        int ves_index = rec.indexOf("ves");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant allsum = main_query.value(allsum_index);
        QVariant ves = main_query.value(ves_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        allsum_total = allsum.toFloat();
        ves_total = ves.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
        arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant allsum = main_query.value(allsum_index);
            QVariant ves = main_query.value(ves_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            allsum_total = allsum_total + allsum.toFloat();
            ves_total = ves_total + ves.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");
            arg8.append("<td align=center colspan=2>"+russian.toString(ves.toFloat()/allsum.toFloat(), 'f', 2)+"</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");
    arg8.append("<td align=center colspan=2>"+russian.toString(ves_total/allsum_total, 'f', 2)+"</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

QString ReportDialog::granul_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style><p align=left><b>Таблица 7. Распределение площади с/х угодий по гранулометрическому составу </b></p><table border-collapse>"
            "<tr><td align=center rowspan=3>Класс<br>обеспеченности</td><td align=center rowspan=3>Разновидности почв<br>по гранулометри-<br>ческому составу </td><td align=center rowspan=3>Содер.<br>физич.<br>глины,<br>%)</td> %1 <td colspan=2 rowspan=2>Всего с/х<br>угодий</td></tr>"
            "<tr>%2</tr>"
            "<tr>%3<td align=right>    га.</td><td align=right>    %</td></tr>"
            "<tr><td align=center>1</td><td>Глинистые</td><td align=center> >60</td>%4</tr>"
            "<tr><td align=center>2</td><td>Тяжелосуглинистые</td><td align=center>45-60</td>%5</tr>"
            "<tr><td align=center>3</td><td>Среднесуглинистые</td><td align=center>30-45</td>%6</tr>"
            "<tr><td align=center>4</td><td>Легкосуглинистые</td><td align=center>20-30</td>%7</tr>"
            "<tr><td align=center>5</td><td>Супесчаные</td><td align=center>10-20</td>%8</tr>"
            "<tr><td align=center>6</td><td>Песчаные</td><td align=center> <10</td>%9</tr>");
    QByteArray russ2("<tr><td align=center colspan=3><b>Итого:</b></td>%1</tr>"
                     "</table>");
    QByteArray forArg3("<td align=right>    га.</td><td align=right>    %</td>");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString string2 = codec->toUnicode(russ2);
    QString string_a3 = codec->toUnicode(forArg3);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, zemlepolz, tip_table.name, owner.owner, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, zemlepolz as zemlepolz1, sum(s_pochv_kont) as partsum from main where  main.granul = 5 and (main.smo in (%1)) "
                   "group by zemlepolz1, tip1) as table1 "
                   "on (main.tip=table1.tip1 and main.zemlepolz=table1.zemlepolz1) "
                   "left join (select tip as tip2, zemlepolz as zemlepolz2, sum(s_pochv_kont) as partsum from main where  main.granul = 3 and (main.smo in (%1)) "
                   "group by zemlepolz2, tip2) as table2 "
                   "on (main.tip=table2.tip2 and main.zemlepolz=table2.zemlepolz2) "
                   "left join (select tip as tip3, zemlepolz as zemlepolz3, sum(s_pochv_kont) as partsum from main where  main.granul = 2 and (main.smo in (%1)) "
                   "group by zemlepolz3, tip3) as table3 "
                   "on (main.tip=table3.tip3 and main.zemlepolz=table3.zemlepolz3) "
                   "left join (select tip as tip4, zemlepolz as zemlepolz4, sum(s_pochv_kont) as partsum from main where  main.granul = 4 and (main.smo in (%1)) "
                   "group by zemlepolz4, tip4) as table4 "
                   "on (main.tip=table4.tip4 and main.zemlepolz=table4.zemlepolz4) "
                   "left join (select tip as tip5, zemlepolz as zemlepolz5, sum(s_pochv_kont) as partsum from main where  main.granul = 7 and (main.smo in (%1)) "
                   "group by zemlepolz5, tip5) as table5 "
                   "on (main.tip=table5.tip5 and main.zemlepolz=table5.zemlepolz5) "
                   "left join (select tip as tip6, zemlepolz as zemlepolz6, sum(s_pochv_kont) as partsum from main where  main.granul = 6 and (main.smo in (%1)) "
                   "group by zemlepolz6, tip6) as table6 "
                   "on (main.tip=table6.tip6 and main.zemlepolz=table6.zemlepolz6) "
                   "left join owner "
                   "on owner.id_owner=main.zemlepolz "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.zemlepolz, main.tip "
                   "order by main.tip, main.zemlepolz ");

    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    QList<QString> tip;
    QList<int> tip_kol;

    int tip_count = 0;
    int tip_index = 0;

    QString str, str2;
    QString arg1(""), arg2(""), arg3(""), arg4(""), arg5(""), arg6(""), arg7(""), arg8(""), arg9(""), arg10("");
    float partsum1_total = 0;
    float partsum2_total = 0;
    float partsum3_total = 0;
    float partsum4_total = 0;
    float partsum5_total = 0;
    float partsum6_total = 0;
    float allsum_total = 0;

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_index = rec.indexOf("partsum1");
        int partsum2_index = rec.indexOf("partsum2");
        int partsum3_index = rec.indexOf("partsum3");
        int partsum4_index = rec.indexOf("partsum4");
        int partsum5_index = rec.indexOf("partsum5");
        int partsum6_index = rec.indexOf("partsum6");
        int allsum_index = rec.indexOf("allsum");

        main_query.first();

        QVariant partsum1 = main_query.value(partsum1_index);
        QVariant partsum2 = main_query.value(partsum2_index);
        QVariant partsum3 = main_query.value(partsum3_index);
        QVariant partsum4 = main_query.value(partsum4_index);
        QVariant partsum5 = main_query.value(partsum5_index);
        QVariant partsum6 = main_query.value(partsum6_index);
        QVariant allsum = main_query.value(allsum_index);

        partsum1_total = partsum1.toFloat();
        partsum2_total = partsum2.toFloat();
        partsum3_total = partsum3.toFloat();
        partsum4_total = partsum4.toFloat();
        partsum5_total = partsum5.toFloat();
        partsum6_total = partsum6.toFloat();
        allsum_total = allsum.toFloat();


        tip.append(main_query.value(2).toString());
        arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
        arg3.append(string_a3);
        arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
        arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");

        tip_kol.append(1);
        tip_count = 1;
        while (main_query.next()){

            QVariant partsum1 = main_query.value(partsum1_index);
            QVariant partsum2 = main_query.value(partsum2_index);
            QVariant partsum3 = main_query.value(partsum3_index);
            QVariant partsum4 = main_query.value(partsum4_index);
            QVariant partsum5 = main_query.value(partsum5_index);
            QVariant partsum6 = main_query.value(partsum6_index);
            QVariant allsum = main_query.value(allsum_index);

            partsum1_total = partsum1_total + partsum1.toFloat();
            partsum2_total = partsum2_total + partsum2.toFloat();
            partsum3_total = partsum3_total + partsum3.toFloat();
            partsum4_total = partsum4_total + partsum4.toFloat();
            partsum5_total = partsum5_total + partsum5.toFloat();
            partsum6_total = partsum6_total + partsum6.toFloat();
            allsum_total = allsum_total + allsum.toFloat();

            arg2.append("<td colspan=2>"+main_query.value(3).toString()+"</td>");
            arg3.append(string_a3);
            arg4.append("<td align=right>"+russian.toString(partsum1.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum1.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg5.append("<td align=right>"+russian.toString(partsum2.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum2.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg6.append("<td align=right>"+russian.toString(partsum3.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum3.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg7.append("<td align=right>"+russian.toString(partsum4.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum4.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg8.append("<td align=right>"+russian.toString(partsum5.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum5.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg9.append("<td align=right>"+russian.toString(partsum6.toFloat(), 'f', 2)+"</td><td align=right>"+russian.toString(partsum6.toFloat()/allsum.toFloat()*100, 'f', 2)+"</td>");
            arg10.append("<td align=right>"+russian.toString(allsum.toFloat(), 'f', 2)+"</td><td align=right>100.00</td>");

            if(main_query.value(2).toString()==tip[tip_index]){
                tip_count++;
            } else {
                tip_kol[tip_index] = tip_count;
                tip.append(main_query.value(2).toString());
                tip_index++;
                tip_kol.append(1);
                tip_count = 1;
            }
        }

        tip_kol[tip_index] = tip_count;

        for (int i = 0; i <= tip_index; i++){
            arg1.append(QString("<td colspan=%1>%2</td>").arg(str.setNum(2*tip_kol[i]),tip[i]));
        }

    } else {
        return main_query.lastError().databaseText();
    }

    arg4.append("<td align=right>"+russian.toString(partsum1_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum1_total/allsum_total*100), 'f', 2)+"</td>");
    arg5.append("<td align=right>"+russian.toString(partsum2_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum2_total/allsum_total*100), 'f', 2)+"</td>");
    arg6.append("<td align=right>"+russian.toString(partsum3_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum3_total/allsum_total*100), 'f', 2)+"</td>");
    arg7.append("<td align=right>"+russian.toString(partsum4_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum4_total/allsum_total*100), 'f', 2)+"</td>");
    arg8.append("<td align=right>"+russian.toString(partsum5_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum5_total/allsum_total*100), 'f', 2)+"</td>");
    arg9.append("<td align=right>"+russian.toString(partsum6_total, 'f', 2)+"</td><td align=right>"+russian.toString((partsum6_total/allsum_total*100), 'f', 2)+"</td>");
    arg10.append("<td align=right>"+str.setNum(allsum_total, 'f', 2)+"</td><td align=right>100.00</td>");

    return string.arg(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)+ string2.arg(arg10);
}

QString ReportDialog::Cd_report(QString id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b> Таблица 11. Группировка почв  по эколого-токсикологической (ЭТГ) оценке содержания подвижных форм тяжелых металлов, мышьяка  и ртути.</b></p>"
                    "<table border-collapse>"
                    "<tr><td align=center rowspan=3>Элемент</td><td align=center rowspan=3>Класс<br>опас-<br>ности</td><td align=center rowspan=3>Вид c/х<br>угодья</td><td align=center rowspan=3>ПДК</td><td align=center rowspan=3>Ср. взв.<br>мг/кг</td><td align=center rowspan=2>Всего</td><td align=center colspan=5>Группы по содержанию ТМ</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td></tr>"
                    "<tr><td align=center>га.</td><td align=center>га.</td><td align=center>га.</td><td align=center>га.</td><td align=center>га.</td><td align=center>га.</td></tr>");
    QByteArray russ2("<tr><td align=center>Кадмий</td><td></td><td rowspan=4 align=center>%1</td><td align=center>2,0</td>%2</tr>"
                    "<tr><td align=center>Свинец</td><td></td><td align=center>6,0</td>%3</tr>"
                    "<tr><td align=center>Мышьяк</td><td></td><td align=center>%4</td>%5</tr>"
                    "<tr><td align=center>Ртуть</td><td></td><td align=center>2,1</td>%6</tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QString main_str = codec->toUnicode(russ2);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, main.granul, tip_table.name, granul_table.name, sum(s_pochv_kont) as allsum, sum(Cd*s_pochv_kont) as ves_Cd, sum(Pb*s_pochv_kont) as ves_Pb, sum(Hg*s_pochv_kont) as ves_Hg, sum(main.As*s_pochv_kont) as ves_As, "
                               "table1_Cd.partsum as partsum1_Cd, table2_Cd.partsum as partsum2_Cd, table3_Cd.partsum as partsum3_Cd, table4_Cd.partsum as partsum4_Cd, table5_Cd.partsum as partsum5_Cd, "
                               "table1_Pb.partsum as partsum1_Pb, table2_Pb.partsum as partsum2_Pb, table3_Pb.partsum as partsum3_Pb, table4_Pb.partsum as partsum4_Pb, table5_Pb.partsum as partsum5_Pb, "
                               "table1_Hg.partsum as partsum1_Hg, table2_Hg.partsum as partsum2_Hg, table3_Hg.partsum as partsum3_Hg, table4_Hg.partsum as partsum4_Hg, table5_Hg.partsum as partsum5_Hg, "
                               "table1_As_pes.partsum as partsum1_As_pes, table2_As_pes.partsum as partsum2_As_pes, table3_As_pes.partsum as partsum3_As_pes, table4_As_pes.partsum as partsum4_As_pes, table5_As_pes.partsum as partsum5_As_pes, "
                               "table1_As_gli.partsum as partsum1_As_gli, table2_As_gli.partsum as partsum2_As_gli, table3_As_gli.partsum as partsum3_As_gli, table4_As_gli.partsum as partsum4_As_gli, table5_As_gli.partsum as partsum5_As_gli "
                               " from main "
                               "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.Cd < 1 and (main.smo in (%1)) "
                               "group by granul1, tip1) as table1_Cd "
                               "on (main.tip=table1_Cd.tip1 and main.granul=table1_Cd.granul1) "
                               "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.Cd >= 1 and main.Cd <= 2 and (main.smo in (%1)) "
                               "group by granul2, tip2) as table2_Cd "
                               "on (main.tip=table2_Cd.tip2 and main.granul=table2_Cd.granul2) "
                               "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.Cd > 2 and main.Cd <= 4 and (main.smo in (%1)) "
                               "group by granul3, tip3) as table3_Cd "
                               "on (main.tip=table3_Cd.tip3 and main.granul=table3_Cd.granul3) "
                               "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.Cd > 4 and main.Cd <= 6 and (main.smo in (%1)) "
                               "group by granul4, tip4) as table4_Cd "
                               "on (main.tip=table4_Cd.tip4 and main.granul=table4_Cd.granul4) "
                               "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.Cd > 6 and (main.smo in (%1)) "
                               "group by granul5, tip5) as table5_Cd "
                               "on (main.tip=table5_Cd.tip5 and main.granul=table5_Cd.granul5) "
                               "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.Pb < 3 and (main.smo in (%1)) "
                               "group by granul1, tip1) as table1_Pb "
                               "on (main.tip=table1_Pb.tip1 and main.granul=table1_Pb.granul1) "
                               "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.Pb >= 3 and main.Pb <= 6 and (main.smo in (%1)) "
                               "group by granul2, tip2) as table2_Pb "
                               "on (main.tip=table2_Pb.tip2 and main.granul=table2_Pb.granul2) "
                               "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.Pb > 6 and main.Pb <= 12 and (main.smo in (%1)) "
                               "group by granul3, tip3) as table3_Pb "
                               "on (main.tip=table3_Pb.tip3 and main.granul=table3_Pb.granul3) "
                               "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.Pb > 12 and main.Pb <= 18 and (main.smo in (%1)) "
                               "group by granul4, tip4) as table4_Pb "
                               "on (main.tip=table4_Pb.tip4 and main.granul=table4_Pb.granul4) "
                               "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.Pb > 18 and (main.smo in (%1)) "
                               "group by granul5, tip5) as table5_Pb "
                               "on (main.tip=table5_Pb.tip5 and main.granul=table5_Pb.granul5) "
                               "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.Hg < 1 and (main.smo in (%1)) "
                               "group by granul1, tip1) as table1_Hg "
                               "on (main.tip=table1_Hg.tip1 and main.granul=table1_Hg.granul1) "
                               "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.Hg >= 1 and main.Hg <= 2.1 and (main.smo in (%1)) "
                               "group by granul2, tip2) as table2_Hg "
                               "on (main.tip=table2_Hg.tip2 and main.granul=table2_Hg.granul2) "
                               "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.Hg > 2.1 and main.Hg <= 4.2 and (main.smo in (%1)) "
                               "group by granul3, tip3) as table3_Hg "
                               "on (main.tip=table3_Hg.tip3 and main.granul=table3_Hg.granul3) "
                               "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.Hg > 4.2 and main.Hg <= 6.2 and (main.smo in (%1)) "
                               "group by granul4, tip4) as table4_Hg "
                               "on (main.tip=table4_Hg.tip4 and main.granul=table4_Hg.granul4) "
                               "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.Hg > 6.2 and (main.smo in (%1)) "
                               "group by granul5, tip5) as table5_Hg "
                               "on (main.tip=table5_Hg.tip5 and main.granul=table5_Hg.granul5) "
                               "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.As < 1 and (main.smo in (%1)) "
                               "group by granul1, tip1) as table1_As_pes "
                               "on (main.tip=table1_As_pes.tip1 and main.granul=table1_As_pes.granul1) "
                               "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.As >= 1 and main.As <= 2 and (main.smo in (%1)) "
                               "group by granul2, tip2) as table2_As_pes "
                               "on (main.tip=table2_As_pes.tip2 and main.granul=table2_As_pes.granul2) "
                               "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.As > 2 and main.As <= 4 and (main.smo in (%1)) "
                               "group by granul3, tip3) as table3_As_pes "
                               "on (main.tip=table3_As_pes.tip3 and main.granul=table3_As_pes.granul3) "
                               "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.As > 4 and main.As <= 6 and (main.smo in (%1)) "
                               "group by granul4, tip4) as table4_As_pes "
                               "on (main.tip=table4_As_pes.tip4 and main.granul=table4_As_pes.granul4) "
                               "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.As > 6 and (main.smo in (%1)) "
                               "group by granul5, tip5) as table5_As_pes "
                               "on (main.tip=table5_As_pes.tip5 and main.granul=table5_As_pes.granul5) "
                               "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.As < 5 and (main.smo in (%1)) "
                               "group by granul1, tip1) as table1_As_gli "
                               "on (main.tip=table1_As_gli.tip1 and main.granul=table1_As_gli.granul1) "
                               "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.As >= 5 and main.As <= 10 and (main.smo in (%1)) "
                               "group by granul2, tip2) as table2_As_gli "
                               "on (main.tip=table2_As_gli.tip2 and main.granul=table2_As_gli.granul2) "
                               "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.As > 10 and main.As <= 20 and (main.smo in (%1)) "
                               "group by granul3, tip3) as table3_As_gli "
                               "on (main.tip=table3_As_gli.tip3 and main.granul=table3_As_gli.granul3) "
                               "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.As > 20 and main.As <= 30 and (main.smo in (%1)) "
                               "group by granul4, tip4) as table4_As_gli "
                               "on (main.tip=table4_As_gli.tip4 and main.granul=table4_As_gli.granul4) "
                               "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.As > 30 and (main.smo in (%1)) "
                               "group by granul5, tip5) as table5_As_gli "
                               "on (main.tip=table5_As_gli.tip5 and main.granul=table5_As_gli.granul5) "
                               "left join granul AS granul_table "
                               "on granul_table.id_granul=main.granul "
                               "left join tip AS tip_table "
                               "on tip_table.id_tip=main.tip "
                               "where main.smo in (%1) "
                               "group by main.tip, main.granul "
                               "order by main.tip, main.granul ");
    QSqlQuery main_query;
    zapros = zapros.arg(id);
    main_query.prepare(zapros);

    if(main_query.exec()&&(main_query.size()>0)){

        QSqlRecord rec = main_query.record();
        int partsum1_Cd_index = rec.indexOf("partsum1_Cd");
        int partsum2_Cd_index = rec.indexOf("partsum2_Cd");
        int partsum3_Cd_index = rec.indexOf("partsum3_Cd");
        int partsum4_Cd_index = rec.indexOf("partsum4_Cd");
        int partsum5_Cd_index = rec.indexOf("partsum5_Cd");
        int partsum1_Pb_index = rec.indexOf("partsum1_Pb");
        int partsum2_Pb_index = rec.indexOf("partsum2_Pb");
        int partsum3_Pb_index = rec.indexOf("partsum3_Pb");
        int partsum4_Pb_index = rec.indexOf("partsum4_Pb");
        int partsum5_Pb_index = rec.indexOf("partsum5_Pb");
        int partsum1_Hg_index = rec.indexOf("partsum1_Hg");
        int partsum2_Hg_index = rec.indexOf("partsum2_Hg");
        int partsum3_Hg_index = rec.indexOf("partsum3_Hg");
        int partsum4_Hg_index = rec.indexOf("partsum4_Hg");
        int partsum5_Hg_index = rec.indexOf("partsum5_Hg");
        int partsum1_As_pes_index = rec.indexOf("partsum1_As_pes");
        int partsum2_As_pes_index = rec.indexOf("partsum2_As_pes");
        int partsum3_As_pes_index = rec.indexOf("partsum3_As_pes");
        int partsum4_As_pes_index = rec.indexOf("partsum4_As_pes");
        int partsum5_As_pes_index = rec.indexOf("partsum5_As_pes");
        int partsum1_As_gli_index = rec.indexOf("partsum1_As_gli");
        int partsum2_As_gli_index = rec.indexOf("partsum2_As_gli");
        int partsum3_As_gli_index = rec.indexOf("partsum3_As_gli");
        int partsum4_As_gli_index = rec.indexOf("partsum4_As_gli");
        int partsum5_As_gli_index = rec.indexOf("partsum5_As_gli");

        int allsum_index = rec.indexOf("allsum");
        int ves_Cd_index = rec.indexOf("ves_Cd");
        int ves_Pb_index = rec.indexOf("ves_Pb");
        int ves_Hg_index = rec.indexOf("ves_Hg");
        int ves_As_index = rec.indexOf("ves_As");

        while (main_query.next()){

            QVariant partsum1_Cd = main_query.value(partsum1_Cd_index);
            QVariant partsum2_Cd = main_query.value(partsum2_Cd_index);
            QVariant partsum3_Cd = main_query.value(partsum3_Cd_index);
            QVariant partsum4_Cd = main_query.value(partsum4_Cd_index);
            QVariant partsum5_Cd = main_query.value(partsum5_Cd_index);

            QVariant partsum1_Pb = main_query.value(partsum1_Pb_index);
            QVariant partsum2_Pb = main_query.value(partsum2_Pb_index);
            QVariant partsum3_Pb = main_query.value(partsum3_Pb_index);
            QVariant partsum4_Pb = main_query.value(partsum4_Pb_index);
            QVariant partsum5_Pb = main_query.value(partsum5_Pb_index);

            QVariant partsum1_Hg = main_query.value(partsum1_Hg_index);
            QVariant partsum2_Hg = main_query.value(partsum2_Hg_index);
            QVariant partsum3_Hg = main_query.value(partsum3_Hg_index);
            QVariant partsum4_Hg = main_query.value(partsum4_Hg_index);
            QVariant partsum5_Hg = main_query.value(partsum5_Hg_index);

            QVariant partsum1_As_pes = main_query.value(partsum1_As_pes_index);
            QVariant partsum2_As_pes = main_query.value(partsum2_As_pes_index);
            QVariant partsum3_As_pes = main_query.value(partsum3_As_pes_index);
            QVariant partsum4_As_pes = main_query.value(partsum4_As_pes_index);
            QVariant partsum5_As_pes = main_query.value(partsum5_As_pes_index);

            QVariant partsum1_As_gli = main_query.value(partsum1_As_gli_index);
            QVariant partsum2_As_gli = main_query.value(partsum2_As_gli_index);
            QVariant partsum3_As_gli = main_query.value(partsum3_As_gli_index);
            QVariant partsum4_As_gli = main_query.value(partsum4_As_gli_index);
            QVariant partsum5_As_gli = main_query.value(partsum5_As_gli_index);

            QVariant allsum = main_query.value(allsum_index);
            QVariant ves_Cd = main_query.value(ves_Cd_index);
            QVariant ves_Pb = main_query.value(ves_Pb_index);
            QVariant ves_Hg = main_query.value(ves_Hg_index);
            QVariant ves_As = main_query.value(ves_As_index);

            int granul = main_query.value(1).toInt();
            QString tip_name = main_query.value(2).toString();
            QString granul_name = main_query.value(3).toString();

            QString arg2 = "<td align=center>"+russian.toString(ves_Cd.toFloat()/allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum1_Cd.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum2_Cd.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum3_Cd.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum4_Cd.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum5_Cd.toFloat(), 'f', 2)+
                    "</td>";

            QString arg3 = "<td align=center>"+russian.toString(ves_Pb.toFloat()/allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum1_Pb.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum2_Pb.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum3_Pb.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum4_Pb.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum5_Pb.toFloat(), 'f', 2)+
                    "</td>";

            QString arg5_pes = "<td align=center>"+russian.toString(ves_As.toFloat()/allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum1_As_pes.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum2_As_pes.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum3_As_pes.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum4_As_pes.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum5_As_pes.toFloat(), 'f', 2)+
                    "</td>";

            QString arg5_gli = "<td align=center>"+russian.toString(ves_As.toFloat()/allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum1_As_gli.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum2_As_gli.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum3_As_gli.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum4_As_gli.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum5_As_gli.toFloat(), 'f', 2)+
                    "</td>";

            QString arg6 = "<td align=center>"+russian.toString(ves_Hg.toFloat()/allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(allsum.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum1_Hg.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum2_Hg.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum3_Hg.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum4_Hg.toFloat(), 'f', 2)+
                    "</td><td align=center>"+russian.toString(partsum5_Hg.toFloat(), 'f', 2)+
                    "</td>";

            if((granul==6)||(granul==7)){
                string = string + main_str.arg(tip_name+" "+granul_name, arg2, arg3, "2,0", arg5_pes, arg6);
            } else {
                string = string + main_str.arg(tip_name+" "+granul_name, arg2, arg3, "10,0", arg5_gli, arg6);
            }





        }
    }
    string = string + "</table>";
    return string;
}

QString ReportDialog::Pb_report(QString ids)
{
    return Cd_report(ids);
}

QString ReportDialog::Hg_report(QString ids)
{
    return Cd_report(ids);
}

QString ReportDialog::As_report(QString ids)
{
    return Cd_report(ids);
}

QString ReportDialog::report(QModelIndexList index, QString ids){
    QString result("");
    for (int i = 0; i < index.count(); i++) {
        if (index.at(i).parent() != QModelIndex()){
            switch (index.at(i).row()){
                case 0:
                    result = result + "<br>" + N_report(ids);
                    break;
                case 1:
                    result = result + "<br>" + P_report(ids);
                    break;
                case 2:
                    result = result + "<br>" + K_report(ids);
                    break;
                case 3:
                    result = result + "<br>" + S_report(ids);
                    break;
                case 4:
                    result = result + "<br>" + Org_report(ids);
                    break;
                case 5:
                    result = result + "<br>" + PH_report(ids);
                    break;
                case 6:
                    result = result + "<br>" + Ca_report(ids);
                    break;
                case 7:
                    result = result + "<br>" + Mg_report(ids);
                    break;
                case 8:
                    result = result + "<br>" + Zn_report(ids);
                    break;
                case 9:
                    result = result + "<br>" + Cu_report(ids);
                    break;
                case 10:
                    result = result + "<br>" + Mo_report(ids);
                    break;
                case 11:
                    result = result + "<br>" + B_report(ids);
                    break;
                case 12:
                    result = result + "<br>" + granul_report(ids);
                    break;
                case 13:
                    result = result + "<br>" + REKO_report(ids);
                    break;
                case 14:
                    result = result + "<br>" + Cd_report(ids);
                    break;
                case 15:
                    result = result + "<br>" + Pb_report(ids);
                    break;
                case 16:
                    result = result + "<br>" + Hg_report(ids);
                    break;
                case 17:
                    result = result + "<br>" + As_report(ids);
                    break;
             }
        }
    }
    return result;
}

QString ReportDialog::N_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 1. Группировка почв по содержанию щелочногидролизуемого азота </b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Средне-<br>взвешен.<br>мг/кг </td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=8>содержание щелочногидролизуемого азота по классам</td></tr>"
                    "<tr><td align=center colspan=2>очень низкое</td><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>повышенное</td></tr>"
                    "<tr><td align=center colspan=2> &#60 100 мг/кг </td><td align=center colspan=2>101-150 мг/кг</td><td align=center colspan=2>151-200 мг/кг</td><td align=center colspan=2> &#62 200 мг/кг</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(N*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.N<=100 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.N>100 and main.N<=150 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.N>150 and main.N<=200 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.N>200 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(N*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.N<=100 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.N>100 and main.N<=150 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.N>150 and main.N<=200 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.N>200 and (main.smo in (%1))) as partsum4 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();

        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();

    string = string + "</table>";
    return string;
}

QString ReportDialog::P_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 2. Группировка почв по содержанию подвижного фосфора</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Средне-<br>взвешен.<br>мг/кг </td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=12>содержание подвижного фосфора по классам</td></tr>"
                    "<tr><td align=center colspan=2>очень низкое</td><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>повышенное</td><td align=center colspan=2>высокое</td><td align=center colspan=2>очень высокое</td></tr>"
                    "<tr><td align=center colspan=2> &#60 10 мг/кг  </td><td align=center colspan=2>11-15 мг/кг</td><td align=center colspan=2>16-30 мг/кг</td><td align=center colspan=2>31-45 мг/кг</td><td align=center colspan=2>46-60 мг/кг</td><td align=center colspan=2> &#62 60 мг/кг</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td><td align=center>13</td><td align=center>14</td><td align=center>15</td><td align=center>16</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(P*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.P<=10 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.P>10 and main.P<=15 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.P>15 and main.P<=30 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.P>30 and main.P<=45 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join (select tip as tip5, sum(s_pochv_kont) as partsum from main where  main.P>45 and main.P<=60 and (main.smo in (%1)) "
                   "group by tip5) as table5 "
                   "on (main.tip=table5.tip5) "
                   "left join (select tip as tip6, sum(s_pochv_kont) as partsum from main where  main.P>60 and (main.smo in (%1)) "
                   "group by tip6) as table6 "
                   "on (main.tip=table6.tip6) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(P*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.P<=10 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.P>10 and main.P<=15 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.P>15 and main.P<=30 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.P>30 and main.P<=45 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.P>45 and main.P<=60 and (main.smo in (%1))) as partsum5, "
                   "(select sum(s_pochv_kont) from main where  main.P>60 and (main.smo in (%1))) as partsum6 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int partsum6_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float partsum6;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                partsum6 = all_query.value(partsum6_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    partsum6 = main_query.value(partsum6_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            partsum6 = all_query.value(partsum6_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                partsum6 = main_query.value(partsum6_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::K_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 3. Группировка почв по содержанию обменного калия</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Средне-<br>взвешен.<br>мг/кг </td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=12>содержание подвижного фосфора по классам</td></tr>"
                    "<tr><td align=center colspan=2>очень низкое</td><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>повышенное</td><td align=center colspan=2>высокое</td><td align=center colspan=2>очень высокое</td></tr>"
                    "<tr><td align=center colspan=2> &#60 100 мг/кг  </td><td align=center colspan=2>101-200 мг/кг</td><td align=center colspan=2>201-300 мг/кг</td><td align=center colspan=2>301-400 мг/кг</td><td align=center colspan=2>401-600 мг/кг</td><td align=center colspan=2> &#62 600 мг/кг</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td><td align=center>13</td><td align=center>14</td><td align=center>15</td><td align=center>16</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(K*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.K<=100 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.K>100 and main.K<=200 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.K>200 and main.K<=300 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.K>300 and main.K<=400 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join (select tip as tip5, sum(s_pochv_kont) as partsum from main where  main.K>400 and main.K<=600 and (main.smo in (%1)) "
                   "group by tip5) as table5 "
                   "on (main.tip=table5.tip5) "
                   "left join (select tip as tip6, sum(s_pochv_kont) as partsum from main where  main.K>600 and (main.smo in (%1)) "
                   "group by tip6) as table6 "
                   "on (main.tip=table6.tip6) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(K*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.K<=100 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.K>100 and main.K<=200 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.K>200 and main.K<=300 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.K>300 and main.K<=400 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.K>400 and main.K<=600 and (main.smo in (%1))) as partsum5, "
                   "(select sum(s_pochv_kont) from main where  main.K>600 and (main.smo in (%1))) as partsum6 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int partsum6_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float partsum6;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                partsum6 = all_query.value(partsum6_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    partsum6 = main_query.value(partsum6_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            partsum6 = all_query.value(partsum6_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                partsum6 = main_query.value(partsum6_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::S_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 5. Группировка почв по содержанию подвижной серы</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Средне-<br>взвешен.<br>мг/кг </td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=6>содержание подвижной серы по классам</td></tr>"
                    "<tr><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>высокое</td></tr>"
                    "<tr><td align=center colspan=2> &#60 6 мг/кг   </td><td align=center colspan=2>6-12 мг/кг</td><td align=center colspan=2> &#62 12 мг/кг</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(S*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.S<=6 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.S>6 and main.S<=12 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.S>12 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(S*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.S<=6 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.S>6 and main.S<=12 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.S>12 and (main.smo in (%1))) as partsum3 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Org_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 4. Группировка почв по содержанию органического вещества</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=3>Наименование<br>СМО</td><td align=center rowspan=3>Вид<br>сельхозугодья</td><td align=center rowspan=3>Средне-<br>взвешен.<br>%</td><td align=center rowspan=3>Обслед. <br>площадь<br>га.</td><td align=center colspan=18>Группировка почв по содержанию органического вещества</td></tr>"
                    "<tr><td align=center colspan=2> &#60 1,0</td><td align=center colspan=2>1,01-1,5 </td><td align=center colspan=2>1,51-2,0 </td><td align=center colspan=2>2,01-2,5 </td><td align=center colspan=2>2,51-3,0 </td><td align=center colspan=2>3,01-4,0 </td><td align=center colspan=2>4,01-6,0 </td><td align=center colspan=2>6,01-8,0 </td><td align=center colspan=2>8,01-10,0 </td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td><td align=center>13</td><td align=center>14</td><td align=center>15</td><td align=center>16</td><td align=center>17</td><td align=center>18</td><td align=center>19</td><td align=center>20</td><td align=center>21</td><td align=center>22</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(Org*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, "
                   "table5.partsum as partsum5, table6.partsum as partsum6, table7.partsum as partsum7, table8.partsum as partsum8, table9.partsum as partsum9 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.Org<=1 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.Org>1 and main.Org<=1.5 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.Org>1.5 and main.Org<=2 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.Org>2 and main.Org<=2.5 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join (select tip as tip5, sum(s_pochv_kont) as partsum from main where  main.Org>2.5 and main.Org<=3 and (main.smo in (%1)) "
                   "group by tip5) as table5 "
                   "on (main.tip=table5.tip5) "
                   "left join (select tip as tip6, sum(s_pochv_kont) as partsum from main where  main.Org>3 and main.Org<=4 and (main.smo in (%1)) "
                   " group by tip6) as table6 "
                   "on (main.tip=table6.tip6) "
                   "left join (select tip as tip7, sum(s_pochv_kont) as partsum from main where  main.Org>4 and main.Org<=6 and (main.smo in (%1)) "
                   "group by tip7) as table7 "
                   "on (main.tip=table7.tip7) "
                   "left join (select tip as tip8, sum(s_pochv_kont) as partsum from main where  main.Org>6 and main.Org<=8 and (main.smo in (%1)) "
                   "group by tip8) as table8 "
                   "on (main.tip=table8.tip8) "
                   "left join (select tip as tip9, sum(s_pochv_kont) as partsum from main where  main.Org>8 and main.Org<=10 and (main.smo in (%1)) "
                   "group by tip9) as table9 "
                   "on (main.tip=table9.tip9) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(Org*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Org<=1 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Org>1 and main.Org<=1.5 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Org>1.5 and main.Org<=2 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.Org>2 and main.Org<=2.5 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.Org>2.5 and main.Org<=3 and (main.smo in (%1))) as partsum5, "
                   "(select sum(s_pochv_kont) from main where  main.Org>3 and main.Org<=4 and (main.smo in (%1))) as partsum6, "
                   "(select sum(s_pochv_kont) from main where  main.Org>4 and main.Org<=6 and (main.smo in (%1))) as partsum7, "
                   "(select sum(s_pochv_kont) from main where  main.Org>6 and main.Org<=8 and (main.smo in (%1))) as partsum8, "
                   "(select sum(s_pochv_kont) from main where  main.Org>8 and main.Org<=10 and (main.smo in (%1))) as partsum9 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int partsum6_index;
    int partsum7_index;
    int partsum8_index;
    int partsum9_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float partsum6;
    float partsum7;
    float partsum8;
    float partsum9;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                partsum7_index = rec.indexOf("partsum7");
                partsum8_index = rec.indexOf("partsum8");
                partsum9_index = rec.indexOf("partsum9");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                partsum6 = all_query.value(partsum6_index).toFloat();
                partsum7 = all_query.value(partsum7_index).toFloat();
                partsum8 = all_query.value(partsum8_index).toFloat();
                partsum9 = all_query.value(partsum9_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum7, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum7/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum8, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum8/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum9, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum9/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum7, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum7/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum8, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum8/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum9, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum9/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            partsum7_index = rec.indexOf("partsum7");
            partsum8_index = rec.indexOf("partsum8");
            partsum9_index = rec.indexOf("partsum9");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            partsum6 = all_query.value(partsum6_index).toFloat();
            partsum7 = all_query.value(partsum7_index).toFloat();
            partsum8 = all_query.value(partsum8_index).toFloat();
            partsum9 = all_query.value(partsum9_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum7, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum7/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum8, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum8/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum9, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum9/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum7, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum7/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum8, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum8/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum9, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum9/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::PH_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 6. Группировка почв по степени щелочности (кислотности) </b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Средне-<br>взвешен.<br>мг/кг </td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=10>Группировка почв по степени щелочности</td></tr>"
                    "<tr><td align=center colspan=2>Близко к нейтр.</td><td align=center colspan=2>Нейтральные</td><td align=center colspan=2>Слабощелочные</td><td align=center colspan=2>Щелочные</td><td align=center colspan=2>Сильнощелочные</td></tr>"
                    "<tr><td align=center colspan=2>5,6-6,5</td><td align=center colspan=2>6,6-7,5</td><td align=center colspan=2>7,6-8,5</td><td align=center colspan=2>8,6-9,0</td><td align=center colspan=2>9,1-11,0</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td><td align=center>13</td><td align=center>14</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(ph*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.ph > 5.5 and main.ph <= 6.5 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.ph > 6.5 and main.ph <= 7.5 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.ph > 7.5 and main.ph <= 8.5 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.ph > 8.5 and main.ph <= 9 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join (select tip as tip5, sum(s_pochv_kont) as partsum from main where  main.ph > 9 and main.ph <= 11 and (main.smo in (%1)) "
                   "group by tip5) as table5 "
                   "on (main.tip=table5.tip5) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(ph*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.ph > 5.5 and main.ph <= 6.5 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.ph > 6.5 and main.ph <= 7.5 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.ph > 7.5 and main.ph <= 8.5 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.ph > 8.5 and main.ph <= 9 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.ph > 9 and main.ph <= 11 and (main.smo in (%1))) as partsum5 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Ca_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 7. Группировка почв по содержанию обменного кальция</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Средне-<br>взвешен.<br>содер.<br>мг-экв/100г<br>почвы</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=12>Группировка почв по содержанию обменного кальция</td></tr>"
                    "<tr><td align=center colspan=2>очень низкое</td><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>повышенное</td><td align=center colspan=2>высокое</td><td align=center colspan=2>очень высокое</td></tr>"
                    "<tr><td align=center colspan=2> &#8804 2,5мг-<br>экв/100г</td><td align=center colspan=2>2,6-5,0мг-<br>экв/100г</td><td align=center colspan=2>5,1-10мг-<br>экв/100г</td><td align=center colspan=2>10,1-15,0мг-<br>экв/100г</td><td align=center colspan=2>15,1-20мг-<br>экв/100г</td><td align=center colspan=2> &#62 20 мг-<br>экв/100г</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td><td align=center>13</td><td align=center>14</td><td align=center>15</td><td align=center>16</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(Ca*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.Ca <= 2.5 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.Ca > 2.5 and main.Ca <= 5 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.Ca > 5 and main.Ca <= 10 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.Ca > 10 and main.Ca <= 15 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join (select tip as tip5, sum(s_pochv_kont) as partsum from main where  main.Ca > 15 and main.Ca <= 20 and (main.smo in (%1)) "
                   "group by tip5) as table5 "
                   "on (main.tip=table5.tip5) "
                   "left join (select tip as tip6, sum(s_pochv_kont) as partsum from main where  main.Ca > 20 and (main.smo in (%1)) "
                   "group by tip6) as table6 "
                   "on (main.tip=table6.tip6) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(Ca*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Ca <= 2.5 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Ca > 2.5 and main.Ca <= 5 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Ca > 5 and main.Ca <= 10 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.Ca > 10 and main.Ca <= 15 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.Ca > 15 and main.Ca <= 20 and (main.smo in (%1))) as partsum5, "
                   "(select sum(s_pochv_kont) from main where  main.Ca > 20 and (main.smo in (%1))) as partsum6 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int partsum6_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float partsum6;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                partsum6 = all_query.value(partsum6_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    partsum6 = main_query.value(partsum6_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            partsum6 = all_query.value(partsum6_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                partsum6 = main_query.value(partsum6_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Mg_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 8. Группировка почв по содержанию обменного магния</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Средне-<br>взвешен.<br>содер.<br>мг-экв/100г<br>почвы</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=12>Группировка почв по содержанию обменного магния</td></tr>"
                    "<tr><td align=center colspan=2>очень низкое</td><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>повышенное</td><td align=center colspan=2>высокое</td><td align=center colspan=2>очень высокое</td></tr>"
                    "<tr><td align=center colspan=2> &#8804 0,5мг-<br>экв/100г</td><td align=center colspan=2>0,6-1,0мг-<br>экв/100г</td><td align=center colspan=2>1,1-2,0мг-<br>экв/100г</td><td align=center colspan=2>2,1-3,0мг-<br>экв/100г</td><td align=center colspan=2>3,1-4,0мг-<br>экв/100г</td><td align=center colspan=2> &#62 4,0 мг-<br>экв/100г</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td><td align=center>13</td><td align=center>14</td><td align=center>15</td><td align=center>16</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(Mg*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.Mg <= 0.5 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.Mg > 0.5 and main.Mg <= 1 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.Mg > 1 and main.Mg <= 2 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.Mg > 2 and main.Mg <= 3 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join (select tip as tip5, sum(s_pochv_kont) as partsum from main where  main.Mg > 3 and main.Mg <= 4 and (main.smo in (%1)) "
                   "group by tip5) as table5 "
                   "on (main.tip=table5.tip5) "
                   "left join (select tip as tip6, sum(s_pochv_kont) as partsum from main where  main.Mg > 4 and (main.smo in (%1)) "
                   "group by tip6) as table6 "
                   "on (main.tip=table6.tip6) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(Mg*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Mg <= 0.5 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Mg > 0.5 and main.Mg <= 1 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Mg > 1 and main.Mg <= 2 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.Mg > 2 and main.Mg <= 3 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.Mg > 3 and main.Mg <= 4 and (main.smo in (%1))) as partsum5, "
                   "(select sum(s_pochv_kont) from main where  main.Mg > 4 and (main.smo in (%1))) as partsum6 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int partsum6_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float partsum6;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                partsum6 = all_query.value(partsum6_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    partsum6 = main_query.value(partsum6_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            partsum6 = all_query.value(partsum6_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                partsum6 = main_query.value(partsum6_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::REKO_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 9. Группировка почв по ёмкости катионного обмена</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Средне-<br>взвешен.<br>содер.<br>мг-экв/100г<br>почвы</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=12>Группировка почв по ёмкости катионного обмена</td></tr>"
                    "<tr><td align=center colspan=2>очень низкое</td><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>повышенное</td><td align=center colspan=2>высокое</td><td align=center colspan=2>очень высокое</td></tr>"
                    "<tr><td align=center colspan=2> &#8804 5мг-<br>экв/100г</td><td align=center colspan=2>5,1-10,0мг-<br>экв/100г</td><td align=center colspan=2>10,1-15,0мг-<br>экв/100г</td><td align=center colspan=2>15,1-20,0мг-<br>экв/100г</td><td align=center colspan=2>20,1-30,0мг-<br>экв/100г</td><td align=center colspan=2> &#62 30 мг-<br>экв/100г</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td><td align=center>13</td><td align=center>14</td><td align=center>15</td><td align=center>16</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(Reko*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.Reko <= 5 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.Reko > 5 and main.Reko <= 10 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.Reko > 10 and main.Reko <= 15 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.Reko > 15 and main.Reko <= 20 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join (select tip as tip5, sum(s_pochv_kont) as partsum from main where  main.Reko > 20 and main.Reko <= 30 and (main.smo in (%1)) "
                   "group by tip5) as table5 "
                   "on (main.tip=table5.tip5) "
                   "left join (select tip as tip6, sum(s_pochv_kont) as partsum from main where  main.Reko > 30 and (main.smo in (%1)) "
                   "group by tip6) as table6 "
                   "on (main.tip=table6.tip6) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(Reko*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Reko <= 5 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Reko > 5 and main.Reko <= 10 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Reko > 10 and main.Reko <= 15 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.Reko > 15 and main.Reko <= 20 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.Reko > 20 and main.Reko <= 30 and (main.smo in (%1))) as partsum5, "
                   "(select sum(s_pochv_kont) from main where  main.Reko > 30 and (main.smo in (%1))) as partsum6 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int partsum6_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float partsum6;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                partsum6 = all_query.value(partsum6_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    partsum6 = main_query.value(partsum6_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            partsum6 = all_query.value(partsum6_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                partsum6 = main_query.value(partsum6_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Zn_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 11. Группировка почв по содержанию микроэлементов (цинк)</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=7>Цинк</td></tr>"
                    "<tr><td align=center rowspan=3>ср.взв.<br>мг/кг</td><td align=center colspan=2>&#60 2,0</td><td align=center colspan=2>2,1-5,0</td><td align=center colspan=2>&#62 5,0</td></tr>"
                    "<tr><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>высокое</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(Zn*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.Zn <= 2 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.Zn > 2 and main.Zn <= 5 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.Zn > 5 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(Zn*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Zn <= 2 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Zn > 2 and main.Zn <= 5 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Zn > 5 and (main.smo in (%1))) as partsum3 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Cu_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 11. Группировка почв по содержанию микроэлементов (медь)</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=7>Медь</td></tr>"
                    "<tr><td align=center rowspan=3>ср.взв.<br>мг/кг</td><td align=center colspan=2>&#60 0,20</td><td align=center colspan=2>0,21-0,50</td><td align=center colspan=2>&#62 0,50</td></tr>"
                    "<tr><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>высокое</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(Cu*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.Cu <= 0.2 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.Cu > 0.2 and main.Cu <= 0.5 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.Cu > 0.5 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(Cu*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Cu <= 0.2 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Cu > 0.2 and main.Cu <= 0.5 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Cu > 0.5 and (main.smo in (%1))) as partsum3 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Mo_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 11. Группировка почв по содержанию микроэлементов (молибден)</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=7>Молибден</td></tr>"
                    "<tr><td align=center rowspan=3>ср.взв.<br>мг/кг</td><td align=center colspan=2>&#60 0,10</td><td align=center colspan=2>0,11-0,22</td><td align=center colspan=2>&#62 0,22</td></tr>"
                    "<tr><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>высокое</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(Mo*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.Mo <= 0.1 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.Mo > 0.1 and main.Mo <= 0.22 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.Mo > 0.22 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(Mo*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Mo <= 0.1 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Mo > 0.1 and main.Mo <= 0.22 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Mo > 0.22 and (main.smo in (%1))) as partsum3 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::B_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 11. Группировка почв по содержанию микроэлементов (бор)</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=7>Бор</td></tr>"
                    "<tr><td align=center rowspan=3>ср.взв.<br>мг/кг</td><td align=center colspan=2>&#60 0,33</td><td align=center colspan=2>0,34-0,70</td><td align=center colspan=2>&#62 0,70</td></tr>"
                    "<tr><td align=center colspan=2>низкое</td><td align=center colspan=2>среднее</td><td align=center colspan=2>высокое</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(B*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.B <= 0.33 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.B > 0.33 and main.B <= 0.7 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.B > 0.7 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(B*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.B <= 0.33 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.B > 0.33 and main.B <= 0.7 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.B > 0.7 and (main.smo in (%1))) as partsum3 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::granul_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 10. Гранулометрический состав почв</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование СМО<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=12>Группировка почв по гранулометрическому составу</td></tr>"
                    "<tr><td align=center colspan=2>глинистые</td><td align=center colspan=2>тяжелосуглинист.</td><td align=center colspan=2>среднесуглинист.</td><td align=center colspan=2>легкосуглинист.</td><td align=center colspan=2>супесчаные</td><td align=center colspan=2>песчаные</td></tr>"
                    "<tr><td align=center colspan=2> > 60%<br>физической глины</td><td align=center colspan=2>45-60%<br>физической глины</td><td align=center colspan=2>30-45%<br>физической глины</td><td align=center colspan=2>20-30%<br>физической глины</td><td align=center colspan=2>10-20%<br>физической глины</td><td align=center colspan=2> >10%<br>физической глины</td></tr>"
                    "<tr><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td><td align=center>га</td><td align=center>%</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td><td align=center>6</td><td align=center>7</td><td align=center>8</td><td align=center>9</td><td align=center>10</td><td align=center>11</td><td align=center>12</td><td align=center>13</td><td align=center>14</td><td align=center>15</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, table6.partsum as partsum6 from main "
                   "left join (select tip as tip1, sum(s_pochv_kont) as partsum from main where  main.granul = 5 and (main.smo in (%1)) "
                   "group by tip1) as table1 "
                   "on (main.tip=table1.tip1) "
                   "left join (select tip as tip2, sum(s_pochv_kont) as partsum from main where  main.granul = 3 and (main.smo in (%1)) "
                   " group by tip2) as table2 "
                   " on (main.tip=table2.tip2) "
                   "left join (select tip as tip3, sum(s_pochv_kont) as partsum from main where  main.granul = 2 and (main.smo in (%1)) "
                   "group by tip3) as table3 "
                   "on (main.tip=table3.tip3) "
                   "left join (select tip as tip4, sum(s_pochv_kont) as partsum from main where  main.granul = 4 and (main.smo in (%1)) "
                   "group by tip4) as table4 "
                   "on (main.tip=table4.tip4) "
                   "left join (select tip as tip5, sum(s_pochv_kont) as partsum from main where  main.granul = 7 and (main.smo in (%1)) "
                   "group by tip5) as table5 "
                   "on (main.tip=table5.tip5) "
                   "left join (select tip as tip6, sum(s_pochv_kont) as partsum from main where  main.granul = 6 and (main.smo in (%1)) "
                   "group by tip6) as table6 "
                   "on (main.tip=table6.tip6) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "where main.smo in (%1) "
                   "group by main.tip "
                   "order by main.tip ");

    QString zapros_all("select sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.granul = 5 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.granul = 3 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.granul = 2 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.granul = 4 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.granul = 7 and (main.smo in (%1))) as partsum5, "
                   "(select sum(s_pochv_kont) from main where  main.granul = 6 and (main.smo in (%1))) as partsum6 "
                   "from main "
                   "where main.smo in (%1) ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int partsum6_index;
    int allsum_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float partsum6;
    float allsum;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                partsum6 = all_query.value(partsum6_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                allsum_index = rec.indexOf("allsum");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    partsum6 = main_query.value(partsum6_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5/allsum*100, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6/allsum*100, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            partsum6 = all_query.value(partsum6_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td align=center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5/allsum*100, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum6/allsum*100, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            allsum_index = rec.indexOf("allsum");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                partsum6 = main_query.value(partsum6_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5/allsum*100, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum6/allsum*100, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Cd_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 12. Группировка почв по содержанию подвижных форм тяжелых металлов, мышьяка и ртути (кадмий)</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Грануломет-<br>рический<br>состав</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=6>Кадмий</td></tr>"
                    "<tr><td align=center rowspan=3>ср.взв.<br>мг/кг</td><td align=center colspan=5>Группы по содержанию ТМ</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td></tr>"
                    "<tr><td align=center>га</td><td align=center>га</td><td align=center>га</td><td align=center>га</td><td align=center>га</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, granul_table.name, sum(Cd*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5 from main "
                   "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.Cd IS NOT NULL and main.Cd < 1 and (main.smo in (%1)) "
                   "group by tip1, granul1) as table1 "
                   "on (main.tip=table1.tip1 and main.granul=table1.granul1) "
                   "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.Cd IS NOT NULL and main.Cd >= 1 and main.Cd <= 2 and (main.smo in (%1)) "
                   " group by tip2, granul2) as table2 "
                   " on (main.tip=table2.tip2 and main.granul=table2.granul2) "
                   "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.Cd IS NOT NULL and main.Cd > 2 and main.Cd <= 4 and (main.smo in (%1)) "
                   "group by tip3, granul3) as table3 "
                   "on (main.tip=table3.tip3 and main.granul=table3.granul3) "
                   "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.Cd IS NOT NULL and main.Cd > 4 and main.Cd <= 6 and (main.smo in (%1)) "
                   "group by tip4, granul4) as table4 "
                   "on (main.tip=table4.tip4 and main.granul=table4.granul4) "
                   "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.Cd IS NOT NULL and main.Cd > 6 and (main.smo in (%1)) "
                   "group by tip5, granul5) as table5 "
                   "on (main.tip=table5.tip5 and main.granul=table5.granul5) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "left join granul AS granul_table "
                   "on granul_table.id_granul=main.granul "
                   "where main.smo in (%1) and main.Cd IS NOT NULL "
                   "group by main.tip, main.granul "
                   "order by main.tip, main.granul ");

    QString zapros_all("select sum(Cd*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Cd IS NOT NULL and main.Cd < 1 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Cd IS NOT NULL and main.Cd >= 1 and main.Cd <= 2 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Cd IS NOT NULL and main.Cd > 2 and main.Cd <= 4 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.Cd IS NOT NULL and main.Cd > 4 and main.Cd <= 6 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.Cd IS NOT NULL and main.Cd > 6 and (main.smo in (%1))) as partsum5 "
                   "from main "
                   "where main.smo in (%1) and main.Cd IS NOT NULL ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td></td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td></td><td align=center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Pb_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 12. Группировка почв по содержанию подвижных форм тяжелых металлов, мышьяка и ртути (свинец)</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Грануломет-<br>рический<br>состав</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=6>Свинец</td></tr>"
                    "<tr><td align=center rowspan=3>ср.взв.<br>мг/кг</td><td align=center colspan=5>Группы по содержанию ТМ</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td></tr>"
                    "<tr><td align=center>га</td><td align=center>га</td><td align=center>га</td><td align=center>га</td><td align=center>га</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, granul_table.name, sum(Pb*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5 from main "
                   "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.Pb IS NOT NULL and main.Pb < 3 and (main.smo in (%1)) "
                   "group by tip1, granul1) as table1 "
                   "on (main.tip=table1.tip1 and main.granul=table1.granul1) "
                   "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.Pb IS NOT NULL and main.Pb >= 3 and main.Pb <= 6 and (main.smo in (%1)) "
                   " group by tip2, granul2) as table2 "
                   " on (main.tip=table2.tip2 and main.granul=table2.granul2) "
                   "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.Pb IS NOT NULL and main.Pb > 6 and main.Pb <= 12 and (main.smo in (%1)) "
                   "group by tip3, granul3) as table3 "
                   "on (main.tip=table3.tip3 and main.granul=table3.granul3) "
                   "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.Pb IS NOT NULL and main.Pb > 12 and main.Pb <= 18 and (main.smo in (%1)) "
                   "group by tip4, granul4) as table4 "
                   "on (main.tip=table4.tip4 and main.granul=table4.granul4) "
                   "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.Pb IS NOT NULL and main.Pb > 18 and (main.smo in (%1)) "
                   "group by tip5, granul5) as table5 "
                   "on (main.tip=table5.tip5 and main.granul=table5.granul5) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "left join granul AS granul_table "
                   "on granul_table.id_granul=main.granul "
                   "where main.smo in (%1) and main.Pb IS NOT NULL "
                   "group by main.tip, main.granul "
                   "order by main.tip, main.granul ");

    QString zapros_all("select sum(Pb*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Pb IS NOT NULL and main.Pb < 3 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Pb IS NOT NULL and main.Pb >= 3 and main.Pb <= 6 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Pb IS NOT NULL and main.Pb > 6 and main.Pb <= 12 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.Pb IS NOT NULL and main.Pb > 12 and main.Pb <= 18 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.Pb IS NOT NULL and main.Pb > 18 and (main.smo in (%1))) as partsum5 "
                   "from main "
                   "where main.smo in (%1) and main.Pb IS NOT NULL ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td></td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td></td><td align=center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::Hg_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 12. Группировка почв по содержанию подвижных форм тяжелых металлов, мышьяка и ртути (ртуть)</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Грануломет-<br>рический<br>состав</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=6>Ртуть</td></tr>"
                    "<tr><td align=center rowspan=3>ср.взв.<br>мг/кг</td><td align=center colspan=5>Группы по содержанию ТМ</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td></tr>"
                    "<tr><td align=center>га</td><td align=center>га</td><td align=center>га</td><td align=center>га</td><td align=center>га</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, granul_table.name, sum(Hg*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5 from main "
                   "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.Hg IS NOT NULL and main.Hg < 1 and (main.smo in (%1)) "
                   "group by tip1, granul1) as table1 "
                   "on (main.tip=table1.tip1 and main.granul=table1.granul1) "
                   "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.Hg IS NOT NULL and main.Hg >= 1 and main.Hg <= 2.1 and (main.smo in (%1)) "
                   " group by tip2, granul2) as table2 "
                   " on (main.tip=table2.tip2 and main.granul=table2.granul2) "
                   "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.Hg IS NOT NULL and main.Hg > 2.1 and main.Hg <= 4.2 and (main.smo in (%1)) "
                   "group by tip3, granul3) as table3 "
                   "on (main.tip=table3.tip3 and main.granul=table3.granul3) "
                   "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.Hg IS NOT NULL and main.Hg > 4.2 and main.Hg <= 6.2 and (main.smo in (%1)) "
                   "group by tip4, granul4) as table4 "
                   "on (main.tip=table4.tip4 and main.granul=table4.granul4) "
                   "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.Hg IS NOT NULL and main.Hg > 6.2 and (main.smo in (%1)) "
                   "group by tip5, granul5) as table5 "
                   "on (main.tip=table5.tip5 and main.granul=table5.granul5) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "left join granul AS granul_table "
                   "on granul_table.id_granul=main.granul "
                   "where main.smo in (%1) and main.Hg IS NOT NULL "
                   "group by main.tip, main.granul "
                   "order by main.tip, main.granul ");

    QString zapros_all("select sum(Hg*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  main.Hg IS NOT NULL and main.Hg < 1 and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  main.Hg IS NOT NULL and main.Hg >= 1 and main.Hg <= 2.1 and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  main.Hg IS NOT NULL and main.Hg > 2.1 and main.Hg <= 4.2 and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  main.Hg IS NOT NULL and main.Hg > 4.2 and main.Hg <= 6.2 and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  main.Hg IS NOT NULL and main.Hg > 6.2 and (main.smo in (%1))) as partsum5 "
                   "from main "
                   "where main.smo in (%1) and main.Hg IS NOT NULL ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td></td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){

                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td></tr>";

                }
            }
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td></td><td align=center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5, 'f', 2)
                        + "</td></tr>";

            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::As_report_resp(QString reg_id)
{
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Таблица 12. Группировка почв по содержанию подвижных форм тяжелых металлов, мышьяка и ртути (мышьяк)</b></p>"
                    "<table>"
                    "<tr><td align=center rowspan=4>Наименование<br>СМО</td><td align=center rowspan=4>Вид<br>сельхозугодья</td><td align=center rowspan=4>Грануломет-<br>рический<br>состав</td><td align=center rowspan=4>Обслед. <br>площадь<br>га.</td><td align=center colspan=6>Мышьяк</td></tr>"
                    "<tr><td align=center rowspan=3>ср.взв.<br>мг/кг</td><td align=center colspan=5>Группы по содержанию ТМ</td></tr>"
                    "<tr><td align=center>1</td><td align=center>2</td><td align=center>3</td><td align=center>4</td><td align=center>5</td></tr>"
                    "<tr><td align=center>га</td><td align=center>га</td><td align=center>га</td><td align=center>га</td><td align=center>га</td></tr>"
                    );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QString zapros("select main.tip, tip_table.name, granul_table.name, main.granul, sum(main.As*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, table1.partsum as partsum1, table2.partsum as partsum2, table3.partsum as partsum3, table4.partsum as partsum4, table5.partsum as partsum5, "
                   "table6.partsum as partsum6, table7.partsum as partsum7, table8.partsum as partsum8, table9.partsum as partsum9, table10.partsum as partsum10 from main "
                   "left join (select tip as tip1, granul as granul1, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As < 1 and (main.smo in (%1)) "
                   "group by tip1, granul1) as table1 "
                   "on (main.tip=table1.tip1 and main.granul=table1.granul1) "
                   "left join (select tip as tip2, granul as granul2, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As >= 1 and main.As <= 2 and (main.smo in (%1)) "
                   " group by tip2, granul2) as table2 "
                   " on (main.tip=table2.tip2 and main.granul=table2.granul2) "
                   "left join (select tip as tip3, granul as granul3, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As > 2 and main.As <= 4 and (main.smo in (%1)) "
                   "group by tip3, granul3) as table3 "
                   "on (main.tip=table3.tip3 and main.granul=table3.granul3) "
                   "left join (select tip as tip4, granul as granul4, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As > 4 and main.As <= 6 and (main.smo in (%1)) "
                   "group by tip4, granul4) as table4 "
                   "on (main.tip=table4.tip4 and main.granul=table4.granul4) "
                   "left join (select tip as tip5, granul as granul5, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As > 6 and (main.smo in (%1)) "
                   "group by tip5, granul5) as table5 "
                   "on (main.tip=table5.tip5 and main.granul=table5.granul5) "
                   "left join (select tip as tip6, granul as granul6, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As < 5 and (main.smo in (%1)) "
                   "group by tip6, granul6) as table6 "
                   "on (main.tip=table6.tip6 and main.granul=table6.granul6) "
                   "left join (select tip as tip7, granul as granul7, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As >= 5 and main.As <= 10 and (main.smo in (%1)) "
                   "group by tip7, granul7) as table7 "
                   " on (main.tip=table7.tip7 and main.granul=table7.granul7) "
                   "left join (select tip as tip8, granul as granul8, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As > 10 and main.As <= 20 and (main.smo in (%1)) "
                   "group by tip8, granul8) as table8 "
                   "on (main.tip=table8.tip8 and main.granul=table8.granul8) "
                   "left join (select tip as tip9, granul as granul9, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As > 20 and main.As <= 30 and (main.smo in (%1)) "
                   "group by tip9, granul9) as table9 "
                   "on (main.tip=table9.tip9 and main.granul=table9.granul9) "
                   "left join (select tip as tip10, granul as granul10, sum(s_pochv_kont) as partsum from main where  main.As IS NOT NULL and main.As > 30 and (main.smo in (%1)) "
                   "group by tip10, granul10) as table10 "
                   "on (main.tip=table10.tip10 and main.granul=table10.granul10) "
                   "left join tip AS tip_table "
                   "on tip_table.id_tip=main.tip "
                   "left join granul AS granul_table "
                   "on granul_table.id_granul=main.granul "
                   "where main.smo in (%1) and main.As IS NOT NULL "
                   "group by main.tip, main.granul "
                   "order by main.tip, main.granul ");

    QString zapros_all("select sum(main.As*s_pochv_kont) as ves, sum(s_pochv_kont) as allsum, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As < 1 and (main.granul = 6 or main.granul = 7) and (main.smo in (%1))) as partsum1, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As >= 1 and main.As <= 2 and (main.granul = 6 or main.granul = 7) and (main.smo in (%1))) as partsum2, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As > 2 and main.As <= 4 and (main.granul = 6 or main.granul = 7) and (main.smo in (%1))) as partsum3, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As > 4 and main.As <= 6 and (main.granul = 6 or main.granul = 7) and (main.smo in (%1))) as partsum4, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As > 6 and (main.granul = 6 or main.granul = 7) and (main.smo in (%1))) as partsum5, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As < 5 and (main.granul <> 6 and main.granul <> 7) and (main.smo in (%1))) as partsum6, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As >= 5 and main.As <= 10 and (main.granul <> 6 and main.granul <> 7) and (main.smo in (%1))) as partsum7, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As > 10 and main.As <= 20 and (main.granul <> 6 and main.granul <> 7) and (main.smo in (%1))) as partsum8, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As > 20 and main.As <= 30 and (main.granul <> 6 and main.granul <> 7) and (main.smo in (%1))) as partsum9, "
                   "(select sum(s_pochv_kont) from main where  (main.As IS NOT NULL) and main.As > 30 and (main.granul <> 6 and main.granul <> 7) and (main.smo in (%1))) as partsum10 "
                   "from main "
                   "where main.smo in (%1) and main.As IS NOT NULL ");

    QSqlQuery main_query;
    QSqlQuery all_query;

    QSqlRecord rec;
    int partsum1_index;
    int partsum2_index;
    int partsum3_index;
    int partsum4_index;
    int partsum5_index;
    int partsum6_index;
    int partsum7_index;
    int partsum8_index;
    int partsum9_index;
    int partsum10_index;
    int allsum_index;
    int ves_index;
    float partsum1;
    float partsum2;
    float partsum3;
    float partsum4;
    float partsum5;
    float partsum6;
    float partsum7;
    float partsum8;
    float partsum9;
    float partsum10;
    float allsum;
    float ves;

    QSqlQuery smo_name;
    smo_name.prepare("SELECT id_smo, name from smo where region_id = "+reg_id);
    QString id, name;
    //------------------------
    QString numbers;
    QByteArray vsego("Всего по району");
    QString vsego_str = codec->toUnicode(vsego);
    //------------------------

    if(smo_name.exec()&&(smo_name.size()>0)){
        while (smo_name.next()){
            id = smo_name.value(0).toString();
            name = smo_name.value(1).toString();
            main_query.prepare(zapros.arg(id));
            all_query.prepare(zapros_all.arg(id));
            qDebug() << "id and name "<<id << name;

            //numbers - array with smo ids
            numbers.append(id + ",");

            if(all_query.exec()&&(all_query.size()>0)){

                rec = all_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                partsum7_index = rec.indexOf("partsum7");
                partsum8_index = rec.indexOf("partsum8");
                partsum9_index = rec.indexOf("partsum9");
                partsum10_index = rec.indexOf("partsum10");

                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                all_query.first();

                partsum1 = all_query.value(partsum1_index).toFloat();
                partsum2 = all_query.value(partsum2_index).toFloat();
                partsum3 = all_query.value(partsum3_index).toFloat();
                partsum4 = all_query.value(partsum4_index).toFloat();
                partsum5 = all_query.value(partsum5_index).toFloat();
                partsum6 = all_query.value(partsum6_index).toFloat();
                partsum7 = all_query.value(partsum7_index).toFloat();
                partsum8 = all_query.value(partsum8_index).toFloat();
                partsum9 = all_query.value(partsum9_index).toFloat();
                partsum10 = all_query.value(partsum10_index).toFloat();
                allsum = all_query.value(allsum_index).toFloat();
                ves = all_query.value(ves_index).toFloat();

                string = string + "<tr><td>" + name + "</td><td></td><td></td><td align=center>"
                        + russian.toString(allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(ves/allsum, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum1+partsum6, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum2+partsum7, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum3+partsum8, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum4+partsum9, 'f', 2)
                        + "</td><td align = center>"
                        + russian.toString(partsum5+partsum10, 'f', 2)
                        + "</td></tr>";
            }
            qDebug() << all_query.lastError().databaseText();
            if(main_query.exec()&&(main_query.size()>0)){
                qDebug() << "kuku";
                rec = main_query.record();
                partsum1_index = rec.indexOf("partsum1");
                partsum2_index = rec.indexOf("partsum2");
                partsum3_index = rec.indexOf("partsum3");
                partsum4_index = rec.indexOf("partsum4");
                partsum5_index = rec.indexOf("partsum5");
                partsum6_index = rec.indexOf("partsum6");
                partsum7_index = rec.indexOf("partsum7");
                partsum8_index = rec.indexOf("partsum8");
                partsum9_index = rec.indexOf("partsum9");
                partsum10_index = rec.indexOf("partsum10");
                allsum_index = rec.indexOf("allsum");
                ves_index = rec.indexOf("ves");

                while (main_query.next()){

                    partsum1 = main_query.value(partsum1_index).toFloat();
                    partsum2 = main_query.value(partsum2_index).toFloat();
                    partsum3 = main_query.value(partsum3_index).toFloat();
                    partsum4 = main_query.value(partsum4_index).toFloat();
                    partsum5 = main_query.value(partsum5_index).toFloat();
                    partsum6 = main_query.value(partsum6_index).toFloat();
                    partsum7 = main_query.value(partsum7_index).toFloat();
                    partsum8 = main_query.value(partsum8_index).toFloat();
                    partsum9 = main_query.value(partsum9_index).toFloat();
                    partsum10 = main_query.value(partsum10_index).toFloat();
                    allsum = main_query.value(allsum_index).toFloat();
                    ves = main_query.value(ves_index).toFloat();

                    if((main_query.value(3).toInt() == 6) or (main_query.value(3).toInt() == 7)){
                        qDebug() << main_query.value(3).toInt();
                        string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                                + russian.toString(allsum, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(ves/allsum, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum1, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum2, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum3, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum4, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum5, 'f', 2)
                                + "</td></tr>";
                    } else {
                        string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                                + russian.toString(allsum, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(ves/allsum, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum6, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum7, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum8, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum9, 'f', 2)
                                + "</td><td align = center>"
                                + russian.toString(partsum10, 'f', 2)
                                + "</td></tr>";
                    }

                }
            }
            qDebug() << "main query size" << main_query.size();
            main_query.lastError().databaseText();
        }
        //----------------------
        numbers.chop(1);
        qDebug() << "numbers" << numbers;
        main_query.prepare(zapros.arg(numbers));
        all_query.prepare(zapros_all.arg(numbers));
        //----------------------
        if(all_query.exec()&&(all_query.size()>0)){

            rec = all_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            partsum7_index = rec.indexOf("partsum7");
            partsum8_index = rec.indexOf("partsum8");
            partsum9_index = rec.indexOf("partsum9");
            partsum10_index = rec.indexOf("partsum10");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            all_query.first();

            partsum1 = all_query.value(partsum1_index).toFloat();
            partsum2 = all_query.value(partsum2_index).toFloat();
            partsum3 = all_query.value(partsum3_index).toFloat();
            partsum4 = all_query.value(partsum4_index).toFloat();
            partsum5 = all_query.value(partsum5_index).toFloat();
            partsum6 = all_query.value(partsum6_index).toFloat();
            partsum7 = all_query.value(partsum7_index).toFloat();
            partsum8 = all_query.value(partsum8_index).toFloat();
            partsum9 = all_query.value(partsum9_index).toFloat();
            partsum10 = all_query.value(partsum10_index).toFloat();
            allsum = all_query.value(allsum_index).toFloat();
            ves = all_query.value(ves_index).toFloat();

            string = string + "<tr><td>" + vsego_str + "</td><td></td><td></td><td align=center>"
                    + russian.toString(allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(ves/allsum, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum1+partsum6, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum2+partsum7, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum3+partsum8, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum4+partsum9, 'f', 2)
                    + "</td><td align = center>"
                    + russian.toString(partsum5+partsum10, 'f', 2)
                    + "</td></tr>";
        }
        qDebug() << all_query.lastError().databaseText();
        if(main_query.exec()&&(main_query.size()>0)){

            rec = main_query.record();
            partsum1_index = rec.indexOf("partsum1");
            partsum2_index = rec.indexOf("partsum2");
            partsum3_index = rec.indexOf("partsum3");
            partsum4_index = rec.indexOf("partsum4");
            partsum5_index = rec.indexOf("partsum5");
            partsum6_index = rec.indexOf("partsum6");
            partsum7_index = rec.indexOf("partsum7");
            partsum8_index = rec.indexOf("partsum8");
            partsum9_index = rec.indexOf("partsum9");
            partsum10_index = rec.indexOf("partsum10");
            allsum_index = rec.indexOf("allsum");
            ves_index = rec.indexOf("ves");

            while (main_query.next()){

                partsum1 = main_query.value(partsum1_index).toFloat();
                partsum2 = main_query.value(partsum2_index).toFloat();
                partsum3 = main_query.value(partsum3_index).toFloat();
                partsum4 = main_query.value(partsum4_index).toFloat();
                partsum5 = main_query.value(partsum5_index).toFloat();
                partsum6 = main_query.value(partsum6_index).toFloat();
                partsum7 = main_query.value(partsum7_index).toFloat();
                partsum8 = main_query.value(partsum8_index).toFloat();
                partsum9 = main_query.value(partsum9_index).toFloat();
                partsum10 = main_query.value(partsum10_index).toFloat();
                allsum = main_query.value(allsum_index).toFloat();
                ves = main_query.value(ves_index).toFloat();

                if((main_query.value(3).toInt() == 6) or (main_query.value(3).toInt() == 7)){
                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum1, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum2, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum3, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum4, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum5, 'f', 2)
                            + "</td></tr>";
                } else {
                    string = string + "<tr><td></td><td>" + main_query.value(1).toString()+ "</td><td>" + main_query.value(2).toString()+ "</td><td align=center>"
                            + russian.toString(allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(ves/allsum, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum6, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum7, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum8, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum9, 'f', 2)
                            + "</td><td align = center>"
                            + russian.toString(partsum10, 'f', 2)
                            + "</td></tr>";
                }


            }
        }
    } else {
        qDebug() << "Zhopa!!!";

    }
    qDebug() <<" gdfgfdg" << smo_name.lastError().databaseText();
    string = string + "</table>";
    return string;
}

QString ReportDialog::report_resp(QModelIndexList index, QString ids){
    QString result("");
    qDebug() << ids;
    for (int i = 0; i < index.count(); i++) {
        if (index.at(i).parent() != QModelIndex()){
            switch (index.at(i).row()){
                case 0:
                result = result + "<br>" + N_report_resp(ids);
                    break;
                case 1:
                    result = result + "<br>" + P_report_resp(ids);
                    break;
                case 2:
                    result = result + "<br>" + K_report_resp(ids);
                    break;
                case 3:
                    result = result + "<br>" + S_report_resp(ids);
                    break;
                case 4:
                    result = result + "<br>" + Org_report_resp(ids);
                    break;
                case 5:
                    result = result + "<br>" + PH_report_resp(ids);
                    break;
                case 6:
                    result = result + "<br>" + Ca_report_resp(ids);
                    break;
                case 7:
                    result = result + "<br>" + Mg_report_resp(ids);
                    break;
                case 8:
                    result = result + "<br>" + Zn_report_resp(ids);
                    break;
                case 9:
                    result = result + "<br>" + Cu_report_resp(ids);
                    break;
                case 10:
                    result = result + "<br>" + Mo_report_resp(ids);
                    break;
                case 11:
                    result = result + "<br>" + B_report_resp(ids);
                    break;
                case 12:
                    result = result + "<br>" + granul_report_resp(ids);
                    break;
                case 13:
                    result = result + "<br>" + REKO_report_resp(ids);
                    break;
                case 14:
                    result = result + "<br>" + Cd_report_resp(ids);
                    break;
                case 15:
                    result = result + "<br>" + Pb_report_resp(ids);
                    break;
                case 16:
                    result = result + "<br>" + Hg_report_resp(ids);
                    break;
                case 17:
                    result = result + "<br>" + As_report_resp(ids);
                    break;
             }
        }
    }
    return result;
}
