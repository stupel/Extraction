#include "crossingnumber.h"

CrossingNumber::CrossingNumber()
{

}

void CrossingNumber::setParams(const cv::Mat &imgSkeleton, const PREPROCESSING_RESULTS &input)
{
    this->imgSkeleton = imgSkeleton;
    this->input = input;
}

void CrossingNumber::findMinutiae()
{
    int cn;
    int quality;

    for (int y = 0; y < this->imgSkeleton.rows; y++) {
        for (int x = 0; x < this->imgSkeleton.cols; x++) {
            if (this->imgSkeleton.at<uchar>(y, x) == 0) {
                cn = abs((this->imgSkeleton.at<uchar>(y - 1, x - 1)) - (this->imgSkeleton.at<uchar>(y, x - 1))) +
                        abs((this->imgSkeleton.at<uchar>(y, x - 1)) - (this->imgSkeleton.at<uchar>(y + 1, x - 1))) +
                        abs((this->imgSkeleton.at<uchar>(y + 1, x - 1)) - (this->imgSkeleton.at<uchar>(y + 1, x))) +
                        abs((this->imgSkeleton.at<uchar>(y + 1, x)) - (this->imgSkeleton.at<uchar>(y + 1, x + 1))) +
                        abs((this->imgSkeleton.at<uchar>(y + 1, x + 1)) - (this->imgSkeleton.at<uchar>(y, x + 1))) +
                        abs((this->imgSkeleton.at<uchar>(y, x + 1)) - (this->imgSkeleton.at<uchar>(y - 1, x + 1))) +
                        abs((this->imgSkeleton.at<uchar>(y - 1, x + 1)) - (this->imgSkeleton.at<uchar>(y - 1, x))) +
                        abs((this->imgSkeleton.at<uchar>(y - 1, x)) - (this->imgSkeleton.at<uchar>(y - 1, x - 1)));

                if (this->input.qualityMap.cols > x && this->input.qualityMap.rows > y) quality = this->input.qualityMap.at<uchar>(y,x);
                else quality = 254;

                if (cn / 255 / 2 == 1) {
                    this->minutiae.push_back(MINUTIA{QPoint{x,y}, 0, this->input.orientationMap.at<float>(y,x) + M_PI_2, quality, QPoint{this->input.imgOriginal.cols, this->input.imgOriginal.rows}});
                }
                else if (cn / 255 / 2 == 3) {
                    this->minutiae.push_back(MINUTIA{QPoint{x,y}, 1, this->input.orientationMap.at<float>(y,x) + M_PI_2, quality, QPoint{this->input.imgOriginal.cols, this->input.imgOriginal.rows}});
                }
            }
        }
    }
}

QVector<MINUTIA> CrossingNumber::getMinutiae() const
{
    return minutiae;
}

void CrossingNumber::clean()
{
    this->minutiae.clear();
}
