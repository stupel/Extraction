#include "orientationfixer.h"

OrientationFixer::OrientationFixer()
{

}

void OrientationFixer::setParams(const cv::Mat &imgSkeleton, const cv::Mat &imgInvertedSkeleton, const QVector<MINUTIA> &minutiae, const QVector<MINUTIA> &invertedMinutiae)
{
    this->imgSkeleton = imgSkeleton;
    this->imgInvertedSkeleton = imgInvertedSkeleton;
    this->minutiae = minutiae;
    this->invertedMinutiae = invertedMinutiae;
}

void OrientationFixer::fix()
{
    this->fixedMinutiae = this->minutiae;

    this->fixEndings();
    if (this->imgInvertedSkeleton.cols > 0 && this->imgInvertedSkeleton.rows > 0) {
        this->fixBifurcations();
    }
    emit errorSignal(21);
}

void OrientationFixer::fixEndings()
{
    int centerPixel = -1;
    int incomingPixel = -1;

    for (MINUTIA &minutia : this->fixedMinutiae) {
        // len ukoncenia
        if (minutia.type == 0) {

            // ukoncenie prichadza z laveho horneho rohu
            // x 0 0
            // 0 x 0
            // 0 0 0
            centerPixel = this->imgSkeleton.at<uchar>(minutia.xy.y(), minutia.xy.x());
            incomingPixel = this->imgSkeleton.at<uchar>(minutia.xy.y() - 1, minutia.xy.x()  -1);
            if (centerPixel == 0 && incomingPixel == 0) {
                minutia.angle += M_PI;
            }

            // ukoncenie prichadza zhora
            // 0 x 0
            // 0 x 0
            // 0 0 0
            incomingPixel = this->imgSkeleton.at<uchar>(minutia.xy.y() - 1, minutia.xy.x());
            if(centerPixel == 0 && incomingPixel == 0){
                minutia.angle += M_PI;
            }

            // ukoncenie prichadza z praveho horneho rohu
            // 0 0 x
            // 0 x 0
            // 0 0 0
            incomingPixel = this->imgSkeleton.at<uchar>(minutia.xy.y() - 1, minutia.xy.x() + 1);
            if(centerPixel == 0 && incomingPixel == 0){
                minutia.angle += M_PI;
            }

            // ukoncenie prichadza zlava
            // 0 0 0
            // x x 0
            // 0 0 0
            incomingPixel = this->imgSkeleton.at<uchar>(minutia.xy.y(), minutia.xy.x() - 1);
            if((centerPixel == 0 && incomingPixel == 0)){
                if(minutia.angle<=M_PI_2){
                    minutia.angle += M_PI;
                }
            }

            // ukoncenie prichadza zprava
            // 0 0 0
            // 0 x x
            // 0 0 0
            incomingPixel = this->imgSkeleton.at<uchar>(minutia.xy.y(), minutia.xy.x() + 1);
            if((centerPixel == 0 && incomingPixel == 0)){
                if(minutia.angle>M_PI_2){
                    minutia.angle += M_PI;
                }
            }
        }
    }
}

void OrientationFixer::fixBifurcations()
{
    int centerPixel = -1;
    int incomingPixel = -1;

    for (MINUTIA &minutia : this->fixedMinutiae) {
        // len vidlice
        if (minutia.type == 1) {

            MINUTIA closestMinutia;
            qreal minLength = 999999;
            qreal currentLength = -1;

            for(MINUTIA invertedMinutia : invertedMinutiae) {
                if (invertedMinutia.type == 0) {
                    QLineF line(QPointF(minutia.xy.x(), minutia.xy.y()), QPointF(invertedMinutia.xy.x(), invertedMinutia.xy.y()));
                    currentLength = line.length();
                    if(currentLength < minLength){
                        minLength = currentLength;
                        closestMinutia = invertedMinutia;
                    }
                }
            }

            // 0 0 0
            // 0 x 0
            // x 0 0
            centerPixel = this->imgInvertedSkeleton.at<uchar>(closestMinutia.xy.y(),closestMinutia.xy.x());
            incomingPixel = this->imgInvertedSkeleton.at<uchar>(closestMinutia.xy.y()+1,closestMinutia.xy.x()-1);
            if (centerPixel == 0 && incomingPixel == 0) {
                minutia.angle += M_PI;
            }

            // 0 0 0
            // 0 x 0
            // 0 x 0
            incomingPixel = this->imgInvertedSkeleton.at<uchar>(closestMinutia.xy.y()+1,closestMinutia.xy.x());
            if (centerPixel == 0 && incomingPixel == 0) {
                minutia.angle += M_PI;
            }


            // 0 0 0
            // 0 x 0
            // 0 0 x
            incomingPixel = this->imgInvertedSkeleton.at<uchar>(closestMinutia.xy.y() + 1, closestMinutia.xy.x() + 1);
            if (centerPixel == 0 && incomingPixel == 0) {
                minutia.angle += M_PI;
            }

            // 0 0 0
            // x x 0
            // 0 0 0
            incomingPixel = this->imgInvertedSkeleton.at<uchar>(closestMinutia.xy.y(), closestMinutia.xy.x() - 1);
            if ((centerPixel == 0 && incomingPixel == 0)) {
                if (minutia.angle > M_PI_2) {
                    minutia.angle += M_PI;
                }
            }

            // 0 0 0
            // 0 x x
            // 0 0 0
            incomingPixel = this->imgInvertedSkeleton.at<uchar>(closestMinutia.xy.y(), closestMinutia.xy.x()+1);
            if ((centerPixel == 0 && incomingPixel == 0)) {
                if (minutia.angle <= M_PI_2) {
                    minutia.angle += M_PI;
                }
            }
        }
    }
}

QVector<MINUTIA> OrientationFixer::getFixedMinutiae() const
{
    return fixedMinutiae;
}
