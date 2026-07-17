QT += widgets

CONFIG += c++17 warn_on
CONFIG -= app_bundle

TEMPLATE = app
TARGET = PromptRadiationQt

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    GlobalSettingsDialog.cpp \
    LocationFactorsDialog.cpp \
    LocationFactorsModel.cpp \
    StopBoundariesDialog.cpp \
    PromptRadiationDetailed.cpp

HEADERS += \
    MainWindow.h \
    GlobalSettingsDialog.h \
    LocationFactorsDialog.h \
    LocationFactorsModel.h \
    StopBoundariesDialog.h \
    PromptRadiationDetailed.h

DISTFILES += README.md PromptRadiation.ini \
    Icons/sign_ok.png \
    Icons/copy.png \
    Icons/paste1.png \
    Icons/cancel.png \
    Icons/trans.gif \
    Icons/quit.png \
    Icons/lisepp_small.png \
    Icons/lise_small_new.ico

RESOURCES += PromptRadiationResources.qrc
win32:RC_ICONS += Icons/lise_small_new.ico
