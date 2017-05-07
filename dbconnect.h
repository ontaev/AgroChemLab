#ifndef DBCONNECT_H
#define DBCONNECT_H

#include <QtSql>
#include <QSqlDatabase>
#include <QString>
#include "mainwindow.h"



bool createConnection(QString host, QString database);

#endif // DBCONNECT_H
