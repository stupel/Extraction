#include "neuralchecker.h"

NeuralChecker::NeuralChecker()
{
    this->isNetworkLoaded = false;
}

void NeuralChecker::setParams(const cv::Mat &imgOriginal, const QVector<MINUTIA> &minutiae, int extractionBlockSize, bool useVariableBlockSize)
{
    this->imgOriginal = imgOriginal.clone();
    this->minutiae = minutiae;
    this->extractionBlockSize = extractionBlockSize;
    this->useVariableBlockSize = useVariableBlockSize;

    cv::copyMakeBorder(this->imgOriginal, this->imgOriginal, this->extractionBlockSize, this->extractionBlockSize, this->extractionBlockSize, this->extractionBlockSize, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));
}

void NeuralChecker::loadModel(const CAFFE_FILES &extractionFiles)
{
    if (this->isNetworkLoaded) {
        delete this->classifier;
        this->isNetworkLoaded = false;
    }

    this->classifier = new ExtractionCaffeNetwork;

    this->classifier->loadModel(extractionFiles.model, extractionFiles.trained, extractionFiles.imageMean, extractionFiles.label);
    this->isNetworkLoaded = true;
}

void NeuralChecker::check()
{

#ifdef CPU_ONLY
    Caffe::set_mode(Caffe::CPU);
#else
    Caffe::set_mode(Caffe::GPU);
#endif

    cv::Mat block;
    std::vector<cv::Mat> blocks;

    for (MINUTIA minutia : this->minutiae) {

        block = this->imgOriginal(cv::Rect(minutia.xy.x() + this->extractionBlockSize/2, minutia.xy.y() + this->extractionBlockSize/2, this->extractionBlockSize, this->extractionBlockSize));
        blocks.push_back(block);

        if (this->useVariableBlockSize) {
            for (int i = 1; i <= VARBLOCKITER; i++) {
                block = this->imgOriginal(cv::Rect(minutia.xy.x() + this->extractionBlockSize/2 - i*VARBLOCKSTEP, minutia.xy.y() + this->extractionBlockSize/2 - i*VARBLOCKSTEP, this->extractionBlockSize + i*VARBLOCKSTEP, this->extractionBlockSize + i*VARBLOCKSTEP));
                blocks.push_back(block);
            }
        }
    }

    std::vector<std::vector<Prediction>> predictions;

    //Use Batch
    predictions = this->classifier->classifyBatch(blocks, 3);

    //Without Batch
    /*for (int i = 0; i < blocks.size(); i++) {
        predictions.push_back(this->classifier->classify(blocks[i]));
    }*/

    int i = 0;

    int varBlockIteration = 0;
    float bestBlock[3] = {0,0,0};

    for (std::vector<Prediction> prediction : predictions) {
        if (useVariableBlockSize) {
            if (varBlockIteration > VARBLOCKITER) {
                varBlockIteration = 0;

                if (bestBlock[0] > bestBlock[1] && bestBlock[0] > bestBlock[2])
                    this->checkedMinutiae.push_back(MINUTIA{QPoint{this->minutiae.at(i).xy.x(), this->minutiae.at(i).xy.y()}, 0, this->minutiae.at(i).angle, 0});
                else if (bestBlock[1] > bestBlock[0] && bestBlock[1] > bestBlock[2])
                    this->checkedMinutiae.push_back(MINUTIA{QPoint{this->minutiae.at(i).xy.x(), this->minutiae.at(i).xy.y()}, 1, this->minutiae.at(i).angle, 0});

                for (int j = 0; j < 3; j++) bestBlock[j] = 0;
                i++;
            }

            if (prediction[0].first[0] == 'E') bestBlock[0] += prediction[0].second;
            else if (prediction[0].first[0] == 'B') bestBlock[1] += prediction[0].second;
            else bestBlock[2] += prediction[0].second;

            varBlockIteration++;
        }
        else {
            if (prediction[0].first[0] == 'E' /* && prediction[0].second > 0.9999*/) {
                this->checkedMinutiae.push_back(MINUTIA{QPoint{this->minutiae.at(i).xy.x(), this->minutiae.at(i).xy.y()}, 0, this->minutiae.at(i).angle, 0});
            }
            else if (prediction[0].first[0] == 'B' /* && prediction[0].second > 0.9999*/) {
                this->checkedMinutiae.push_back(MINUTIA{QPoint{this->minutiae.at(i).xy.x(), this->minutiae.at(i).xy.y()}, 1, this->minutiae.at(i).angle, 0});
            }
            i++;
        }
    }
}

QVector<MINUTIA> NeuralChecker::getCheckedMinutiae() const
{
    return checkedMinutiae;
}

void NeuralChecker::clean()
{
    this->checkedMinutiae.clear();
}
