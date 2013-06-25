/**
 * ofxRGBDPlayer 
 *
 * Class to play an RGBDScene -- keeps the video and depth in sync based on a pairings file
 * set by the visualizer.
 *
 *
 * Most commonly used with the ofxRGBDRenderer or ofxRGBDMeshBuilder. An example use looks like this:
 *
 * in setup:
 * player.setup("/path/to/my/MediaBin/scene1/"); //this is one folder inside of a MediaBin, called a scene
 * renderer.setup(player.getScene().calibrationFolder);
 * renderer.setRGBTexture(player.getVideoPlayer());
 * renderer.setDepthImage(player.getDepthPixels());
 * 
 * in update:
 * player.update();
 * if(player.isFrameNew()){
 *   renderer.update();
 * }
 */

#pragma once 

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxRGBDScene.h"
#include "ofxRGBDVideoDepthSequence.h"
#include "ofxDepthImageSequence.h"

class ofxRGBDPlayer {
  public:  
	ofxRGBDPlayer();
	~ofxRGBDPlayer();
	
	bool setup(string sceneDirectory, bool forceHires = false);
	bool setup(ofxRGBDScene& scene, bool forceHires = false);
	
	bool setUseTexture(bool useTexture);
	
	//allows easy swapping of video files from other directories
	//we use this for swapping in color corrected files when doing batch exports
	void setAlternativeVideoFolder(string directory);
	
	void update();
	
	bool isLoaded();
	bool isFrameNew();
	
	void play();
	void stop();
	void togglePlay();
		
	int getDurationInFrames();
	float getDurationInSeconds();
	
	ofxRGBDScene& getScene();
	ofShortPixels& getDepthPixels();
	ofPtr<ofVideoPlayer> getVideoPlayer();
	ofPtr<ofxDepthImageSequence> getDepthSequence();
	ofPtr<ofxRGBDVideoDepthSequence> getVideoDepthAligment();
	
	bool hasHighresVideo();
	bool isUsingHighResVideo();
	void useHiresVideo();
	void useLowResVideo();
	
	bool updateVideoPlayer;

  protected:
	bool loaded;
	bool frameIsNew;
	bool currentlyHiRes;
	int lastFrame;
	bool bUseTexture;
	
	void useAlternativeVideo();
	
	string alternativeVideoFolder;
	
	ofxRGBDScene scene;
	ofPtr<ofVideoPlayer> player;
	ofPtr<ofxDepthImageSequence> depthSequence;
	ofPtr<ofxRGBDVideoDepthSequence> videoDepthAligment;
	
};