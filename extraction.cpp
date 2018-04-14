#include "extraction.h"

Extraction::Extraction()
{
    this->extractionFiles.model = "./core/config/Caffe/extraction_deploy.prototxt";
    this->extractionFiles.trained = "./core/config/Caffe/extraction.caffemodel";
    this->extractionFiles.imageMean = "./core/config/Caffe/extraction_imagemean.binaryproto";
    this->extractionFiles.label = "./core/config/Caffe/extraction_labels.txt";

    this->extractionBlockSize = 19;
    this->useOrientationFixer = true;
    this->useVariableBlockSize = false;

    this->neuralChecker.loadModel(this->extractionFiles);

    /* Error Signals:
     * 21 - OrientationFixer: Inverted skeleton not loaded
     *
     *
     */
}

void Extraction::clean()
{
    this->results.minutiae.clear();
    this->results.checkedFixedMinutiae.clear();
    this->results.checkedMinutiae.clear();
    this->invertedMinutiae.clear();
    delete this->results.minutiaeISO;

    this->imgOriginal.release();
    this->imgSkeleton.release();
    this->imgInvertedSkeleton.release();
    this->orientationMap.release();

    this->crossingNumber.clean();
    this->neuralChecker.clean();

    this->durations.crossingNumber = 0;
    this->durations.orientationFixer = 0;
    this->durations.neuralChecker = 0;
    this->durations.isoConverter = 0;
}

void Extraction::loadInput(const cv::Mat &imgOriginal, const cv::Mat &imgSkeleton, const cv::Mat &imgInvertedSkeleton, const cv::Mat &orientationMap, int fpQuality)
{
    this->imgOriginal = imgOriginal;
    this->imgSkeleton = imgSkeleton;
    this->imgInvertedSkeleton = imgInvertedSkeleton;
    this->orientationMap = orientationMap;
    this->fpQuality = fpQuality;
}

void Extraction::setCaffeFiles(CAFFE_FILES extractionFiles)
{
    this->extractionFiles = extractionFiles;

    this->neuralChecker.loadModel(this->extractionFiles);
}

void Extraction::setParams(int extractionBlockSize)
{
    this->extractionBlockSize = extractionBlockSize;
}

void Extraction::setFeatures(bool useOrientationFixer, bool useVariableBlockSize)
{
    this->useOrientationFixer = useOrientationFixer;
    this->useVariableBlockSize = useVariableBlockSize;
}

void Extraction::run()
{
    //Crossing Number
    this->crossingNumber.setParams(this->imgSkeleton, this->orientationMap);

    this->timer.start();
    this->crossingNumber.findMinutiae();
    this->durations.crossingNumber = this->timer.elapsed();
    this->results.minutiae = this->crossingNumber.getMinutiae();

    //Neural Checker
    this->neuralChecker.setParams(this->imgOriginal, this->results.minutiae, this->extractionBlockSize, this->useVariableBlockSize);

    this->timer.start();
    this->neuralChecker.check();
    this->durations.neuralChecker = this->timer.elapsed();

    this->results.checkedMinutiae = this->neuralChecker.getCheckedMinutiae();

    if (this->useOrientationFixer) {
        //Inverted Crossing Number
        this->crossingNumber.setParams(this->imgInvertedSkeleton, this->orientationMap);

        this->timer.start();
        this->crossingNumber.findMinutiae();
        this->durations.crossingNumber += this->timer.elapsed();

        this->invertedMinutiae = this->crossingNumber.getMinutiae();

        //Orietation Fixer
        this->orientationFixer.setParams(this->imgSkeleton, this->imgInvertedSkeleton, this->results.checkedMinutiae, this->invertedMinutiae);

        this->timer.start();
        this->orientationFixer.fix();
        this->durations.orientationFixer = this->timer.elapsed();

        this->results.checkedFixedMinutiae = this->orientationFixer.getFixedMinutiae();
    }

    //ISO Converter
    this->isoConverter.load(this->imgOriginal.rows, this->imgOriginal.cols, this->fpQuality, this->results.checkedFixedMinutiae);

    this->timer.start();
    this->results.minutiaeISO = this->isoConverter.convert();
    this->durations.isoConverter = this->timer.elapsed();

    //Signals
    emit extractionResultsSignal(this->results);
    emit minutiaeVectorDoneSignal(this->results.checkedMinutiae);
    emit ISOTemplateDoneSignal(this->results.minutiaeISO);
    emit extractionDurationsSignal(this->durations);

    this->clean();
}
