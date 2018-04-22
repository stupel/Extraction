#ifndef CROSSINGNUMBER_H
#define CROSSINGNUMBER_H

#include "extraction_config.h"

class CrossingNumber : public QObject
{
    Q_OBJECT

public:
    CrossingNumber();

    void setParams(const cv::Mat &imgSkeleton, const PREPROCESSING_RESULTS &input);
    void findMinutiae();
    void clean();

    QVector<MINUTIA> getMinutiae() const;

private:

    cv::Mat imgSkeleton;
    PREPROCESSING_RESULTS input;

    QVector<MINUTIA> minutiae;
};

#endif // CROSSINGNUMBER_H
