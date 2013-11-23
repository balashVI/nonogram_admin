#-------------------------------------------------
#
# Project created by QtCreator 2013-07-20T20:47:01
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nonogram_admin
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    crossword.cpp \
    network.cpp

HEADERS  += mainwindow.h \
    crossword.h

FORMS    += mainwindow.ui

TRANSLATIONS += translate_ua.ts\
                translate_ru.ts

RESOURCES += \
    res.qrc

OTHER_FILES += \
    translate_ru.ts \
    translate_ua.ts
