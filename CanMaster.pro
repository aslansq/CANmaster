QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CAN/can.cpp \
    CAN/canstdform.cpp \
    E2E/e2eprotectsend.cpp \
    E2E/e2eprotectsendp11.cpp \
    E2E/e2eprotecttxmsg.cpp \
    main.cpp \
    mainwindow.cpp \
    PEAK/peakbasiccan.cpp \
    PEAK/peakstdcan.cpp \
    AutosarE2E/crc.cpp \
    AutosarE2E/e2e.cpp

HEADERS += \
    CAN/can.h \
    CAN/canstdform.h \
    E2E/e2eprotectsend.h \
    E2E/e2eprotectsendp11.h \
    E2E/e2eprotecttxmsg.h \
    mainwindow.h \
    PEAK/peakbasiccan.h \
    PEAK/peakstdcan.h \
    AutosarE2E/crc.hpp \
    AutosarE2E/e2e.hpp

FORMS += \
    CAN/canstdform.ui \
    E2E/e2eprotectsend.ui \
    E2E/e2eprotectsendp11.ui \
    E2E/e2eprotecttxmsg.ui \
    mainwindow.ui

INCLUDEPATH += /usr/local/include
INCLUDEPATH += AutosarE2E/
INCLUDEPATH += CAN/
INCLUDEPATH += E2E/
INCLUDEPATH += PEAK/
LIBS += -L/usr/local/lib -ldbcppp
LIBS += -lpcanbasic

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
