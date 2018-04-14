#ifndef EXTRACTION_H
#define EXTRACTION_H

#include "extraction_global.h"
#include "extraction_caffenetwork.h"
#include "extraction_config.h"
#include "crossingnumber.h"
#include "orientationfixer.h"
#include "neuralchecker.h"
#include "isoconverter.h"

typedef struct extraction_durations {
    int crossingNumber;
    int orientationFixer;
    int neuralChecker;
    int isoConverter;
} EXTRACTION_DURATIONS;

typedef struct extraction_results {
    QVector<MINUTIA> minutiae;
    QVector<MINUTIA> checkedMinutiae;
    QVector<MINUTIA> checkedFixedMinutiae;
    unsigned char * minutiaeISO;
} EXTRACTION_RESULTS;

class EXTRACTIONSHARED_EXPORT Extraction : public QThread
{
    Q_OBJECT

public:
    Extraction();

    //required
    void loadInput(cv::Mat imgOriginal, cv::Mat imgSkeleton, cv::Mat orientationMap, int fpQuality = 100, cv::Mat imgInvertedSkeleton = cv::Mat(0,0,CV_8UC1));

    void run();

    //not required
    void setCaffeFiles(CAFFE_FILES extractionFiles);
    void setParams(int extractionBlockSize);
    void setFeatures(bool useOrientationFixer, bool useVariableBlockSize = false);

private:
    CrossingNumber crossingNumber;
    OrientationFixer orientationFixer;
    NeuralChecker neuralChecker;
    ISOConverter isoConverter;

    QTime timer;

    cv::Mat imgOriginal;
    cv::Mat imgSkeleton;
    cv::Mat imgInvertedSkeleton;
    cv::Mat orientationMap;
    int fpQuality;

    CAFFE_FILES extractionFiles;
    int extractionBlockSize;
    bool useOrientationFixer;
    bool useVariableBlockSize;

    EXTRACTION_DURATIONS durations;
    EXTRACTION_RESULTS results;

    QVector<MINUTIA> invertedMinutiae;

    bool isExtractionModelLoaded;

    void clean();

signals:
    void extractionResultsSignal(EXTRACTION_RESULTS results);
    void minutiaeVectorDoneSignal(QVector<MINUTIA> minutiae);
    void ISOTemplateDoneSignal(unsigned char * minutiaeISO);
    void extractionDurationsSignal(EXTRACTION_DURATIONS durations);
    void extractionErrorSignal(int errorCode);

};

#endif // EXTRACTION_H
