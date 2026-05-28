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
    src/filters/BandPassFilter.cpp \
    src/filters/BandStopFilter.cpp \
    src/filters/ComplementaryFilter.cpp \
    src/filters/EwmaFilter.cpp \
    src/filters/HampelFilter.cpp \
    src/filters/KalmanFilter1D.cpp \
    src/filters/LowPassFilter.cpp \
    src/filters/MedianFilter.cpp \
    src/filters/MovingAverageFilter.cpp \
    src/filters/NotchFilter.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/widgets/RobotArmWidget.cpp \
    src/widgets/SignalChartWidget.cpp

HEADERS += \
    src/core/SignalFrame.h \
    src/demos/SignalGenerator.h \
    src/filters/BandPassFilter.h \
    src/filters/BandStopFilter.h \
    src/filters/ComplementaryFilter.h \
    src/filters/EwmaFilter.h \
    src/filters/FilterBase.h \
    src/filters/HampelFilter.h \
    src/filters/KalmanFilter1D.h \
    src/filters/LowPassFilter.h \
    src/filters/MedianFilter.h \
    src/filters/MovingAverageFilter.h \
    src/filters/NotchFilter.h \
    src/mainwindow.h \
    src/widgets/RobotArmWidget.h \
    src/widgets/SignalChartWidget.h

FORMS += \
    src/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
