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

#ifndef MINUTIA_DEFINED
typedef struct minutia {
    QPoint xy;
    int type; // 0-end, 1-bif
    qreal angle; // in radians
    int quality;
    QPoint imgWH; // image Width, Height
} MINUTIA;
#define MINUTIA_DEFINED
#endif

#ifndef CAFFE_FILES_DEFINED
    typedef struct caffe_files {
        QString model;
        QString trained;
        QString imageMean;
        QString label;
    } CAFFE_FILES;
#define CAFFE_FILES_DEFINED
#endif

#ifndef PREPROCESSING_RESULTS_DEFINED
typedef struct preprocessing_results {
    cv::Mat imgOriginal;
    cv::Mat imgSkeleton;
    cv::Mat imgSkeletonInverted;
    cv::Mat qualityMap;
    cv::Mat orientationMap;
} PREPROCESSING_RESULTS;
#define PREPROCESSING_RESULTS_DEFINED
#endif

typedef struct extraction_input {
    bool isSequence;
    PREPROCESSING_RESULTS one;
    QMap<QString, PREPROCESSING_RESULTS> sequence;
    QVector<QString> keys;
    int cnt;
    int quality;
    bool loaded;
} EXTRACTION_INPUT;

typedef struct extraction_features {
    bool useISOConverter;
    bool useOrientationFixer;
    bool useVariableBlockSize;
} EXTRACTION_FEATURES;

typedef struct extraction_params {
    CAFFE_FILES caffeFiles;
    int blockSize;
    bool modelLoaded;
    bool cpuOnly;
} EXTRACTION_PARAMS;

typedef struct neural_checker_params {
    QVector<MINUTIA> *minutiaCN;
    int *blockSize;
    bool *useVariableBlockSize;
    bool *cpuOnly;
} NEURAL_CHECKER_PARAMS;

#endif // EXTRACTION_CONFIG_H
