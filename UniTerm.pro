QT += widgets serialport network

APP_NAME = "UniTerm"
APP_DESCRIPTION = "Universal terminal"
APP_COPYRIGHT = "Copyright 2024 Oleg Bolshakov"
APP_VERSION = "0.6"

TARGET = $$APP_NAME
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

RESOURCES += \
    res.qrc

DEFINES += \
    APP_NAME=\\\"$$APP_NAME\\\" \
    APP_VERSION=\\\"$$APP_VERSION\\\"

RC_ICONS = $$PWD/ico/app.ico

Release:DESTDIR = release
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui

DISTFILES +=

win32 {
    QMAKE_TARGET_COMPANY = $$APP_NAME
    QMAKE_TARGET_DESCRIPTION = $$APP_DESCRIPTION
    QMAKE_TARGET_COPYRIGHT = $$APP_COPYRIGHT
    QMAKE_TARGET_PRODUCT = $$APP_NAME
    VERSION = $$APP_VERSION

    # exe to folder
    package.target = package
    package.commands = \
        mkdir $$quote($${PWD}/build/$${APP_NAME}) -p && \
        cp $$quote($${OUT_PWD}/release/$${APP_NAME}.exe) $$quote($${PWD}/build/$${APP_NAME}/$${APP_NAME}.exe)

    # Folder to zip
    zip.target = zip
    zip.depends = package
    zip.commands = 7z a -tzip $$quote($${PWD}/build/$${APP_NAME}-v$${APP_VERSION}.zip) $$system_path($${PWD}/build/$${APP_NAME})

    QMAKE_EXTRA_TARGETS += package zip
}

# bug for static build
QMAKE_LFLAGS += -Wl,--start-group
