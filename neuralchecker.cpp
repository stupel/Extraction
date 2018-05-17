#include "neuralchecker.h"

NeuralChecker::NeuralChecker()
{
    this->isNetworkLoaded = false;
}

void NeuralChecker::setParams(const cv::Mat &imgOriginal, const NEURAL_CHECKER_PARAMS &neuralCheckerParams)
{
    this->imgOriginal = imgOriginal.clone();
    this->neuralChecker = neuralCheckerParams;

    cv::copyMakeBorder(this->imgOriginal, this->imgOriginal, *this->neuralChecker.blockSize, *this->neuralChecker.blockSize, *this->neuralChecker.blockSize, *this->neuralChecker.blockSize, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));
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

    if (*this->neuralChecker.cpuOnly) Caffe::set_mode(Caffe::CPU);
    else Caffe::set_mode(Caffe::GPU);

    cv::Mat block;
    std::vector<cv::Mat> blocks;

    for (MINUTIA minutia : *this->neuralChecker.minutiaCN) {

        block = this->imgOriginal(cv::Rect(minutia.xy.x() + *this->neuralChecker.blockSize/2, minutia.xy.y() + *this->neuralChecker.blockSize/2, *this->neuralChecker.blockSize, *this->neuralChecker.blockSize));
        blocks.push_back(block);

        if (*this->neuralChecker.useVariableBlockSize) {
            for (int i = 1; i <= VARBLOCKITER; i++) {
                block = this->imgOriginal(cv::Rect(minutia.xy.x() + *this->neuralChecker.blockSize/2 - i*VARBLOCKSTEP, minutia.xy.y() + *this->neuralChecker.blockSize/2 - i*VARBLOCKSTEP, *this->neuralChecker.blockSize+ i*VARBLOCKSTEP, *this->neuralChecker.blockSize + i*VARBLOCKSTEP));
                blocks.push_back(block);
            }
        }
    }

    std::vector<std::vector<Prediction>> predictions;

    //Use Batch
    predictions = this->classifier->classifyBatch(blocks, 3);

    //Without Batch
    /*for (int i = 0; i < blocks.size(); i++) {
        //qDebug() << "output: " << i+1;
        predictions.push_back(this->classifier->classify(blocks[i]));
    }*/

    int i = 0;

    int varBlockIteration = 0;
    float bestBlock[3] = {0,0,0};

    for (std::vector<Prediction> prediction : predictions) {
        if (*this->neuralChecker.useVariableBlockSize) {
            if (varBlockIteration > VARBLOCKITER) {
                varBlockIteration = 0;

                if (bestBlock[0] > bestBlock[1] && bestBlock[0] > bestBlock[2])
                    this->checkedMinutiae.push_back(MINUTIA{QPoint{this->neuralChecker.minutiaCN->at(i).xy.x(), this->neuralChecker.minutiaCN->at(i).xy.y()}, 0, this->neuralChecker.minutiaCN->at(i).angle, this->neuralChecker.minutiaCN->at(i).quality, this->neuralChecker.minutiaCN->at(i).imgWH});
                else if (bestBlock[1] > bestBlock[0] && bestBlock[1] > bestBlock[2])
                    this->checkedMinutiae.push_back(MINUTIA{QPoint{this->neuralChecker.minutiaCN->at(i).xy.x(), this->neuralChecker.minutiaCN->at(i).xy.y()}, 1, this->neuralChecker.minutiaCN->at(i).angle, this->neuralChecker.minutiaCN->at(i).quality, this->neuralChecker.minutiaCN->at(i).imgWH});

                for (int j = 0; j < 3; j++) bestBlock[j] = 0;
                i++;
            }

            if (prediction[0].first[0] == 'E' || prediction[0].first[0] == 'e') bestBlock[0] += prediction[0].second;
            else if (prediction[0].first[0] == 'B' || prediction[0].first[0] == 'b') bestBlock[1] += prediction[0].second;
            else bestBlock[2] += prediction[0].second;

            varBlockIteration++;
        }
        else {
            if ((prediction[0].first[0] == 'E' || prediction[0].first[0] == 'e') /*&& prediction[0].second > 0.9999 && this->neuralChecker.minutiaCN->at(i).quality > 26*/) {
                this->checkedMinutiae.push_back(MINUTIA{QPoint{this->neuralChecker.minutiaCN->at(i).xy.x(), this->neuralChecker.minutiaCN->at(i).xy.y()}, 0, this->neuralChecker.minutiaCN->at(i).angle, this->neuralChecker.minutiaCN->at(i).quality, this->neuralChecker.minutiaCN->at(i).imgWH});
            }
            else if ((prediction[0].first[0] == 'B' || prediction[0].first[0] == 'b')  /*&& prediction[0].second > 0.9999 && this->neuralChecker.minutiaCN->at(i).quality > 26*/) {
                this->checkedMinutiae.push_back(MINUTIA{QPoint{this->neuralChecker.minutiaCN->at(i).xy.x(), this->neuralChecker.minutiaCN->at(i).xy.y()}, 1, this->neuralChecker.minutiaCN->at(i).angle, this->neuralChecker.minutiaCN->at(i).quality, this->neuralChecker.minutiaCN->at(i).imgWH});
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
