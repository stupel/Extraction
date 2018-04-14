#ifndef NEURALCHECKER_H
#define NEURALCHECKER_H

#include "extraction_config.h"
#include "extraction_caffenetwork.h"

#define VARBLOCKSTEP 1
#define VARBLOCKITER 3

class NeuralChecker : public QThread
{
    Q_OBJECT

public:
    NeuralChecker();

    void loadModel(const CAFFE_FILES &extractionFiles);
    void setParams(const cv::Mat &imgOriginal, const QVector<MINUTIA> &minutiae, int extractionBlockSize, bool useVariableBlockSize);
    void check();
    void clean();

    //getNset
    QVector<MINUTIA> getCheckedMinutiae() const;

private:

    cv::Mat imgOriginal;

    int extractionBlockSize;
    bool useVariableBlockSize;

    QVector<MINUTIA> minutiae;
    QVector<MINUTIA> checkedMinutiae;

    ExtractionCaffeNetwork *classifier;
    bool isNetworkLoaded;
};

#endif // NEURALCHECKER_H
