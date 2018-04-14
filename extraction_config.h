#ifndef EXTRACTION_CONFIG_H
#define EXTRACTION_CONFIG_H

//OpenCV
#include "opencv2/opencv.hpp"

//Qt
#include <QObject>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QLineF>

//std
#include <string>
#include <bitset>

typedef struct minutia {
    QPoint xy;
    int type; // 0-end, 1-bif
    qreal angle; // in radians
    int quality;
} MINUTIA;

#ifndef CAFFE_FILES_DEFINED
    typedef struct caffe_files {
        QString model;
        QString trained;
        QString imageMean;
        QString label;
    } CAFFE_FILES;
#define CAFFE_FILES_DEFINED
#endif

#endif // EXTRACTION_CONFIG_H
