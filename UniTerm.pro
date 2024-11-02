QT += widgets serialport
requires(qtConfig(combobox))

TARGET = UniTerm
TEMPLATE = app

SOURCES += \
    src/actionbutton.cpp \
    src/find.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/console.cpp \
    src/settings.cpp

HEADERS += \
    src/actionbutton.h \
    src/find.h \
    src/mainwindow.h \
    src/console.h \
    src/settings.h

FORMS += \
    src/find.ui \
    src/mainwindow.ui \
    src/settings.ui

RESOURCES += res.qrc

RC_ICONS = $$PWD/ico/app.ico

Release:DESTDIR = ../UniTerm
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui

QMAKE_LFLAGS += -Wl,--start-group

DISTFILES +=

