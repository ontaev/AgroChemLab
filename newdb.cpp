#include "newdb.h"
#include "ui_newdb.h"
#include <QtSql>
#include <QSqlDatabase>
#include <QtGui>

NewDb::NewDb(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewDb)
{
    ui->setupUi(this);
    QObject::connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

NewDb::~NewDb()
{
    delete ui;
}

void NewDb::on_createButton_clicked()
{
    const QString conn = QLatin1String("dbcreation");
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", conn);
    db.setHostName(ui->HostTxt->text());
    db.setDatabaseName("mysql");
    db.setUserName("root");
    db.setPassword("root");

    QByteArray kultura("(1,'Не указана'),(2,'Озимая пшеница'),(3,'Яровая пшеница'),(4,'Яровой ячмень');");
    QByteArray granul("(1,'Не указан'),(2,'Сс'),(3,'Тс'),(4,'Лс'),(5,'Гл'),(6,'П'),(7,'СП');");
    QByteArray owner("(1,'Не указан');");
    QByteArray index("(1,'Не указан'),(2,'К1'),(3,CONCAT(CHAR(0xe28693),'K1')),(4,CONCAT(CHAR(0xe28693),CONCAT(CHAR(0xe28693),'K1'))),(5,CONCAT(CHAR(0xe28693),CONCAT(CHAR(0xe28693),CONCAT(CHAR(0xe28693),'K1')))),(6,'СН1'),(7,'СН2'),(8,'Лг'),(9,'ЛБ'),(10,CONCAT(CHAR(0xcf9f),' K1'));");
    QByteArray smo("(1,'Не указано',1);");
    QByteArray tip("(1,'Не указан'),(2,'Пашня богарная'),(3,'Сенокосы'),(4,'Пастбища'),(5,'Лиман');");
    QByteArray region("(1,'Не указан'),(2,'Городовиковский'),(3,'Ики-Бурульский'),(4,'Кетченеровский'),(5,'Лаганский'),(6,'Малодербетовский'),(7,'Октябрьский'),(8,'Приютненский'),(9,'Сарпинский'),(10,'Целинный'),(11,'Черноземельский'),(12,'Юстинский'),(13,'Яшалтинский'),(14,'Яшкульский'),(15,'Элиста');");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString kultura_str = codec->toUnicode(kultura);
    QString granul_str = codec->toUnicode(granul);
    QString owner_str = codec->toUnicode(owner);
    QString index_str = codec->toUnicode(index);
    QString smo_str = codec->toUnicode(smo);
    QString tip_str = codec->toUnicode(tip);
    QString region_str = codec->toUnicode(region);

    if(db.open()){
        QSqlQuery query(QSqlDatabase::database("dbcreation"));
        query.prepare("CREATE DATABASE "+ui->NameTxt->text());

        if(query.exec()){
            db.setDatabaseName(ui->NameTxt->text());
            db.open();
            db.transaction();

            query.exec("CREATE TABLE `granul` ("
            "  `id_granul` int(11) NOT NULL AUTO_INCREMENT,"
            "  `name` varchar(45) DEFAULT NULL,"
            "  PRIMARY KEY (`id_granul`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
            query.exec("INSERT INTO `granul` VALUES "+granul_str);
            qDebug()<<query.lastError();
            query.exec("CREATE TABLE `indexes` ("
            "  `id_index` int(11) NOT NULL AUTO_INCREMENT,"
            "  `name` varchar(45) DEFAULT NULL,"
            "  PRIMARY KEY (`id_index`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
            query.exec("INSERT INTO `indexes` VALUES "+index_str);
            qDebug()<<query.lastError();
            query.exec("CREATE TABLE `kultura` ("
            "  `id_kultura` int(11) NOT NULL AUTO_INCREMENT,"
            "  `name` varchar(100) DEFAULT NULL,"
            "  PRIMARY KEY (`id_kultura`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
            query.exec("INSERT INTO `kultura` VALUES "+kultura_str);
            qDebug()<<query.lastError();
            query.exec("CREATE TABLE `main` ("
            "  `id` int(11) NOT NULL AUTO_INCREMENT,"
            "  `zemlepolz` int(11) DEFAULT NULL,"
            "  `smo` int(11) DEFAULT NULL,"
            "  `tip` int(11) DEFAULT NULL,"
            "  `nomer` text,"
            "  `s_polya` float DEFAULT NULL,"
            "  `kultura` int(11) DEFAULT NULL,"
            "  `nomer_rab_uch` int(11) DEFAULT NULL,"
            "  `s_rab_uch` float DEFAULT NULL,"
            "  `s_pochv_kont` float DEFAULT NULL,"
            "  `index_p` int(11) DEFAULT NULL,"
            "  `granul` int(11) DEFAULT NULL,"
            "  `zasor` float DEFAULT NULL,"
            "  `gamma` float DEFAULT NULL,"
            "  `N` float DEFAULT NULL,"
            "  `P` float DEFAULT NULL,"
            "  `K` float DEFAULT NULL,"
            "  `Org` float DEFAULT NULL,"
            "  `S` float DEFAULT NULL,"
            "  `ph` float DEFAULT NULL,"
            "  `Ca` float DEFAULT NULL,"
            "  `Mg` float DEFAULT NULL,"
            "  `Na` float DEFAULT NULL,"
            "  `Reko` float DEFAULT NULL,"
            "  `Cd` float DEFAULT NULL,"
            "  `Pb` float DEFAULT NULL,"
            "  `As` float DEFAULT NULL,"
            "  `Hg` float DEFAULT NULL,"
            "  `Zn` float DEFAULT NULL,"
            "  `Cu` float DEFAULT NULL,"
            "  `Mo` float DEFAULT NULL,"
            "  `B` float DEFAULT NULL,"
            "  PRIMARY KEY (`id`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
            query.exec("CREATE TABLE `owner` ("
            "  `id_owner` int(11) NOT NULL AUTO_INCREMENT,"
            "  `owner` varchar(100) DEFAULT NULL,"
            "  PRIMARY KEY (`id_owner`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
            query.exec("INSERT INTO `owner` VALUES "+owner_str);
            qDebug()<<query.lastError();
            query.exec("CREATE TABLE `regions` ("
            "  `id_region` int(11) NOT NULL AUTO_INCREMENT,"
            "  `region` varchar(45) DEFAULT NULL,"
            "  PRIMARY KEY (`id_region`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
            query.exec("INSERT INTO `regions` VALUES "+region_str);
            qDebug()<<query.lastError();
            query.exec("CREATE TABLE `smo` ("
            "  `id_smo` int(11) NOT NULL AUTO_INCREMENT,"
            "  `name` varchar(45) DEFAULT NULL,"
            "  `region_id` int(11) DEFAULT NULL,"
            "  PRIMARY KEY (`id_smo`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
            query.exec("INSERT INTO `smo` VALUES "+smo_str);
            qDebug()<<query.lastError();
            query.exec("CREATE TABLE `tip` ("
            "  `id_tip` int(11) NOT NULL AUTO_INCREMENT,"
            "  `name` varchar(100) DEFAULT NULL,"
            "  PRIMARY KEY (`id_tip`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
            query.exec("INSERT INTO `tip` VALUES "+tip_str);


            qDebug()<<query.lastError();
            db.commit();
            QSettings settings("Infoservice", "AgroChemLab");
                settings.beginGroup("Databases");
                    settings.beginGroup(ui->NameTxt->text());
                        settings.setValue("Server", "localhost");
                    settings.endGroup();
                settings.endGroup();
            QMessageBox::information(this, tr("New database"), tr("Database %1 succesfully created").arg(ui->NameTxt->text()));

        }
        else{
            QMessageBox::warning(this, tr("New database"), tr("The database reported an error: %1").arg(query.lastError().databaseText()));
        }
        db.close();
    }
    else{
        QMessageBox::warning(this, tr("New database"), tr("The database reported an error: %1").arg(db.lastError().databaseText()));
    }
    QSqlDatabase::removeDatabase("dbcreation");
}
