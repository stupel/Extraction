# Extraction
Fingerprint extraction module for DBOX  
  
This library is a continuation of [Preprocessing](https://github.com/stupel/Preprocessing) module, it fully supports the outputs from that library.
  
**Dependencies:**
- [OpenCV 3.4.1](https://github.com/opencv/opencv)  
- [Caffe 1.0](https://github.com/BVLC/caffe)  
- [CUDA 9.1 (minimum 8.0)](https://developer.nvidia.com/cuda-downloads) 
- [cuDNN 7.1](https://developer.nvidia.com/rdp/cudnn-download)
- [Qt5 / Qt Creator 4](https://www.qt.io/download)  
  
*The mentioned or newer versions are recommended*   
  
**Getting Started:**
1. You need to provide valid paths to these libraries and their header files in ```.pro``` file.
2. Build and run the project to generate .so (.dll / .lib) files  
3. Include the library and header files to your own application  
4. Copy the 'core' folder to your root project directory  
  
*This library supports QThread*
  
  <br />
  
## API  
**Required**
```cpp  
int loadInput(cv::Mat imgOriginal, cv::Mat imgSkeleton, cv::Mat orientationMap, int fpQuality = 100, cv::Mat qualityMap = cv::Mat(0,0,CV_8UC1), cv::Mat imgSkeletonInverted = cv::Mat(0,0,CV_8UC1));
int loadInput(PREPROCESSING_RESULTS preprocessingResults);  
int loadInput(QMap<QString, PREPROCESSING_RESULTS> preprocessingResults);  
  
void start();  
```  
Usage:
1. Load the input parameters, they can be added straight from the Preprocessing library  
- If you want to add the input parameters manually:
  - If you want to use the Orientation Fixer you have to load the inverted skeleton map  
  - If you want to have quality values for each minutia you have to load a quality map  
2. Just call ```start()```  
  
<br />   
  
**Optional:**  
```cpp  
int setExtractionParams(CAFFE_FILES extractionFiles, int extractionBlockSize);  
  
int setFeatures(bool useISOConverter, bool useOrientationFixer = true, bool useVariableBlockSize = false);  
  
int setCPUOnly(bool enabled);  
``` 
Usage:  
- With ```setExtractionParams(...)``` you can set the Caffe model files and parameteres required for classification with neural network and the block size used during the classification  
- With ```setFeatures(...)``` you can set some optional features:
  - you can convert the founded minutia to ISO/IEC 19794-2:2005 format (**important notice**: after use you need to free up the memory of the ISO templates, because they are dynamic arrays)
  - you can use Orientation Fixer to get minutiae angles from 0 to 360 degrees instead of 0 to 180 degrees 
  - you can use Variable Block Size during classification, in some cases it can be effective
- With ```setCPUOnly(...)``` you can force the library to use the CPU during classification (notice: it can be slower than the GPU)  

  <br />
  
**SIGNALS:**  
```cpp  
void extractionDoneSignal(EXTRACTION_RESULTS results);  
  
void extractionDoneSignal(QVector<MINUTIA> minutiae);  
  
void extractionDoneSignal(unsigned char* minutiaeISO);  
  
void extractionSequenceDoneSignal(QMap<QString, EXTRACTION_RESULTS> results);

void extractionSequenceDoneSignal(QMap<QString, QVector<MINUTIA>> minutiaeMap);  

void extractionSequenceDoneSignal(QMap<QString, unsigned char *> minutiaeISO);  
  
void extractionDurationsSignal(EXTRACTION_DURATIONS durations);  
  
void extractionErrorSignal(int errorCode);  
```  
Important notice:  
- The first three signals are emitted only if you loaded one input
- The next three signals are emitted only if you loaded a sequence of input
- Signals with ISO templates are emmitted only if you activated that in ```setFeatures(...)``` (remind: do not forget to free up the memory)  
- You get ```extractionDurationSignal``` with duration values in ms for each phase during extraction if it's finished successfully  
- You get ```extractionErrorSignal``` with the error code if an error occured during preprocessing

  
<br />  
<br />  
  
**A simple example how to use signals in your application**  
*yourclass.h:*
```cpp  
#include "extraction.h"

class YourClass: public QObject
{
    Q_OBJECT  
  
private:  
    Extraction e;  
    
private slots:
    void extractionResultsSlot(EXTRACTION_RESULTS result);
}
```

*yourclass.cpp:*
```cpp 
#include "yourclass.h"
YourClass::YourClass()
{
    qRegisterMetaType<EXTRACTION_RESULTS >("EXTRACTION_RESULTS");
    connect(&e, SIGNAL(extractionDoneSignal(EXTRACTION_RESULTS)), this, SLOT(extractionResultsSlot(EXTRACTION_RESULTS)));
}

void YourClass::extractionResultsSlot(EXTRACTION_RESULTS result)
{
    ...
}
```
For more please visit [Qt Signals & Slots](http://doc.qt.io/archives/qt-4.8/signalsandslots.html).
