QT       += core gui opengl serialport positioning

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    camera.cpp \
    chunk.cpp \
    chunkworker.cpp \
    kalmanfilter.cpp \
    logger.cpp \
    main.cpp \
    mainwindow.cpp \
    myglwidget.cpp \
    noiseutils.cpp \
    speedcontroller.cpp \
    terrainmanager.cpp

HEADERS += \
    camera.h \
    chunk.h \
    chunkworker.h \
    kalmanfilter.h \
    logger.h \
    mainwindow.h \
    myglwidget.h \
    noiseutils.h \
    speedcontroller.h \
    terrainmanager.h \
    worldconfig.h

FORMS += \
    myglwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/libs/Eigen
#Para que o compilador possa encontrar os cabeçalhos da eigen


