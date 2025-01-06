TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        client.cpp \
        clientbase.cpp \
        main.cpp

HEADERS += \
    client.hpp \
    clientbase.hpp

DISTFILES += \
    README.md
