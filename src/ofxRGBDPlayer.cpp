//
//  ofxRGBDPlayer.cpp
//  FeatureTriangulation
//
//  Created by James George on 6/8/12.
//  Copyright (c) 2012 FlightPhase. All rights reserved.
//

#include "ofxRGBDPlayer.h"

ofxRGBDPlayer::ofxRGBDPlayer(){
    loaded = false;
    frameIsNew = false;
    currentlyHiRes = false;
    shift = ofVec2f(0,0);
	updateVideoPlayer = true;
	lastFrame = 0;
}

ofxRGBDPlayer::~ofxRGBDPlayer(){
}

bool ofxRGBDPlayer::setup(string sceneDirectory, bool forceHiRes){
    cout << "setting up scene " << sceneDirectory << endl;
    scene.loadFromFolder(sceneDirectory);
    if(!scene.hasColor || !scene.hasPairings){
        scene.clear();
        ofLogError("Scene at " + sceneDirectory + " is not valid!");
        return (loaded = false);
    }
    return setup(scene, forceHiRes);
}

bool ofxRGBDPlayer::setup(ofxRGBDScene newScene, bool forceHiRes){

    scene = newScene;
    
    if(forceHiRes){
        useHiresVideo();
    }
    else{
	    useLowResVideo();    
    }
	
    if(depthSequence == NULL){
        depthSequence = ofPtr<ofxDepthImageSequence>( new ofxDepthImageSequence() );
    }
    depthSequence->loadSequence(scene.depthFolder);
    videoDepthAligment.loadPairingFile(scene.pairingsFile);
    
    if(scene.hasXYShift){
    	ofxXmlSettings xyshift;
        xyshift.loadFile(scene.xyshiftFile);
        shift = ofVec2f(xyshift.getValue("xshift", 0.),
                        xyshift.getValue("yshift", 0.));
        
        scale = ofVec2f(xyshift.getValue("xscale", 1.0),
                        xyshift.getValue("yscale", 1.0));
    }
    else{
        shift = ofVec2f(0,0);
        scale = ofVec2f(1,1);
    }
    
    player->play();
    player->setSpeed(0);
    
    return (loaded = true);
}

bool ofxRGBDPlayer::hasHighresVideo(){
	return loaded && scene.hasAlternativeHiResVideo;
}
                                      
bool ofxRGBDPlayer::isUsingHighResVideo(){
    return loaded && currentlyHiRes;
}

void ofxRGBDPlayer::useHiresVideo(){
    
    if(hasHighresVideo()){
        ofLogError("ofxRGBDPlayer::useHiresVideo -- no hi res video to load");
        return;        
    }
    if(currentlyHiRes){
        ofLogError("ofxRGBDPlayer::useHiresVideo -- already using hi res video");
        return;        
    }
	
    player = ofPtr<ofVideoPlayer>(new ofVideoPlayer());
    if(!player->loadMovie(scene.alternativeHiResVideoPath)){
        ofLogError("ofxRGBDPlayer::useHiresVideo -- error loading hi res video, returning to low res");
        useLowResVideo();
        return;
    }
    currentlyHiRes = true;
}

void ofxRGBDPlayer::useLowResVideo(){
    player = ofPtr<ofVideoPlayer>(new ofVideoPlayer());
    if(!player->loadMovie(scene.videoPath)){
        scene.clear();
        ofLogError("Movie failed to load");
        loaded = false;
    }    
    currentlyHiRes = false;
}

ofVec2f ofxRGBDPlayer::getXYShift(){
    return shift;
}
                                                 
void ofxRGBDPlayer::update(){
	if(!loaded) return;
    
	if(updateVideoPlayer){
	    player->update();
	}
	
	int thisFrame = player->getCurrentFrame();
	if(thisFrame != lastFrame){
        long videoTime = player->getPosition()*player->getDuration()*1000;
        depthSequence->selectTime(videoDepthAligment.getDepthFrameForVideoFrame(videoTime));
        depthSequence->updatePixels();            
        frameIsNew = true;
		lastFrame = thisFrame;
    }
}

bool ofxRGBDPlayer::isLoaded(){
    return loaded;
}

bool ofxRGBDPlayer::isFrameNew(){
    bool isNew = frameIsNew;
    frameIsNew = false;
    return isNew;
}

void ofxRGBDPlayer::play(){
    if(!loaded) return;
    player->setSpeed(1.0);
}

void ofxRGBDPlayer::stop(){
    if(!loaded) return;
    player->setSpeed(0.0);
}
                           
void ofxRGBDPlayer::togglePlay(){
    
    if(!loaded) return;
    
    if(player->getSpeed() == 0.0){
        play();
    }
    else{
        stop();
    }
}

int ofxRGBDPlayer::getDurationInFrames(){
    if(loaded){
        return player->getTotalNumFrames();
    }
    return 0;
}

float ofxRGBDPlayer::getDurationInSeconds(){
    if(loaded){
        return player->getDuration(); 
    }
    return 0;
}

ofShortPixels& ofxRGBDPlayer::getDepthPixels(){
	return depthSequence->getPixels();    
}

ofPtr<ofVideoPlayer> ofxRGBDPlayer::getVideoPlayer(){
	return player;
}

ofPtr<ofxDepthImageSequence> ofxRGBDPlayer::getDepthSequence(){
	return depthSequence;    
}

ofxRGBDScene& ofxRGBDPlayer::getScene(){
	return scene;    
}

ofxRGBDVideoDepthSequence& ofxRGBDPlayer::getVideoDepthAligment(){
    return videoDepthAligment;
}