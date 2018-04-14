#ifndef FIXORIENTATIONS_H
#define FIXORIENTATIONS_H

#include "extraction_config.h"

class OrientationFixer : public QObject
{
    Q_OBJECT

public:
    OrientationFixer();

    void setParams(const cv::Mat &imgSkeleton, const cv::Mat &imgInvertedSkeleton, const QVector<MINUTIA> &minutiae, const QVector<MINUTIA> &invertedMinutiae);
    void fix();

    //getNset
    QVector<MINUTIA> getFixedMinutiae() const;

private:

    cv::Mat imgSkeleton;
    cv::Mat imgInvertedSkeleton;
    QVector<MINUTIA> minutiae;
    QVector<MINUTIA> invertedMinutiae;

    QVector<MINUTIA> fixedMinutiae;

    void fixEndings();
    void fixBifurcations();

signals:
    void errorSignal(int errorCode);

};

#endif // FIXORIENTATIONS_H
