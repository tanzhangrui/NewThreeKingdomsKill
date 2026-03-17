QT       += core gui widgets multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += utf8_source

SOURCES += \
    main.cpp \
    card.cpp \
    hero.cpp \
    Player.cpp \
    gameengine.cpp \
    mainwindow.cpp

HEADERS += \
    Card.h \
    Hero.h \
    Player.h \
    gameengine.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

DEFINES += QT_DEPRECATED_WARNINGS

RC_ICONS = 

TARGET = ThreeKingdomsKill

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
