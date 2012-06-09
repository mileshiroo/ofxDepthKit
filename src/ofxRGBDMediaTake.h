//
//  ofxRGBDMediaTake.h
//  ScreenLabRenderer
//
//  Created by James George on 4/16/12.
//

#pragma once
#include "ofMain.h"

class ofxRGBDMediaTake {
  public:
    ofxRGBDMediaTake();
    
    bool loadFromFolder(string sourceMediaFolder);
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
    string videoThumbsPath;
    string depthFolder;
    string pairingsFile;
    string xyshiftFile; //bogus file to save the xy fudge params
    
    int totalDepthFrameCount;
    int compressedDepthFrameCount;
    int uncompressedDepthFrameCount;

    void clear();
    
  protected:

    
};


