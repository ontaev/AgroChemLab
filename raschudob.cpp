#include "raschudob.h"
#include "ui_raschudob.h"
#include "previewform.h"

RaschUdob::RaschUdob(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RaschUdob)
{
    ui->setupUi(this);
    QObject::connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    smoModel = new QSqlTableModel(this);
    smoModel->setTable("smo");
    smoModel->select();
    ui->smoCombo->setModel(smoModel);
    ui->smoCombo->setModelColumn(smoModel->fieldIndex("name"));
    ui->smoCombo->setCurrentIndex(0);
}

RaschUdob::~RaschUdob()
{
    delete ui;
}

void RaschUdob::on_calcButton_clicked()
{
    QString id = smoModel->data(smoModel->index(ui->smoCombo->currentIndex(),0)).toString();

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");

    QByteArray pshen("<p align=left><b>Потребность в минеральных удобрениях под планируемую урожайность сельскохозяйственных культур (озимая пшеница) с учетом содержания питательных веществ</b></p>");
    QByteArray yach("<p align=left><b>Потребность в минеральных удобрениях под планируемую урожайность сельскохозяйственных культур (ячмень) с учетом содержания питательных веществ в почве</b></p>");
    QString rep_str = codec->toUnicode(pshen)+report(id, "2")+codec->toUnicode(yach)+report(id, "4");

    PreviewForm *PF = new PreviewForm (this, rep_str);
    PF->setAttribute(Qt::WA_DeleteOnClose);
    PF->setWindowFlags(Qt::Window);
    PF->show();

}

QString RaschUdob::report(QString smo_id, QString kultura_id)
{
    QString zapros("select kultura.name as kultura_col, nomer, nomer_rab_uch, s_rab_uch, "
                   "sum(main_t.Org*main_t.s_pochv_kont)/sum(main_t.s_pochv_kont) AS Org_ves, "
                   "sum(main_t.N*main_t.s_pochv_kont)/sum(main_t.s_pochv_kont) AS N_ves, "
                   "sum(main_t.P*main_t.s_pochv_kont)/sum(main_t.s_pochv_kont) AS P_ves, "
                   "sum(main_t.K*main_t.s_pochv_kont)/sum(main_t.s_pochv_kont) AS K_ves, "
                   "sum(main_t.Zn*main_t.s_pochv_kont)/sum(main_t.s_pochv_kont) AS Zn_ves, "
                   "sum(main_t.Cu*main_t.s_pochv_kont)/sum(main_t.s_pochv_kont) AS Cu_ves, "
                   "sum(main_t.Mo*main_t.s_pochv_kont)/sum(main_t.s_pochv_kont) AS Mo_ves, "
                   "sum(main_t.B*main_t.s_pochv_kont)/sum(main_t.s_pochv_kont) AS B_ves "
                   "from "
                   "((select kultura, nomer, nomer_rab_uch, s_rab_uch, s_pochv_kont, s_polya, zemlepolz, tip, Org, N, P, K, Zn, Cu, Mo, B "
                   "from main where smo IN (%1) and kultura = %2) AS main_t) "
                   "left join kultura "
                   "on kultura.id_kultura = main_t.kultura "
                   "group by main_t.nomer, main_t.s_polya, main_t.zemlepolz, main_t.tip, main_t.nomer_rab_uch");

    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<table>"
                    "<tr><td rowspan=4 align=center>Культура</td><td rowspan=4 align=center>№ поля</td>"
                    "<td rowspan=4 align=center>№ рабочего<br>участка</td>"
                    "<td rowspan=4 align=center>Площадь,<br>га</td>"
                    "<td rowspan=4 align=center>Содержание<br>органического<br>вещ-ва,%</td>"
                    "<td rowspan=3 colspan=3 align=center>Содержание<br>макроэлементов,<br>мг/кг</td>"
                    "<td rowspan=2 colspan=4 align=center>Содержание<br>микроэлементов, мг/кг </td>"
                    "<td colspan=6 align=center>Планируемая урожайность</td>"
                    "</tr>"
                    "<tr><td colspan=2 align=center>20 ц/га</td><td colspan=2 align=center>25 ц/га</td><td colspan=2 align=center>30 ц/га</td>	"
                    "</tr>"
                    "<tr><td rowspan=2 align=center>Цинк</td><td rowspan=2 align=center>Медь</td><td rowspan=2 align=center>Молибден</td><td rowspan=2 align=center>Бор</td>"
                    "<td colspan=6 align=center>Норма минеральных удобрений с учетом содержания<br>питательных элементов в почве, кг д.в/га</td>"
                    "</tr>"
                    "<tr>"
                    "<td align=center>N</td><td align=center>P</td><td align=center>K**</td>"
                    "<td align=center>N**</td><td align=center>P**</td><td align=center>N**</td><td align=center>P**</td><td align=center>N**</td><td align=center>P**</td>"
                    "</tr>");

    QByteArray p_pshen("под-<br>кормка<br>в два<br>приема<br>по 34-40");
    QByteArray p_yach("60 в два<br>приема");

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString p_pshen_str = codec->toUnicode(p_pshen);
    QString p_yach_str = codec->toUnicode(p_yach);
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);
    float p_1 = 0;
    float p_2 = 0;
    float p_3 = 0;
    QString p_1_str ="";
    QString p_2_str ="";
    QString p_3_str ="";
    QSqlQuery main_query;
    QSqlRecord rec;
    main_query.prepare(zapros.arg(smo_id, kultura_id));

    if(main_query.exec()&&(main_query.size()>0)){

        rec = main_query.record();
        int kultura_index = rec.indexOf("kultura_col");
        int nomer_index = rec.indexOf("nomer");
        int nomer_rab_uch_index = rec.indexOf("nomer_rab_uch");
        int s_rab_uch_index = rec.indexOf("s_rab_uch");

        int Org_index = rec.indexOf("Org_ves");
        int N_index = rec.indexOf("N_ves");
        int P_index = rec.indexOf("P_ves");
        int K_index = rec.indexOf("K_ves");
        int Zn_index = rec.indexOf("Zn_ves");
        int Cu_index = rec.indexOf("Cu_ves");
        int Mo_index = rec.indexOf("Mo_ves");
        int B_index = rec.indexOf("B_ves");

        int first = 1;
        int count = main_query.size();
        QString count_str = "<td align =center rowspan=%1>";
        count_str = count_str.arg(count);
        while (main_query.next()){

            QVariant kultura = main_query.value(kultura_index);
            QVariant nomer = main_query.value(nomer_index);
            QVariant nomer_rab_uch = main_query.value(nomer_rab_uch_index);
            QVariant s_rab_uch = main_query.value(s_rab_uch_index);

            QVariant Org = main_query.value(Org_index);
            QVariant N = main_query.value(N_index);
            QVariant P = main_query.value(P_index);
            QVariant K = main_query.value(K_index);
            QVariant Zn = main_query.value(Zn_index);
            QVariant Cu = main_query.value(Cu_index);
            QVariant Mo = main_query.value(Mo_index);
            QVariant B = main_query.value(B_index);

            float p_1_pshen = ((20*1.3)-(P.toFloat()/10*30*0.18))/0.2;
            float p_2_pshen = ((25*1.3)-(P.toFloat()/10*30*0.18))/0.2;
            float p_3_pshen = ((30*1.3)-(P.toFloat()/10*30*0.18))/0.2;

            float p_1_yach = ((25*1.1)-(P.toFloat()/10*30*0.18))/0.2;
            float p_2_yach = ((30*1.1)-(P.toFloat()/10*30*0.18))/0.2;
            float p_3_yach = ((40*1.1)-(P.toFloat()/10*30*0.18))/0.2;

            if (kultura_id == "2"){
                p_1 = p_1_pshen;
                p_2 = p_2_pshen;
                p_3 = p_3_pshen;
                p_1_str = p_pshen_str;
                p_2_str = p_pshen_str;
                p_3_str = p_pshen_str;
            }
            if (kultura_id == "4"){
                p_1 = p_1_yach;
                p_2 = p_2_yach;
                p_3 = p_3_yach;
                p_1_str = "40";
                p_2_str = "40";
                p_3_str = p_yach_str;
            }

            string = string + "<tr><td align=center>"+kultura.toString()+"</td><td align=center>"
            +nomer.toString()+"</td><td align=center>"+nomer_rab_uch.toString()+"</td><td align=center>"
            +s_rab_uch.toString()+"</td><td align=center>"
            +russian.toString(Org.toFloat(), 'f', 2)+"</td><td align=center>"
            +russian.toString(N.toFloat(), 'f', 2)+"</td><td align=center>"
            +russian.toString(P.toFloat(), 'f', 2)+"</td><td align=center>"
            +russian.toString(K.toFloat(), 'f', 2)+"</td><td align=center>"
            +russian.toString(Zn.toFloat(), 'f', 2)+"</td><td align=center>"
            +russian.toString(Cu.toFloat(), 'f', 2)+"</td><td align=center>"
            +russian.toString(Mo.toFloat(), 'f', 2)+"</td><td align=center>"
            +russian.toString(B.toFloat(), 'f', 2)+"</td>";

            if (first==1){
                string = string + count_str + p_1_str +"</td><td align=center>"+russian.toString(p_1, 'f', 2)
                        + count_str + p_2_str +"</td><td align=center>"+russian.toString(p_2, 'f', 2)
                        + count_str + p_3_str +"</td><td align=center>"+russian.toString(p_3, 'f', 2)+"</tr>";
            }
            else{
                string = string + "<td align=center>"+russian.toString(p_1, 'f', 2)
                        + "</td><td align=center>"+russian.toString(p_2, 'f', 2)
                        + "</td><td align=center>"+russian.toString(p_3, 'f', 2)+"</tr>";

            }
            first++;
        }

    }
    return string+"</table>";
}
