/**
 * ofxRGBDPlayer 
 
 * Class to play an RGBDScene -- keeps the video and depth in sync based on a pairings file
 * set by the visualizer.
 *
 *
 * Most commonly used with the ofxRGBDRenderer or ofxRGBDMeshBuilder. An example use looks like this:
 *
 * ofxRGBDPlayer player;
 * player.setup("/path/to/my/MediaBin/scene1/"); //this is one folder inside of a MediaBin, called a scene
 * renderer.setup(player.getScene().calibrationFolder);
 * renderer.setRGBTexture(player.getVideoPlayer());
 * renderer.setDepthImage(player.getCurrentDepthPixels.getPixels());
 * 
 */

#pragma once 

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxRGBDAlignment.h"
#include "ofxRGBDScene.h"
#include "ofxRGBDVideoDepthSequence.h"
#include "ofxDepthImageSequence.h"

class ofxRGBDPlayer {
  public:  
    ofxRGBDPlayer();
    ~ofxRGBDPlayer();
    
    bool setup(string sceneDirectory);
    bool setup(ofxRGBDScene scene);
    void update();
    
    bool isLoaded();
    bool isFrameNew();
    
    void play();
    void stop();
    void togglePlay();
    
    ofVec2f getXYShift();
    
    int getDurationInFrames();
    float getDurationInSeconds();
    
    ofxRGBDScene& getScene();
    ofShortPixels& getDepthPixels();
    ofVideoPlayer& getVideoPlayer();
    
  protected:
    bool loaded;
    bool frameIsNew;
    ofVec2f shift;
    ofVideoPlayer* player;
	ofxRGBDScene scene;
    ofxDepthImageSequence depthSequence;
    ofxRGBDVideoDepthSequence videoDepthAligment;
    
};