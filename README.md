# Extraction
Fingerprint extraction module for DBOX  
  
**Dependencies:**
- OpenCV 3.4.1 (https://github.com/opencv/opencv)  
- Caffe 1.0 (https://github.com/BVLC/caffe)  
- CUDA 9.1 (https://developer.nvidia.com/cuda-downloads)  
- Qt5 / Qt Creator 4 (https://www.qt.io/download)
  
*The mentioned versions are recommended*  
  
**Getting Started:**
1. Build and run the project to generate .so (.dll / .lib) files  
2. Include the library and header files to your own application  
3. Copy the 'core' folder to your root project directory  
  
  *This library supports QThread*
  
  <br />
  
**APIs:**  
```cpp  
void loadInput(cv::Mat imgOriginal, cv::Mat imgSkeleton, cv::Mat orientationMap, int fpQuality = 100, cv::Mat imgInvertedSkeleton = cv::Mat(0,0,CV_8UC1));  
  
void start();  
```  
  
Optional:  
```cpp  
void setParams(int extractionBlockSize);  
  
void setFeatures(bool useOrientationFixer, bool useVariableBlockSize);  
  
void setCaffeFiles(CAFFE_FILES extractionFiles);  
``` 
  
  <br />
  
**SIGNALS:**  
```cpp  
void extractionResultsSignal(EXTRACTION_RESULTS results);  
  
void minutiaeVectorDoneSignal(QVector<MINUTIA> minutiae);  
  
void ISOTemplateDoneSignal(unsigned char* minutiaeISO);  
  
void extractionDurationsSignal(EXTRACTION_DURATIONS durations);  

void extractionErrorSignal(int errorCode);
```  
