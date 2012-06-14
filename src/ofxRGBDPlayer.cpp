//
//  ofxRGBDPlayer.cpp
//  FeatureTriangulation
//
//  Created by James George on 6/8/12.
//  Copyright (c) 2012 FlightPhase. All rights reserved.
//

#include "ofxRGBDPlayer.h"

ofxRGBDPlayer::ofxRGBDPlayer(){
	player = NULL;    
    loaded = false;
    frameIsNew = false;
    shift = ofVec2f(0,0);
}

ofxRGBDPlayer::~ofxRGBDPlayer(){
    if(player != NULL){
        delete player;
    }
}

bool ofxRGBDPlayer::setup(string sceneDirectory){
    cout << "setting up scene " << sceneDirectory << endl;
    scene.loadFromFolder(sceneDirectory);
    if(!scene.hasColor || !scene.hasPairings){
        scene.clear();
        ofLogError("Scene at " + sceneDirectory + " is not valid!");
        return (loaded = false);
    }
    return setup(scene);
}

bool ofxRGBDPlayer::setup(ofxRGBDScene scene){

    if(player != NULL){
        delete player;
    }
    
    player = new ofVideoPlayer();
    if(!player->loadMovie(scene.videoPath)){
        scene.clear();
        ofLogError("Movie failed to load");
        return (loaded = false);
    }
    
    depthSequence.loadSequence(scene.depthFolder);
    videoDepthAligment.loadPairingFile(scene.pairingsFile);
    
//    renderer.setup(scene.calibrationFolder);
//    renderer.setRGBTexture(*player);
//    renderer.setDepthImage(depthSequence.getPixels());
//    renderer.setSimplification(2); //default simplification
    
    if(scene.hasXYShift){
    	ofxXmlSettings xyshift;
        xyshift.loadFile(scene.xyshiftFile);
        shift = ofVec2f(xyshift.getValue("xshift", 0.),
                        xyshift.getValue("yshift", 0.));
    }
    else{
        shift = ofVec2f(0,0);
    }
    
    player->play();
    player->setSpeed(0);
    
    return (loaded = true);
}

ofVec2f ofxRGBDPlayer::getXYShift(){
    return shift;
}
                                                 
void ofxRGBDPlayer::update(){
	if(!loaded) return;
    
    player->update();
    if(player->isFrameNew()){
        long videoTime = player->getPosition()*player->getDuration()*1000;
        depthSequence.selectTime(videoDepthAligment.getDepthFrameForVideoFrame(videoTime));
        depthSequence.updatePixels();            
        frameIsNew = true;
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
	return depthSequence.getPixels();    
}

ofVideoPlayer& ofxRGBDPlayer::getVideoPlayer(){
	return *player;
}

//ofxRGBDRenderer& ofxRGBDPlayer::getRenderer(){
//	return renderer;    
//}

ofxRGBDScene& ofxRGBDPlayer::getScene(){
	return scene;    
}
