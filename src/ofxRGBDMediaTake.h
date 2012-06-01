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
    bool hasCalibration;
    bool hasDepth;
    bool hasColor;
    bool hasHiResVideo;
    bool hasLoResVideo;

    string mediaFolder;
    string calibrationFolder;
    string lowResVideoPath;
    string hiResVideoPath;
    string videoThumbsPath;
    string depthFolder;
    string pairingsFile;
    
    int totalDepthFrameCount;
    int compressedDepthFrameCount;
    int uncompressedDepthFrameCount;

  protected:

    
};


