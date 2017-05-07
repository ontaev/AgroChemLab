#-------------------------------------------------
#
# Project created by QtCreator 2014-05-04T13:56:32
#
#-------------------------------------------------

QT       += core gui
QT       += sql
QT       += webkit

TARGET = AgroChemLab
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    databaselist.cpp \
    dbconnect.cpp \
    adddatabase.cpp \
    smolist.cpp \
    ownerlist.cpp \
    indexlist.cpp \
    granullist.cpp \
    opencard.cpp \
    regionlist.cpp \
    tiplist.cpp \
    kulturalist.cpp \
    noeditabledelegate.cpp \
    reportdialog.cpp \
    treemodel.cpp \
    treeitem.cpp \
    newdb.cpp \
    previewform.cpp \
    filter.cpp \
    raschudob.cpp \
    agrohim.cpp

HEADERS  += mainwindow.h \
    databaselist.h \
    dbconnect.h \
    adddatabase.h \
    smolist.h \
    ownerlist.h \
    indexlist.h \
    granullist.h \
    opencard.h \
    regionlist.h \
    tiplist.h \
    kulturalist.h \
    noeditabledelegate.h \
    reportdialog.h \
    treemodel.h \
    treeitem.h \
    newdb.h \
    previewform.h \
    filter.h \
    raschudob.h \
    agrohim.h

FORMS    += mainwindow.ui \
    databaselist.ui \
    adddatabase.ui \
    smolist.ui \
    ownerlist.ui \
    indexlist.ui \
    granullist.ui \
    opencard.ui \
    regionlist.ui \
    tiplist.ui \
    kulturalist.ui \
    reportdialog.ui \
    newdb.ui \
    previewform.ui \
    filter.ui \
    raschudob.ui \
    agrohim.ui

RESOURCES += \
    res.qrc \
    res.qrc

TRANSLATIONS +=\
    res/russian.ts
