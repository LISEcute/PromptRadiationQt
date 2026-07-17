TEMPLATE = app
TARGET = PromptRadiation

QT += widgets

CONFIG += c++17 warn_on
CONFIG -= app_bundle

win32-g++ {
    DESTDIR = c:/PromptRadiation/_install
}

win32-msvc {
    #DESTDIR = c:/PromptRadiation/_install
    DESTDIR = c:/PromptRadiation/_install_MSVC
}


DEFINES += PROMPT_RADIATION_SOURCE_DIR=\\\"$$PWD\\\"


SOURCES += \
    MainWindowPR.cpp \
    d_promtRad_GlobalSettings.cpp \
    d_promtRad_LocationFactors.cpp \
    d_promtRad_StopBoundaries.cpp \
    mainPR.cpp \
    o_promptRad_Detailed.cpp \
    o_promptRad_LocationFactorsModel.cpp \
    o_promptRad_UiHelpers.cpp

HEADERS += \
    MainWindowPR.h \
    d_promtRad_GlobalSettings.h \
    d_promtRad_LocationFactors.h \
    d_promtRad_StopBoundaries.h \
    o_promptRad_Detailed.h \
    o_promptRad_LocationFactorsModel.h \
    o_promptRad_UiHelpers.h

DISTFILES += README.md PromptRadiation.ini \
    PromtRadiation_InputYields.txt \
    PromtRadiation_InputPositions.txt \
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
