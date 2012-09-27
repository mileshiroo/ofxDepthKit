//
//  ofxDepthImageSequence.h
//  RGBDVisualize
//
//  Created by James George on 4/16/12.
//

#pragma once

#include "ofMain.h"
#include "ofxDepthImageCompressor.h"

typedef struct {
    string path;
    long timestamp;
    int frameNumber;
} DepthImage;

class ofxDepthImageSequence {
  public:
    ofxDepthImageSequence();
    ~ofxDepthImageSequence();

    bool loadSequence(string sequenceDirectory);
    bool isLoaded();
    bool doFramesHaveTimestamps();

    string getSequenceDirectory();
    
    int getCurrentFrame();
    long getCurrentMilliseconds();
    float getCurrentSeconds();
    
    int frameForTime(long timeInMillis);
    
	void setFrame(int frame);
	void setTimeInSeconds(float timeInSeconds);
	void setTimeInMilliseconds(long timeInMillis);
	
    long getDurationInMillis();
    float getDurationInSeconds();

    void updatePixels();
    
    ofShortPixels& getPixels();
    void getPixelsAtTime(long timeInMillis, ofShortPixels& pixels);
    
    vector<DepthImage>& getImageArray();
    ofxDepthImageCompressor& getCompressor();
    
  protected:
    ofxDepthImageCompressor compressor;
    
    bool sequenceLoaded;
    bool framesHaveTimestamps;
    string sequenceDirectory;
    
    vector<DepthImage> images;
    ofShortPixels pixels;
    int currentPixelsFrame; //represents what's loaded in pixels currently
	int currentFrame; //represents the latest selection
    long durationInMillis;
};