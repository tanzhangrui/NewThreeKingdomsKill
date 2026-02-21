QT       += core gui widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
# 解决Qt中文乱码核心配置
msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
} else {
    QMAKE_CXXFLAGS += -fexec-charset=UTF-8
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    Card.cpp \
    KillCard.cpp \
    FlashCard.cpp \
    PeachCard.cpp \
    TrickCard.cpp \
    Player.cpp \
    Hero.cpp \
    XingDaorongHero.cpp \
    LiuBeiHero.cpp \
    LiuSandaoHero.cpp \
    ZhugeLiangHero.cpp \
    CaoCaoHero.cpp \
    ZhangFeiHero.cpp

HEADERS += \
    mainwindow.h \
    Card.h \
    KillCard.h \
    FlashCard.h \
    PeachCard.h \
    TrickCard.h \
    Player.h \
    Hero.h \
    XingDaorongHero.h \
    LiuBeiHero.h \
    LiuSandaoHero.h \
    ZhugeLiangHero.h \
    CaoCaoHero.h \
    ZhangFeiHero.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
