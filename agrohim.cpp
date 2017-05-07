//--------------------------------------------------
//
//          !!!WARNING @BIDLO@ CODE!!!
//
//--------------------------------------------------

#include "agrohim.h"
#include "ui_agrohim.h"
#include "previewform.h"

AgroHim::AgroHim(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AgroHim)
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

AgroHim::~AgroHim()
{
    delete ui;
}

void AgroHim::on_calcButton_clicked()
{
    QString id = smoModel->data(smoModel->index(ui->smoCombo->currentIndex(),0)).toString();

    QString zapros("select owner.owner as zemlepolz_col, tip.name as tip_col, nomer, s_polya, kultura.name as kultura_col, nomer_rab_uch, s_rab_uch, s_pochv_kont, indexes.name as index_col, granul.name as granul_col, "
                   "zasor, gamma, N, P, K, Org, S, ph, Ca, Mg, Na, Reko, Cd, Pb, main.As, Hg, Zn, Cu, Mo, B, pole_table.count_pole AS count_pole, "
                   "pole_table.zasor_ves AS zasor_ves_pole, pole_table.gamma_ves AS gamma_ves_pole, pole_table.N_ves AS N_ves_pole, pole_table.P_ves AS P_ves_pole, pole_table.K_ves AS K_ves_pole, "
                   "pole_table.Org_ves AS Org_ves_pole, pole_table.S_ves AS S_ves_pole, pole_table.ph_ves AS ph_ves_pole, pole_table.Ca_ves AS Ca_ves_pole, pole_table.Mg_ves AS Mg_ves_pole, "
                   "pole_table.Na_ves AS Na_ves_pole, pole_table.Reko_ves AS Reko_ves_pole, pole_table.Cd_ves AS Cd_ves_pole, pole_table.Pb_ves AS Pb_ves_pole, pole_table.As_ves AS As_ves_pole, "
                   "pole_table.Hg_ves AS Hg_ves_pole, pole_table.Zn_ves AS Zn_ves_pole, pole_table.Cu_ves AS Cu_ves_pole, pole_table.Mo_ves AS Mo_ves_pole, pole_table.B_ves AS B_ves_pole, "
                   "uch_table.count_uch AS count_uch, uch_table.zasor_ves AS zasor_ves_uch, uch_table.gamma_ves AS gamma_ves_uch, uch_table.N_ves AS N_ves_uch, uch_table.P_ves AS P_ves_uch, uch_table.K_ves AS K_ves_uch, "
                   "uch_table.Org_ves AS Org_ves_uch, uch_table.S_ves AS S_ves_uch, uch_table.ph_ves AS ph_ves_uch, uch_table.Ca_ves AS Ca_ves_uch, uch_table.Mg_ves AS Mg_ves_uch, "
                   "uch_table.Na_ves AS Na_ves_uch, uch_table.Reko_ves AS Reko_ves_uch, uch_table.Cd_ves AS Cd_ves_uch, uch_table.Pb_ves AS Pb_ves_uch, uch_table.As_ves AS As_ves_uch, "
                   "uch_table.Hg_ves AS Hg_ves_uch, uch_table.Zn_ves AS Zn_ves_uch, uch_table.Cu_ves AS Cu_ves_uch, uch_table.Mo_ves AS Mo_ves_uch, uch_table.B_ves AS B_ves_uch "
                   "from main "
                   "left join (select main.nomer AS nomer_pole, main.s_polya AS s_pole, "
                   "main.zemlepolz AS zemlepolz_pole, main.tip AS tip_pole, count(*) AS count_pole, "
                   "sum(main.zasor*main.s_pochv_kont)/sum(main.s_pochv_kont) AS zasor_ves, "
                   "sum(main.gamma*main.s_pochv_kont)/sum(main.s_pochv_kont) AS gamma_ves, "
                   "sum(main.N*main.s_pochv_kont)/sum(main.s_pochv_kont) AS N_ves, "
                   "sum(main.P*main.s_pochv_kont)/sum(main.s_pochv_kont) AS P_ves, "
                   "sum(main.K*main.s_pochv_kont)/sum(main.s_pochv_kont) AS K_ves, "
                   "sum(main.Org*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Org_ves, "
                   "sum(main.S*main.s_pochv_kont)/sum(main.s_pochv_kont) AS S_ves, "
                   "sum(main.ph*main.s_pochv_kont)/sum(main.s_pochv_kont) AS ph_ves, "
                   "sum(main.Ca*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Ca_ves, "
                   "sum(main.Mg*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Mg_ves, "
                   "sum(main.Na*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Na_ves, "
                   "sum(main.Reko*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Reko_ves, "
                   "sum(main.Cd*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Cd_ves, "
                   "sum(main.Pb*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Pb_ves, "
                   "sum(main.As*main.s_pochv_kont)/sum(main.s_pochv_kont) AS As_ves, "
                   "sum(main.Hg*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Hg_ves, "
                   "sum(main.Zn*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Zn_ves, "
                   "sum(main.Cu*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Cu_ves, "
                   "sum(main.Mo*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Mo_ves, "
                   "sum(main.B*main.s_pochv_kont)/sum(main.s_pochv_kont) AS B_ves "
                   " from main "
                   "group by main.nomer, main.s_polya, main.zemlepolz, main.tip "
                   "order by main.tip "
                   ") as pole_table "
                   "on (main.nomer=nomer_pole AND main.s_polya=s_pole AND main.zemlepolz=zemlepolz_pole AND main.tip=tip_pole) "
                   "left join (select main.nomer AS nomer_uch, main.s_polya AS s_uch, "
                   "main.zemlepolz AS zemlepolz_uch, main.tip AS tip_uch, main.nomer_rab_uch AS n_uch, count(*) AS count_uch, "
                   "sum(main.zasor*main.s_pochv_kont)/sum(main.s_pochv_kont) AS zasor_ves, "
                   "sum(main.gamma*main.s_pochv_kont)/sum(main.s_pochv_kont) AS gamma_ves, "
                   "sum(main.N*main.s_pochv_kont)/sum(main.s_pochv_kont) AS N_ves, "
                   "sum(main.P*main.s_pochv_kont)/sum(main.s_pochv_kont) AS P_ves, "
                   "sum(main.K*main.s_pochv_kont)/sum(main.s_pochv_kont) AS K_ves, "
                   "sum(main.Org*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Org_ves, "
                   "sum(main.S*main.s_pochv_kont)/sum(main.s_pochv_kont) AS S_ves, "
                   "sum(main.ph*main.s_pochv_kont)/sum(main.s_pochv_kont) AS ph_ves, "
                   "sum(main.Ca*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Ca_ves, "
                   "sum(main.Mg*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Mg_ves, "
                   "sum(main.Na*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Na_ves, "
                   "sum(main.Reko*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Reko_ves, "
                   "sum(main.Cd*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Cd_ves, "
                   "sum(main.Pb*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Pb_ves, "
                   "sum(main.As*main.s_pochv_kont)/sum(main.s_pochv_kont) AS As_ves, "
                   "sum(main.Hg*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Hg_ves, "
                   "sum(main.Zn*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Zn_ves, "
                   "sum(main.Cu*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Cu_ves, "
                   "sum(main.Mo*main.s_pochv_kont)/sum(main.s_pochv_kont) AS Mo_ves, "
                   "sum(main.B*main.s_pochv_kont)/sum(main.s_pochv_kont) AS B_ves "
                   " from main "
                   "group by main.nomer, main.s_polya, main.zemlepolz, main.tip, main.nomer_rab_uch "
                   "order by main.tip"
                   ") as uch_table "
                   "on (main.nomer=nomer_uch AND main.s_polya=s_uch AND main.zemlepolz=zemlepolz_uch AND main.tip=tip_uch AND main.nomer_rab_uch=n_uch) "
                   "left join owner "
                   "on owner.id_owner = zemlepolz "
                   "left join tip "
                   "on tip.id_tip = tip "
                   "left join kultura "
                   "on kultura.id_kultura = kultura "
                   "left join indexes "
                   "on indexes.id_index = index_p "
                   "left join granul "
                   "on granul.id_granul = granul "
                   "where smo in (%1)"
                   "order by zemlepolz, tip, nomer, s_polya, nomer_rab_uch ");
    QByteArray russ("<style type=\"text/css\">TABLE {border-collapse: collapse;border: 2px solid black;}TD, TH {border: 1px solid black;}</style>"
                    "<p align=left><b>Агрохимическая характеристика полей и участков</b></p>"
                    "<table>"
                    "<tr><td rowspan=3 align = center>Землепользователь,<br>№ поля и <br>севооборота.</td>"
                    "<td rowspan=3 align = center>Площадь<br>поля, га</td>"
                    "<td rowspan=3 align = center>Культура</td>"
                    "<td rowspan=3 align = center>№ рабочего<br>участка</td>"
                    "<td rowspan=3 align = center>Площадь<br>рабочего<br>участка,<br>га</td>"
                    "<td rowspan=3 align = center>Площадь<br>почвенного<br>контура</td>"
                    "<td rowspan=3 align = center>Индекс<br>почв</td>"
                    "<td rowspan=3 align = center>Грануло-<br>метрический<br>состав</td>"
                    "<td rowspan=3 align = center>Засоренность,<br>закустареность<br>посевов,<br>шт/кв.м,%</td>"
                    "<td rowspan=3 align = center>Гамма-фон,<br>мкР/час</td>"
                    "<td colspan=5 align = center>Содержание</td>"
                    "<td rowspan=3 align = center>pH(водный)</td>"
                    "<td rowspan=2 align = center>Содержание обменного кальция</td>"
                    "<td rowspan=2 align = center>Содержание обменного магния</td>"
                    "<td rowspan=2 align = center>Содержание обменного натрия</td>"
                    "<td rowspan=2 align = center>Реальная<br>ёмкость<br>катионного<br>обмена<br>(РЕКО)</td>"
                    "<td colspan=4 align = center>Содержание подвижных<br>форм тяжелых металлов</td>"
                    "<td colspan=4 align = center>Содержание микроэлементов</td>"
                    "</tr>"
                    "<tr><td align = center>азота<br>гидролизуемого</td>"
                    "<td align = center>подвижного<br>фосфора</td>"
                    "<td align = center>обменного<br>калия</td>"
                    "<td align = center>органического<br>вещества</td>"
                    "<td align = center>подвижной<br>серы</td>"
                    "<td align = center>Кадмий</td>"
                    "<td align = center>Свинец</td>"
                    "<td align = center>Мышьяк</td>"
                    "<td align = center>Ртуть</td>"
                    "<td align = center>Цинк</td>"
                    "<td align = center>Медь</td>"
                    "<td align = center>Молибден</td>"
                    "<td align = center>Бор</td>"
                    "</tr>"
                    "<tr><td align = center>мг/кг</td><td align = center>мг/кг</td>"
                    "<td align = center>мг/кг</td><td align = center>%</td>"
                    "<td align = center>мг/кг</td><td colspan=4 align = center>мг-экв/100 г почвы</td>"
                    "<td colspan=8 align = center>мг/кг почвы</td>"
                    "</tr>"
                    "<tr><td align = center>1</td><td align = center>2</td><td align = center>3</td><td align = center>4</td>"
                    "<td align = center>5</td><td align = center>6</td><td align = center>7</td><td align = center>8</td>"
                    "<td align = center>9</td><td align = center>10</td><td align = center>11</td><td align = center>12</td>"
                    "<td align = center>13</td><td align = center>14</td><td align = center>15</td><td align = center>16</td>"
                    "<td align = center>17</td><td align = center>18</td><td align = center>19</td><td align = center>20</td>"
                    "<td align = center>21</td><td align = center>22</td><td align = center>23</td><td align = center>24</td>"
                    "<td align = center>25</td><td align = center>26</td><td align = center>27</td><td align = center>28</td>"
                    "</tr>"  );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString string = codec->toUnicode(russ);
    QLocale russian(QLocale::Russian, QLocale::RussianFederation);

    QSqlQuery main_query;
    QSqlRecord rec;
    main_query.prepare(zapros.arg(id));

    if(main_query.exec()&&(main_query.size()>0)){

        rec = main_query.record();
        int zemlepolz_index = rec.indexOf("zemlepolz_col");
        int tip_index = rec.indexOf("tip_col");
        int nomer_index = rec.indexOf("nomer");
        int s_polya_index = rec.indexOf("s_polya");
        int kultura_index = rec.indexOf("kultura_col");
        int nomer_rab_uch_index = rec.indexOf("nomer_rab_uch");
        int s_rab_uch_index = rec.indexOf("s_rab_uch");
        int s_pochv_kont_index = rec.indexOf("s_pochv_kont");
        int index_index = rec.indexOf("index_col");
        int granul_index = rec.indexOf("granul_col");

        int count_pole_index = rec.indexOf("count_pole");
        int count_uch_index = rec.indexOf("count_uch");

        int zasor_index = rec.indexOf("zasor");
        int gamma_index = rec.indexOf("gamma");
        int N_index = rec.indexOf("N");
        int P_index = rec.indexOf("P");
        int K_index = rec.indexOf("K");
        int Org_index = rec.indexOf("Org");
        int S_index = rec.indexOf("S");
        int ph_index = rec.indexOf("ph");
        int Ca_index = rec.indexOf("Ca");
        int Mg_index = rec.indexOf("Mg");
        int Na_index = rec.indexOf("Na");
        int Reko_index = rec.indexOf("Reko");
        int Cd_index = rec.indexOf("Cd");
        int Pb_index = rec.indexOf("Pb");
        int As_index = rec.indexOf("As");
        int Hg_index = rec.indexOf("Hg");
        int Zn_index = rec.indexOf("Zn");
        int Cu_index = rec.indexOf("Cu");
        int Mo_index = rec.indexOf("Mo");
        int B_index = rec.indexOf("B");

        int zasor_pole_index = rec.indexOf("zasor_ves_pole");
        int gamma_pole_index = rec.indexOf("gamma_ves_pole");
        int N_pole_index = rec.indexOf("N_ves_pole");
        int P_pole_index = rec.indexOf("P_ves_pole");
        int K_pole_index = rec.indexOf("K_ves_pole");
        int Org_pole_index = rec.indexOf("Org_ves_pole");
        int S_pole_index = rec.indexOf("S_ves_pole");
        int ph_pole_index = rec.indexOf("ph_ves_pole");
        int Ca_pole_index = rec.indexOf("Ca_ves_pole");
        int Mg_pole_index = rec.indexOf("Mg_ves_pole");
        int Na_pole_index = rec.indexOf("Na_ves_pole");
        int Reko_pole_index = rec.indexOf("Reko_ves_pole");
        int Cd_pole_index = rec.indexOf("Cd_ves_pole");
        int Pb_pole_index = rec.indexOf("Pb_ves_pole");
        int As_pole_index = rec.indexOf("As_ves_pole");
        int Hg_pole_index = rec.indexOf("Hg_ves_pole");
        int Zn_pole_index = rec.indexOf("Zn_ves_pole");
        int Cu_pole_index = rec.indexOf("Cu_ves_pole");
        int Mo_pole_index = rec.indexOf("Mo_ves_pole");
        int B_pole_index = rec.indexOf("B_ves_pole");

        int zasor_uch_index = rec.indexOf("zasor_ves_uch");
        int gamma_uch_index = rec.indexOf("gamma_ves_uch");
        int N_uch_index = rec.indexOf("N_ves_uch");
        int P_uch_index = rec.indexOf("P_ves_uch");
        int K_uch_index = rec.indexOf("K_ves_uch");
        int Org_uch_index = rec.indexOf("Org_ves_uch");
        int S_uch_index = rec.indexOf("S_ves_uch");
        int ph_uch_index = rec.indexOf("ph_ves_uch");
        int Ca_uch_index = rec.indexOf("Ca_ves_uch");
        int Mg_uch_index = rec.indexOf("Mg_ves_uch");
        int Na_uch_index = rec.indexOf("Na_ves_uch");
        int Reko_uch_index = rec.indexOf("Reko_ves_uch");
        int Cd_uch_index = rec.indexOf("Cd_ves_uch");
        int Pb_uch_index = rec.indexOf("Pb_ves_uch");
        int As_uch_index = rec.indexOf("As_ves_uch");
        int Hg_uch_index = rec.indexOf("Hg_ves_uch");
        int Zn_uch_index = rec.indexOf("Zn_ves_uch");
        int Cu_uch_index = rec.indexOf("Cu_ves_uch");
        int Mo_uch_index = rec.indexOf("Mo_ves_uch");
        int B_uch_index = rec.indexOf("B_ves_uch");

        QString zemlepolz_str = "";
        QString tip_str = "";
        QString n_polya_str = "";
        QString s_polya_str = "";
        QString n_rab_uch_str = "";
        QString s_rab_uch_str = "";

        while (main_query.next()){
            QVariant zemlepolz = main_query.value(zemlepolz_index);
            QVariant tip = main_query.value(tip_index);
            QVariant nomer = main_query.value(nomer_index);
            QVariant s_polya = main_query.value(s_polya_index);
            QVariant kultura = main_query.value(kultura_index);
            QVariant nomer_rab_uch = main_query.value(nomer_rab_uch_index);
            QVariant s_rab_uch = main_query.value(s_rab_uch_index);
            QVariant s_pochv_kont = main_query.value(s_pochv_kont_index);
            QVariant index = main_query.value(index_index);
            QVariant granul = main_query.value(granul_index);
            QVariant count_pole = main_query.value(count_pole_index);
            QVariant count_uch = main_query.value(count_uch_index);

            QVariant zasor = main_query.value(zasor_index);
            QVariant gamma = main_query.value(gamma_index);
            QVariant N = main_query.value(N_index);
            QVariant P = main_query.value(P_index);
            QVariant K = main_query.value(K_index);
            QVariant Org = main_query.value(Org_index);
            QVariant S = main_query.value(S_index);
            QVariant ph = main_query.value(ph_index);
            QVariant Ca = main_query.value(Ca_index);
            QVariant Mg = main_query.value(Mg_index);
            QVariant Na = main_query.value(Na_index);
            QVariant Reko = main_query.value(Reko_index);
            QVariant Cd = main_query.value(Cd_index);
            QVariant Pb = main_query.value(Pb_index);
            QVariant As = main_query.value(As_index);
            QVariant Hg = main_query.value(Hg_index);
            QVariant Zn = main_query.value(Zn_index);
            QVariant Cu = main_query.value(Cu_index);
            QVariant Mo = main_query.value(Mo_index);
            QVariant B = main_query.value(B_index);

            QVariant zasor_pole = main_query.value(zasor_pole_index);
            QVariant gamma_pole = main_query.value(gamma_pole_index);
            QVariant N_pole = main_query.value(N_pole_index);
            QVariant P_pole = main_query.value(P_pole_index);
            QVariant K_pole = main_query.value(K_pole_index);
            QVariant Org_pole = main_query.value(Org_pole_index);
            QVariant S_pole = main_query.value(S_pole_index);
            QVariant ph_pole = main_query.value(ph_pole_index);
            QVariant Ca_pole = main_query.value(Ca_pole_index);
            QVariant Mg_pole = main_query.value(Mg_pole_index);
            QVariant Na_pole = main_query.value(Na_pole_index);
            QVariant Reko_pole = main_query.value(Reko_pole_index);
            QVariant Cd_pole = main_query.value(Cd_pole_index);
            QVariant Pb_pole = main_query.value(Pb_pole_index);
            QVariant As_pole = main_query.value(As_pole_index);
            QVariant Hg_pole = main_query.value(Hg_pole_index);
            QVariant Zn_pole = main_query.value(Zn_pole_index);
            QVariant Cu_pole = main_query.value(Cu_pole_index);
            QVariant Mo_pole = main_query.value(Mo_pole_index);
            QVariant B_pole = main_query.value(B_pole_index);

            QVariant zasor_uch = main_query.value(zasor_uch_index);
            QVariant gamma_uch = main_query.value(gamma_uch_index);
            QVariant N_uch = main_query.value(N_uch_index);
            QVariant P_uch = main_query.value(P_uch_index);
            QVariant K_uch = main_query.value(K_uch_index);
            QVariant Org_uch = main_query.value(Org_uch_index);
            QVariant S_uch = main_query.value(S_uch_index);
            QVariant ph_uch = main_query.value(ph_uch_index);
            QVariant Ca_uch = main_query.value(Ca_uch_index);
            QVariant Mg_uch = main_query.value(Mg_uch_index);
            QVariant Na_uch = main_query.value(Na_uch_index);
            QVariant Reko_uch = main_query.value(Reko_uch_index);
            QVariant Cd_uch = main_query.value(Cd_uch_index);
            QVariant Pb_uch = main_query.value(Pb_uch_index);
            QVariant As_uch = main_query.value(As_uch_index);
            QVariant Hg_uch = main_query.value(Hg_uch_index);
            QVariant Zn_uch = main_query.value(Zn_uch_index);
            QVariant Cu_uch = main_query.value(Cu_uch_index);
            QVariant Mo_uch = main_query.value(Mo_uch_index);
            QVariant B_uch = main_query.value(B_uch_index);

            if((zemlepolz_str != zemlepolz.toString())||(tip_str != tip.toString())){
                string = string + "<tr><td colspan=28 align=center><b>"+zemlepolz.toString()+" "+tip.toString()+"</b></td></tr>";
                zemlepolz_str = zemlepolz.toString();
                tip_str = tip.toString();
                n_polya_str = "";
                s_polya_str = "";
                n_rab_uch_str = "";
                s_rab_uch_str = "";
                //nado sbrasivat' n_polya ili net?
            }
            if((n_polya_str != nomer.toString())||(s_polya_str != s_polya.toString())){
                string = string + "<tr><td align=center><b>"+nomer.toString()+"</b></td><td align=center><b>"+s_polya.toString()+"</b></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td align=center><b>"
                        +russian.toString(gamma_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(N_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(P_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(K_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Org_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(S_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(ph_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Ca_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Mg_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Na_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Reko_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Cd_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Pb_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(As_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Hg_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Zn_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Cu_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(Mo_pole.toFloat(), 'f', 2)+"</b></td><td align=center><b>"
                        +russian.toString(B_pole.toFloat(), 'f', 2)+"</b></td></tr>";
                n_polya_str = nomer.toString();
                s_polya_str = s_polya.toString();
                n_rab_uch_str = "";
                s_rab_uch_str = "";
            }
            if((n_rab_uch_str != nomer_rab_uch.toString())||(s_rab_uch_str != s_rab_uch.toString())){
                string = string + "<tr><td></td><td></td><td align=center>"+kultura.toString()+"</td><td align=center>"+nomer_rab_uch.toString()+"</td><td align=center>"+s_rab_uch.toString()
                        +"</td><td></td><td></td><td></td><td align=center"+russian.toString(zasor_uch.toFloat(), 'f', 2)+"</td><td></td><td align=center>"
                        +russian.toString(N_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(P_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(K_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Org_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(S_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(ph_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Ca_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Mg_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Na_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Reko_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Cd_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Pb_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(As_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Hg_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Zn_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Cu_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(Mo_uch.toFloat(), 'f', 2)+"</td><td align=center>"
                        +russian.toString(B_uch.toFloat(), 'f', 2)+"</td></tr>";

                n_rab_uch_str = nomer_rab_uch.toString();
                s_rab_uch_str = s_rab_uch.toString();
            }
            string = string + "<tr><td></td><td></td><td></td><td></td><td></td><td align=center>"+s_pochv_kont.toString()
                    +"</td><td align=center>"+index.toString()+"</td><td align=center>"+granul.toString()
                    +"</td><td></td><td></td><td align=center>"
                    +russian.toString(N.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(P.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(K.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Org.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(S.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(ph.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Ca.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Mg.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Na.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Reko.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Cd.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Pb.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(As.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Hg.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Zn.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Cu.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(Mo.toFloat(), 'f', 2)+"</td><td align=center>"
                    +russian.toString(B.toFloat(), 'f', 2)+"</td></tr>";

        }
    }

    PreviewForm *PF = new PreviewForm (this, string);
    PF->setAttribute(Qt::WA_DeleteOnClose);
    PF->setWindowFlags(Qt::Window);
    PF->show();

    return;
}
