#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    qDebug()<<translator.load("russian", ":/res");
    a.installTranslator(&translator);
    MainWindow w;
    w.show();
    
    return a.exec();
}
