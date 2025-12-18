QT += core testlib sql
CONFIG += console c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = LedgerAppTests

SOURCES += \
    tst_database.cpp \
    ../database.cpp

HEADERS += \
    ../database.h

# Make it easy to turn on coverage from CI: qmake "CONFIG+=coverage"
coverage {
    QMAKE_CXXFLAGS += --coverage -O0 -g
    QMAKE_LFLAGS += --coverage
}
