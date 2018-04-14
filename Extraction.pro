#-------------------------------------------------
#
# Project created by QtCreator 2017-05-15T14:09:19
#
#-------------------------------------------------

QT       += gui

TARGET = Extraction
TEMPLATE = lib

DEFINES += EXTRACTION_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    extraction.cpp \
    crossingnumber.cpp \
    neuralchecker.cpp \
    isoconverter.cpp \
    orientationfixer.cpp \
    extraction_caffenetwork.cpp

HEADERS +=\
    extraction.h \
    extraction_global.h \
    crossingnumber.h \
    neuralchecker.h \
    isoconverter.h \
    orientationfixer.h \
    extraction_config.h \
    extraction_caffenetwork.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

#CUDA
unix:!macx: LIBS += -L$$PWD/../../../../../opt/cuda/lib64/ -lcudart
INCLUDEPATH += $$PWD/../../../../../opt/cuda/include
DEPENDPATH += $$PWD/../../../../../opt/cuda/include

#Caffe
unix:!macx: LIBS += -L$$PWD/../../../../../usr/lib64/ -lcaffe
unix:!macx: LIBS += -L$$PWD/../../../../../usr/lib64/ -lboost_system
unix:!macx: LIBS += -L$$PWD/../../../../../usr/lib64/ -lglog
unix:!macx: LIBS += -L$$PWD/../../../../../usr/lib64/ -lprotobuf
