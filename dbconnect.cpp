#include "dbconnect.h"

bool createConnection(QString host, QString database)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    qDebug() << QSqlDatabase::drivers();
    db.setHostName(host);
    db.setDatabaseName(database);
    db.setUserName("root");
    db.setPassword("root");
    bool ok = db.open();
    if(ok){
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            MainWindow *nw = qobject_cast<MainWindow*>(widget);
            if(nw){
                nw->setStatus("Connected");
                nw->updateTable();
            }
        }
    }
    return ok;
}



