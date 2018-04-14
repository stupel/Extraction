#ifndef CROSSINGNUMBER_H
#define CROSSINGNUMBER_H

#include "extraction_config.h"

class CrossingNumber : public QObject
{
    Q_OBJECT

public:
    CrossingNumber();

    void setParams(const cv::Mat &imgSkeleton, const cv::Mat &orientationMap);
    void findMinutiae();
    void clean();

    QVector<MINUTIA> getMinutiae() const;

private:

    cv::Mat imgSkeleton;
    cv::Mat orientationMap;

    QVector<MINUTIA> minutiae;
};

#endif // CROSSINGNUMBER_H
