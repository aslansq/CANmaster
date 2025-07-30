QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CAN/can.cpp \
    CAN/canstdform.cpp \
    E2E/ProtectSend/e2eprotectsend.cpp \
    E2E/ProtectSend/e2eprotectsendp11.cpp \
    E2E/ProtectSend/e2eprotecttxmsg.cpp \
    E2E/ReceiveCheck/e2ereceivecheck.cpp \
    E2E/ReceiveCheck/e2ereceivecheckp11.cpp \
    E2E/ReceiveCheck/e2ereceiverxmsg.cpp \
    main.cpp \
    mainwindow.cpp \
    PEAK/peakbasiccan.cpp \
    PEAK/peakstdcan.cpp \
    AutosarE2E/crc.cpp \
    AutosarE2E/e2e.cpp

HEADERS += \
    CAN/can.h \
    CAN/canstdform.h \
    E2E/ProtectSend/e2eprotectsend.h \
    E2E/ProtectSend/e2eprotectsendp11.h \
    E2E/ProtectSend/e2eprotecttxmsg.h \
    E2E/ReceiveCheck/e2ereceivecheck.h \
    E2E/ReceiveCheck/e2ereceivecheckp11.h \
    E2E/ReceiveCheck/e2ereceiverxmsg.h \
    mainwindow.h \
    PEAK/peakbasiccan.h \
    PEAK/peakstdcan.h \
    AutosarE2E/crc.hpp \
    AutosarE2E/e2e.hpp

FORMS += \
    CAN/canstdform.ui \
    E2E/ProtectSend/e2eprotectsend.ui \
    E2E/ProtectSend/e2eprotectsendp11.ui \
    E2E/ProtectSend/e2eprotecttxmsg.ui \
    E2E/ReceiveCheck/e2ereceivecheck.ui \
    E2E/ReceiveCheck/e2ereceivecheckp11.ui \
    E2E/ReceiveCheck/e2ereceiverxmsg.ui \
    mainwindow.ui

INCLUDEPATH += /usr/local/include
INCLUDEPATH += AutosarE2E/
INCLUDEPATH += CAN/
INCLUDEPATH += E2E/ProtectSend
INCLUDEPATH += E2E/ReceiveCheck
INCLUDEPATH += PEAK/
LIBS += -L/usr/local/lib -ldbcppp
LIBS += -lpcanbasic

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
