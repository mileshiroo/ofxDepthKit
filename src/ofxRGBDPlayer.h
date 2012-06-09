//
//  ofxRGBDPlayer.h
//  FeatureTriangulation
//
//  Created by James George on 6/8/12.
//  Copyright (c) 2012 FlightPhase. All rights reserved.
//

#pragma once 

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxRGBDAlignment.h"
#include "ofxRGBDScene.h"
#include "ofxRGBDRenderer.h"
#include "ofxRGBDVideoDepthSequence.h"
#include "ofxDepthImageSequence.h"

class ofxRGBDPlayer {
  public:  
    ofxRGBDPlayer();
    ~ofxRGBDPlayer();
    
    bool setup(string sceneDirectory);
    bool setup(ofxRGBDScene take);
    void update();
    void drawWireframe();
    
    bool isLoaded();
    
    void play();
    void stop();
    void togglePlay();
    
    int getDurationInFrames();
    float getDurationInSeconds();
    
    ofShortPixels& getCurrentDepthPixels();
    ofVideoPlayer& getVideoPlayer();
    ofxRGBDRenderer& getRenderer();
    ofxRGBDScene& getScene();
    
  protected:
    bool loaded;
    ofVideoPlayer* player;
    ofxRGBDRenderer renderer;
	ofxRGBDScene scene;
    ofxDepthImageSequence depthSequence;
    ofxRGBDVideoDepthSequence videoDepthAligment;

};