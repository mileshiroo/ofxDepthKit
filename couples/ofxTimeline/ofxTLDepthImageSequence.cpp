/*
 *  ofxTLDepthImageSequence.cpp
 *  timelineExampleVideoPlayer
 *
 *  Created by James George on 11/14/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxTLDepthImageSequence.h"
#include "ofxTimeline.h"

ofxTLDepthImageSequence::ofxTLDepthImageSequence(){
//	sequenceLoaded = false;
//	currentDepthRaw = NULL;
//	thumbnailDepthRaw = NULL;
//	selectedFrame = 0;


//    thumbnailUpdatedZoomLevel = -1;
//    thumbnailUpdatedWidth = -1;
//    thumbnailUpdatedHeight = -1;
//    currentlyZooming = false;

}

ofxTLDepthImageSequence::~ofxTLDepthImageSequence(){
}

void ofxTLDepthImageSequence::setup(){
	
	ofxTLImageTrack::setup();
    
	ofAddListener(ofEvents().exit, this, &ofxTLDepthImageSequence::exit);
    startThread();
    
}

void ofxTLDepthImageSequence::enable(){
	if(!enabled){
		ofxTLTrack::enable();
		timeline->events().registerPlaybackEvents(this);
	}
}

void ofxTLDepthImageSequence::disable(){
	if(enabled){
		ofxTLTrack::disable();
		timeline->events().removePlaybackEvents(this);
	}
}

void ofxTLDepthImageSequence::update(ofEventArgs& args){
    if(isLoaded()){
    	return;
    }
    
	selectTimeInMillis( timeline->getCurrentTime()*1000 );

}

void ofxTLDepthImageSequence::draw(){


    if(!isLoaded()){
		ofPushStyle();
        ofSetColor(timeline->getColors().disabledColor);
        ofRect(getDrawRect());
        ofPopStyle();
        return;
    }
	
//    if(thumbsEnabled && !ofGetMousePressed() && 
//       (thumbnailUpdatedZoomLevel != zoomBounds ||
//        thumbnailUpdatedWidth != getDrawRect().width ||
//        thumbnailUpdatedHeight != getDrawRect().height) ) {
//           generateVideoThumbnails();	
//    }
	if(thumbsEnabled && getDrawRect().height > 10){
		ofPushStyle();
		ofSetColor(255);
		lock();
		for(int i = 0; i < videoThumbs.size(); i++){
			if(videoThumbs[i].thumb != NULL){
				if(videoThumbs[i].loaded &&!videoThumbs[i].thumb->isUsingTexture()){
					videoThumbs[i].thumb->setUseTexture(true);
					videoThumbs[i].thumb->update();
				}
				
				videoThumbs[i].thumb->draw(videoThumbs[i].displayRect);
			}
			else{
				ofPushStyle();
				ofSetColor(0);
				ofRect(videoThumbs[i].displayRect);
				ofPopStyle();
			}
		}
		unlock();
		ofPopStyle();
	}

//
	
//	if(thumbsEnabled){
//		ofSetColor(255);
//		for(int i = 0; i < videoThumbs.size(); i++){
//			if(videoThumbs[i].thumb != NULL){
//				videoThumbs[i].thumb->draw(videoThumbs[i].displayRect);
//			}
//		}
//	}
	
//	for(int i = 0; i < videoThumbs.size(); i++){
//		if(videoThumbs[i].thumb != NULL){
//			if(!thumbsEnabled){
//				ofFill();
//				ofSetColor(0);
//				ofRect(videoThumbs[i].displayRect);
//			}
//			ofNoFill();
//			ofSetColor(255, 150, 0);
//			ofDrawBitmapString(ofToString(videoThumbs[i].framenum), videoThumbs[i].displayRect.x+5, videoThumbs[i].displayRect.y+15);
//			ofRect(videoThumbs[i].displayRect);
//		}
//	}
	
	ofPushStyle();
	int selectedFrameX = timeline->millisToScreenX(getSelectedTimeInMillis());
	ofSetColor(0, 125, 255);
	ofLine(selectedFrameX, bounds.y, selectedFrameX, bounds.y+bounds.height);
	ofDrawBitmapString("frame " + ofToString(depthImageSequence->getCurrentFrame()), selectedFrameX, bounds.y+30);	
	ofPopStyle();
}

//width and height of image elements
float ofxTLDepthImageSequence::getContentWidth(){
    return 640;
}

float ofxTLDepthImageSequence::getContentHeight(){
    return 480;
}

void ofxTLDepthImageSequence::framePositionsUpdated(vector<ofxTLVideoThumb>& newThumbs){

	
    for(int i = 0; i < videoThumbs.size(); i++){
        for(int j = 0; j < newThumbs.size(); j++){
            if(videoThumbs[i].framenum == newThumbs[j].framenum && videoThumbs[i].loaded){
                newThumbs[j].thumb = videoThumbs[i].thumb;
                newThumbs[j].loaded = true;
                break;
            }
        }
    }
    
	backLock.lock();
    backThumbs = newThumbs;
	backLock.unlock();
    
	lock();
    videoThumbs = newThumbs;
    unlock();

	//    videoThumbs = newThumbs;
//    generateVideoThumbnails();
}

void ofxTLDepthImageSequence::mouseDragged(ofMouseEventArgs& args, long millis){
    
    if(!isLoaded()) return;
    
	if( bounds.inside(args.x, args.y) ){
		//int index = indexForScreenX(args.x, videoThumbs.size());
		selectTimeInMillis(millis);
		if(timeline->getMovePlayheadOnDrag()){
			timeline->setCurrentTimeMillis(millis);
		}
        timeline->flagUserChangedValue();
	}
}

void ofxTLDepthImageSequence::keyPressed(ofKeyEventArgs& args){
	if(hover){
		if(args.key == OF_KEY_LEFT){
			selectFrame(MAX(getSelectedFrame()-1, 0));
		}
		else if(args.key == OF_KEY_RIGHT){
			selectFrame(MIN(getSelectedFrame()+1, depthImageSequence->getImageArray().size()-1));
		}
	}
}

void ofxTLDepthImageSequence::playbackStarted(ofxTLPlaybackEventArgs& args){
	ofAddListener(ofEvents().update, this, &ofxTLDepthImageSequence::update);
}

void ofxTLDepthImageSequence::playbackEnded(ofxTLPlaybackEventArgs& args){
	ofRemoveListener(ofEvents().update, this, &ofxTLDepthImageSequence::update);
}

void ofxTLDepthImageSequence::playbackLooped(ofxTLPlaybackEventArgs& args){
}

void ofxTLDepthImageSequence::selectTimeInMillis(long timeStampInMillis){
    if(!isLoaded()) return;
	
    depthImageSequence->selectTimeInMillis(timeStampInMillis);
	currentDepthImage = depthImageSequence->getCompressor().convertTo8BitImage(depthImageSequence->getPixels());
	frameIsNew = true;
}

void ofxTLDepthImageSequence::selectFrame(int frame){
	if(!isLoaded()) return;
	
	depthImageSequence->selectFrame(frame);
	currentDepthImage = depthImageSequence->getCompressor().convertTo8BitImage(depthImageSequence->getPixels());
	frameIsNew = true;
}


void ofxTLDepthImageSequence::selectTimeInSeconds(float timeInSeconds){
	selectTimeInMillis(timeInSeconds*1000);
}

long ofxTLDepthImageSequence::getSelectedTimeInMillis(){
    if(!isLoaded()) return 0;
    return depthImageSequence->getCurrentMilliseconds();
}

int ofxTLDepthImageSequence::frameForTime(long timeInMillis){
    if(!isLoaded()) return 0;
    return depthImageSequence->frameForTime(timeInMillis);
}

void ofxTLDepthImageSequence::mouseReleased(ofMouseEventArgs& args, long millis){
}


bool ofxTLDepthImageSequence::loadSequence(){
	ofFileDialogResult r = ofSystemLoadDialog("Load Depth Sequence Directory", true);
	if(r.bSuccess){
		return loadSequence(r.getPath());
	}
	return false;
}


bool ofxTLDepthImageSequence::isLoaded(){
	return depthImageSequence != NULL && depthImageSequence->isLoaded();
}

bool ofxTLDepthImageSequence::isFrameNew(){
    bool result = frameIsNew;
    frameIsNew = false;
    return result;
}

ofPtr<ofxDepthImageSequence> ofxTLDepthImageSequence::getDepthImageSequence(){
	return depthImageSequence;
}

bool ofxTLDepthImageSequence::loadSequence(string seqdir){
    
    if(depthImageSequence == NULL){
        setSequence(ofPtr<ofxDepthImageSequence>(new ofxDepthImageSequence()));
    }
    
    return depthImageSequence->loadSequence(seqdir);
}

void ofxTLDepthImageSequence::setSequence(ofxDepthImageSequence& newSequence){
    setSequence(ofPtr<ofxDepthImageSequence>(&newSequence));
}

void ofxTLDepthImageSequence::setSequence(ofPtr<ofxDepthImageSequence> newSequence){
 	depthImageSequence = newSequence;
}

//void ofxTLDepthImageSequence::generateVideoThumbnails() {
//    
//    if(!isLoaded() ||  currentlyZooming || !thumbsEnabled) return;
//    
//	
//	cout << "** generating thumbs for depth" << endl;
//    thumbnailUpdatedZoomLevel = zoomBounds;
//    thumbnailUpdatedWidth = getDrawRect().width;
//	thumbnailUpdatedHeight = getDrawRect().height; 
////    cout << "generating thumbnails for " << videoThumbs.size() << endl;
//	for(int i = 0; i < videoThumbs.size(); i++){
////        cout << "	generating thumb for frame " << videoThumbs[i].framenum << " time " << videoThumbs[i].timestamp/1000.0 << endl;
//        depthImageSequence->getPixelsAtTime(videoThumbs[i].timestamp, thumbnailDepthRaw);
//        videoThumbs[i].create(depthImageSequence->getCompressor().convertTo8BitImage(thumbnailDepthRaw));
//	}
//}

void ofxTLDepthImageSequence::threadedFunction(){
	while(isThreadRunning()){
		
        backLock.lock();
        if(!ofGetMousePressed() && isLoaded() && !currentlyZooming && thumbsEnabled){
            for(int i = 0; i < backThumbs.size(); i++){
                if(!backThumbs[i].loaded){
                    
					if(currentlyZooming || ofGetMousePressed()){
                        break;
                    }
					
                    if(currentlyZooming || ofGetMousePressed()){
                        break;
                    }
					
					backThumbs[i].useTexture = false;
					depthImageSequence->getPixelsAtTime(videoThumbs[i].timestamp, thumbnailDepthRaw);
					backThumbs[i].create(depthImageSequence->getCompressor().convertTo8BitImage(thumbnailDepthRaw, false));
                    
                    lock();
                    videoThumbs[i] = backThumbs[i];
                    unlock();
					
                }
            }
        }
		
        backLock.unlock();
		
        ofSleepMillis(10);
    }
	
}

void ofxTLDepthImageSequence::toggleThumbs(){
	thumbsEnabled = !thumbsEnabled;
}

int ofxTLDepthImageSequence::getSelectedFrame(){
    if(!isLoaded()) return 0;
	return depthImageSequence->getCurrentFrame();
}

bool ofxTLDepthImageSequence::doFramesHaveTimestamps(){
	return depthImageSequence->doFramesHaveTimestamps();
}

void ofxTLDepthImageSequence::exit(ofEventArgs& args){
//	waitForThread(true);
}


