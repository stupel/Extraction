#include "extraction.h"

Extraction::Extraction()
{
    this->extractionIsRunning = false;

    // INPUT PARAMS
    this->input.isSequence = false;
    this->input.cnt = 0;
    this->input.quality = 100;
    this->input.loaded = false;

    //EXTRACTION FEATURES
    this->extractionFeatures.useISOConverter = false;
    this->extractionFeatures.useOrientationFixer = true;
    this->extractionFeatures.useVariableBlockSize = false;

    // EXTRACTION PARAMS
    this->extractionParams.blockSize = 19;
    this->extractionParams.modelLoaded = false;
    this->extractionParams.cpuOnly = false;
    this->extractionParams.caffeFiles.model = "./core/config/Caffe/extraction_deploy.prototxt";
    this->extractionParams.caffeFiles.trained = "./core/config/Caffe/extraction.caffemodel";
    this->extractionParams.caffeFiles.imageMean = "./core/config/Caffe/extraction_imagemean.binaryproto";
    this->extractionParams.caffeFiles.label = "./core/config/Caffe/extraction_labels.txt";

    // NEURAL CHECKER PARAMS
    this->neuralCheckerParams.minutiaCN = &this->results.minutiaeCN;
    this->neuralCheckerParams.blockSize = &this->extractionParams.blockSize;
    this->neuralCheckerParams.useVariableBlockSize = &this->extractionFeatures.useVariableBlockSize;
    this->neuralCheckerParams.cpuOnly = &this->extractionParams.cpuOnly;

    connect(&this->orientationFixer, SIGNAL(extractionErrorSignal(int)), this, SLOT(extractionError(int)));
}

void Extraction::cleanResults()
{
    this->results.minutiaeCN.clear();
    this->results.minutiaePredictedFixed.clear();
    this->results.minutiaePredicted.clear();

    this->crossingNumber.clean();
    this->neuralChecker.clean();
}

void Extraction::cleanSequenceResults()
{
    this->resultsMap.clear();
    this->resultsMinutiaeMap.clear();
    this->resultsISOMap.clear();
}

void Extraction::cleanDurations()
{
    this->durations.crossingNumber = 0;
    this->durations.orientationFixer = 0;
    this->durations.neuralChecker = 0;
    this->durations.isoConverter = 0;
}

int Extraction::setExtractionParams(CAFFE_FILES extractionFiles, int extractionBlockSize)
{
    if (this->extractionIsRunning) {
        this->extractionError(10);
        return -1;
    }

    this->extractionParams.caffeFiles = extractionFiles;
    this->extractionParams.blockSize = extractionBlockSize;

    this->neuralChecker.loadModel(this->extractionParams.caffeFiles);
    this->extractionParams.modelLoaded = true;

    return 1;
}

int Extraction::setFeatures(bool useISOConverter, bool useOrientationFixer, bool useVariableBlockSize)
{
    if (this->extractionIsRunning) {
        this->extractionError(10);
        return -1;
    }

    this->extractionFeatures.useISOConverter = useISOConverter;
    this->extractionFeatures.useOrientationFixer = useOrientationFixer;
    this->extractionFeatures.useVariableBlockSize = useVariableBlockSize;

    return 1;
}

int Extraction::setCPUOnly(bool enabled)
{
    if (this->extractionIsRunning) {
        this->extractionError(10);
        return -1;
    }

    this->extractionParams.cpuOnly = enabled;

    return 1;
}

int Extraction::loadInput(cv::Mat imgOriginal, cv::Mat imgSkeleton, cv::Mat orientationMap, int fpQuality, cv::Mat qualityMap, cv::Mat imgSkeletonInverted)
{
    if (this->extractionIsRunning) {
        this->extractionError(10);
        return -1;
    }

    this->input.one.imgOriginal = imgOriginal.clone();
    this->input.one.imgSkeleton = imgSkeleton.clone();
    this->input.one.imgSkeletonInverted = imgSkeletonInverted.clone();
    this->input.one.orientationMap = orientationMap.clone();
    this->input.one.qualityMap = qualityMap.clone();
    this->input.quality = fpQuality;
    this->input.loaded = true;

    if (this->input.one.imgOriginal.channels() != 1) cv::cvtColor(this->input.one.imgOriginal, this->input.one.imgOriginal, CV_BGR2GRAY);
    if (this->input.one.imgSkeleton.channels() != 1) cv::cvtColor(this->input.one.imgSkeleton, this->input.one.imgSkeleton, CV_BGR2GRAY);
    if (this->input.one.imgSkeletonInverted.channels() != 1) cv::cvtColor(this->input.one.imgSkeletonInverted, this->input.one.imgSkeletonInverted, CV_BGR2GRAY);
    if (this->input.one.qualityMap.channels() != 1) cv::cvtColor(this->input.one.qualityMap, this->input.one.qualityMap, CV_BGR2GRAY);

    return 1;
}

int Extraction::loadInput(PREPROCESSING_RESULTS preprocessingResults)
{
    if (this->extractionIsRunning) {
        this->extractionError(10);
        return -1;
    }

    this->input.one = preprocessingResults;
    this->input.quality = 100;
    this->input.loaded = true;

    if (this->input.one.imgOriginal.channels() != 1) cv::cvtColor(this->input.one.imgOriginal, this->input.one.imgOriginal, CV_BGR2GRAY);
    if (this->input.one.imgSkeleton.channels() != 1) cv::cvtColor(this->input.one.imgSkeleton, this->input.one.imgSkeleton, CV_BGR2GRAY);
    if (this->input.one.imgSkeletonInverted.channels() != 1) cv::cvtColor(this->input.one.imgSkeletonInverted, this->input.one.imgSkeletonInverted, CV_BGR2GRAY);
    if (this->input.one.qualityMap.channels() != 1) cv::cvtColor(this->input.one.qualityMap, this->input.one.qualityMap, CV_BGR2GRAY);

    return 1;
}

int Extraction::loadInput(QMap<QString, PREPROCESSING_RESULTS> preprocessingResults)
{
    if (this->extractionIsRunning) {
        this->extractionError(10);
        return -1;
    }

    this->input.sequence = preprocessingResults;
    this->input.isSequence = true;
    this->input.quality = 100;
    this->input.loaded = true;

    this->input.keys.clear();
    for (auto i = this->input.sequence.begin(); i != this->input.sequence.end(); ++i) {
        this->input.keys.push_back(i.key());
        if (i.value().imgOriginal.channels() != 1) cv::cvtColor(i.value().imgOriginal, i.value().imgOriginal, CV_BGR2GRAY);
        if (i.value().imgSkeleton.channels() != 1) cv::cvtColor(i.value().imgSkeleton, i.value().imgSkeleton, CV_BGR2GRAY);
        if (i.value().imgSkeletonInverted.channels() != 1) cv::cvtColor(i.value().imgSkeletonInverted, i.value().imgSkeletonInverted, CV_BGR2GRAY);
        if (i.value().qualityMap.channels() != 1) cv::cvtColor(i.value().qualityMap, i.value().qualityMap, CV_BGR2GRAY);
    }

    return 1;
}

void Extraction::run()
{
    if (!this->extractionIsRunning) {
        this->extractionIsRunning = true;
        this->cleanResults();
        this->cleanSequenceResults();
        this->cleanDurations();

        if (!this->input.isSequence) this->startExtraction(this->input.one);
        else if (!this->input.sequence.empty()) this->startExtraction(this->input.sequence.value(this->input.keys[0]));
    }
    else this->extractionError(10);
}

void Extraction::startExtraction(const PREPROCESSING_RESULTS &input)
{
    //CROSSING NUMBER
    this->crossingNumber.setParams(input.imgSkeleton, input);

    this->timer.start();
    this->crossingNumber.findMinutiae();
    this->durations.crossingNumber += this->timer.elapsed();
    this->results.minutiaeCN = this->crossingNumber.getMinutiae();

    //NEURAL CHECKER
    this->neuralChecker.setParams(input.imgOriginal, this->neuralCheckerParams);
    if (!this->extractionParams.modelLoaded) {
        this->neuralChecker.loadModel(this->extractionParams.caffeFiles);
        this->extractionParams.modelLoaded = true;
    }

    this->timer.start();
    this->neuralChecker.check();
    this->durations.neuralChecker += this->timer.elapsed();

    this->results.minutiaePredicted = this->neuralChecker.getCheckedMinutiae();

    if (this->extractionFeatures.useOrientationFixer) {        
        //INVERTED CROSSING NUMBER
        this->crossingNumber.setParams(input.imgSkeletonInverted, input);

        this->timer.start();
        this->crossingNumber.findMinutiae();
        this->durations.crossingNumber += this->timer.elapsed();

        QVector<MINUTIA> invertedMinutiae = this->crossingNumber.getMinutiae();

        //ORIENTATION FIXER
        this->orientationFixer.setParams(input, this->results.minutiaePredicted, invertedMinutiae);

        this->timer.start();
        this->orientationFixer.fix();
        this->durations.orientationFixer += this->timer.elapsed();

        this->results.minutiaePredictedFixed = this->orientationFixer.getFixedMinutiae();

        this->orientationFixer.finalizeDirections(this->results.minutiaePredictedFixed);
    }

    this->orientationFixer.finalizeDirections(this->results.minutiaeCN);
    this->orientationFixer.finalizeDirections(this->results.minutiaePredicted);

    //ISO CONVERTER
    if (this->extractionFeatures.useISOConverter) {
        if (this->extractionFeatures.useOrientationFixer)
            this->isoConverter.load(input.imgOriginal.rows, input.imgOriginal.cols, this->input.quality, this->results.minutiaePredictedFixed);
        else this->isoConverter.load(input.imgOriginal.rows, input.imgOriginal.cols, this->input.quality, this->results.minutiaePredicted);

        this->timer.start();
        this->results.minutiaeISO = this->isoConverter.convert();
        this->durations.isoConverter += this->timer.elapsed();
    }

    // IF WE HAVE ONLY ONE INPUT
    if (!this->input.isSequence) {

        //SIGNALS
        emit extractionDoneSignal(this->results);
        if (this->extractionFeatures.useOrientationFixer) emit extractionDoneSignal(this->results.minutiaePredictedFixed);
        else emit extractionDoneSignal(this->results.minutiaePredicted);
        if (this->extractionFeatures.useISOConverter) emit extractionDoneSignal(this->results.minutiaeISO);
        emit extractionDurationsSignal(this->durations);

        this->extractionIsRunning = false;
    }
    // IF WE HAVE MORE INPUTS
    else {
        // SAVE RESULTS
        this->resultsMap.insert(this->input.keys[this->input.cnt], this->results);
        if (this->extractionFeatures.useOrientationFixer) this->resultsMinutiaeMap.insert(this->input.keys[this->input.cnt], this->results.minutiaePredictedFixed);
        else this->resultsMinutiaeMap.insert(this->input.keys[this->input.cnt], this->results.minutiaePredicted);
        if (this->extractionFeatures.useISOConverter) this->resultsISOMap.insert(this->input.keys[this->input.cnt], this->results.minutiaeISO);

        this->cleanResults();

        // EXTRACTION PROGRESS
        emit this->extractionProgressSignal((int)(this->input.cnt*1.0/(this->input.sequence.size()-1)*100));

        // IF WE EXTRACTED ALL INPUTS
        if (this->input.cnt == this->input.sequence.size()-1) {
            this->extractionIsRunning = false;

            if (this->extractionFeatures.useISOConverter) emit this->extractionSequenceDoneSignal(this->resultsISOMap);
            emit this->extractionSequenceDoneSignal(this->resultsMap);
            emit this->extractionSequenceDoneSignal(this->resultsMinutiaeMap);

            this->cleanSequenceResults();
        }
        // ELSE STRAT EXTRACT THE NEXT INPUT
        else {
            this->startExtraction(this->input.sequence.value(this->input.keys[++this->input.cnt]));
        }
    }
}

void Extraction::extractionError(int errorCode)
{
    /* Error Signals:
     * 10 - Extraction is already running
     * 21 - OrientationFixer: Inverted skeleton not loaded
     *
     *
     */

    emit this->extractionErrorSignal(errorCode);
}
