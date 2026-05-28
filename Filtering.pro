QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
msvc: QMAKE_CXXFLAGS += /utf-8

INCLUDEPATH += \
    src \
    src/core \
    src/filters \
    src/demos \
    src/widgets \
    src/content \
    src/utils

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/demos/SignalGenerator.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/widgets/SignalChartWidget.cpp

HEADERS += \
    src/core/SignalFrame.h \
    src/demos/SignalGenerator.h \
    src/mainwindow.h \
    src/widgets/SignalChartWidget.h

FORMS += \
    src/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
