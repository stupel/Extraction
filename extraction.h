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
    QVector<MINUTIA> minutiaeCN;
    QVector<MINUTIA> minutiaePredicted;
    QVector<MINUTIA> minutiaePredictedFixed;
    unsigned char * minutiaeISO;
} EXTRACTION_RESULTS;

class EXTRACTIONSHARED_EXPORT Extraction : public QThread
{
    Q_OBJECT

public:
    Extraction();

    // REQUIRED
    int loadInput(cv::Mat imgOriginal, cv::Mat imgSkeleton, cv::Mat orientationMap, int fpQuality = 100, cv::Mat qualityMap = cv::Mat(0,0,CV_8UC1), cv::Mat imgSkeletonInverted = cv::Mat(0,0,CV_8UC1));
    int loadInput(PREPROCESSING_RESULTS preprocessingResults);
    int loadInput(QMap<QString, PREPROCESSING_RESULTS> preprocessingResults);

    void run();

    // OPTIONAL
    int setExtractionParams(CAFFE_FILES extractionFiles, int extractionBlockSize);
    int setFeatures(bool useISOConverter, bool useOrientationFixer = true, bool useVariableBlockSize = false);
    int setCPUOnly(bool enabled);


private:
    CrossingNumber crossingNumber;
    OrientationFixer orientationFixer;
    NeuralChecker neuralChecker;
    ISOConverter isoConverter;

    QTime timer;

    bool extractionIsRunning;

    // INPUT
    EXTRACTION_INPUT input;
    EXTRACTION_PARAMS extractionParams;
    EXTRACTION_FEATURES extractionFeatures;

    // PARAMS
    NEURAL_CHECKER_PARAMS neuralCheckerParams;

    // OUTPUT
    EXTRACTION_DURATIONS durations;
    EXTRACTION_RESULTS results;
    QMap<QString, EXTRACTION_RESULTS> resultsMap;
    QMap<QString, QVector<MINUTIA>> resultsMinutiaeMap;
    QMap<QString, unsigned char*> resultsISOMap;


    // PRIVATE FUNCTIONS
    void startExtraction(const PREPROCESSING_RESULTS &input);
    void cleanResults();
    void cleanSequenceResults();
    void cleanDurations();

private slots:
    void extractionError(int errorCode);

signals:
    void extractionDoneSignal(EXTRACTION_RESULTS results);
    void extractionDoneSignal(QVector<MINUTIA> minutiae);
    void extractionDoneSignal(unsigned char * minutiaeISO);

    void extractionSequenceDoneSignal(QMap<QString, EXTRACTION_RESULTS> results);
    void extractionSequenceDoneSignal(QMap<QString, QVector<MINUTIA>> minutiaeMap);
    void extractionSequenceDoneSignal(QMap<QString, unsigned char *> minutiaeISO);

    void extractionDurationsSignal(EXTRACTION_DURATIONS durations);
    void extractionProgressSignal(int progress);
    void extractionErrorSignal(int errorCode);
};

#endif // EXTRACTION_H
