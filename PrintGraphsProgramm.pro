QT       += core gui charts sql
QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    charts/barchartadapter.h \
    charts/linechartadapter.h \
    data/datafactory.h \
    data/jsondata.h \
    data/jsondataadapter.h \
    data/sqldata.h \
    data/sqldataadapter.h \
    interfaces/ichart.h \
    interfaces/ichartstyle.h \
    interfaces/idata.h \
    interfaces/idatafactory.h \
    ioc/container.h \
    mainwindow.h \
    services/chartservice.h \
    services/pdfprinter.h \
    styles/colorstyle.h \
    styles/grayscalestyle.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
