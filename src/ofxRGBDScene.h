#pragma once
#include "ofMain.h"

class ofxRGBDScene {
  public:
    ofxRGBDScene();
    static bool isFolderValid(string sourceMediaFolder);
    bool loadFromFolder(string sourceMediaFolder, bool countFrames = true);
    bool valid();

    bool hasPairings;
    bool hasXYShift;
    bool hasCalibration;
    bool hasDepth;
    bool hasColor;
    bool hasAlternativeHiResVideo;

    string name;
    string mediaFolder;
    string calibrationFolder;
    string videoPath;
    string alternativeHiResVideoPath;
    string depthFolder;
    string pairingsFile;
    string xyshiftFile; //file to save the xy fudge params
    string pathDelim;

    int totalDepthFrameCount;
    int compressedDepthFrameCount;
    int uncompressedDepthFrameCount;

    void clear();
    
};
