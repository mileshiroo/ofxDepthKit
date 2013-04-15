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
	frameIsNew = false;
	timeOffsetInMillis = 0;
	autoUpdate = true;
	depthImageIsDirty = false;
}

ofxTLDepthImageSequence::~ofxTLDepthImageSequence(){
}

void ofxTLDepthImageSequence::setup(){
	ofxTLImageTrack::setup();
	ofAddListener(ofEvents().exit, this, &ofxTLDepthImageSequence::exit);
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
	if(isLoaded() && getAutoUpdate()){
		selectTimeInMillis( timeline->getCurrentTimeMillis() );
	}
}

void ofxTLDepthImageSequence::draw(){
	if(!isLoaded()){
		ofPushStyle();
		ofSetColor(timeline->getColors().disabledColor);
		ofRect(getDrawRect());
		ofPopStyle();
		return;
	}
	
	//clip hanging frames off the sides
	glEnable(GL_SCISSOR_TEST);
	glScissor(bounds.x, 0, bounds.width, ofGetHeight());
	
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
				
				ofNoFill();
				ofSetColor(timeline->getColors().textColor);
				timeline->getFont().drawString(ofToString(videoThumbs[i].framenum), videoThumbs[i].displayRect.x+5, videoThumbs[i].displayRect.y+15);
				ofRect(videoThumbs[i].displayRect);
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

	ofPushStyle();
	string frameString = "F# " + ofToString(depthImageSequence->getCurrentFrame());
	string timecodeString = ofxTimecode::timecodeForMillis(timeOffsetInMillis+depthImageSequence->getCurrentMilliseconds());
	int selectedFrameX = ofClamp(timeline->millisToScreenX(getSelectedTimeInMillis()), bounds.getMinX(), bounds.getMaxX());
	ofSetColor(timeline->getColors().backgroundColor, 175);
	ofRect(selectedFrameX, bounds.y, timeline->getFont().getStringBoundingBox("00:00:00:000",0,0).width+20, bounds.height);
	
	ofSetColor(timeline->getColors().textColor);
	ofLine(selectedFrameX, bounds.y, selectedFrameX, bounds.y+bounds.height);

	timeline->getFont().drawString(timecodeString, selectedFrameX + 10,  bounds.y+timeline->getFont().getLineHeight()+3);
	timeline->getFont().drawString(frameString, selectedFrameX + 10, bounds.y+(timeline->getFont().getLineHeight()+3)*2);

	glDisable(GL_SCISSOR_TEST);
	
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

}

bool ofxTLDepthImageSequence::mousePressed(ofMouseEventArgs& args, long millis){
	if(isLoaded() && isActive() ){
		selectTimeInMillis(millis);
		return true;
	}
	return false;
}

void ofxTLDepthImageSequence::mouseDragged(ofMouseEventArgs& args, long millis){
	
	if(!isLoaded()) return;
	
	if( isActive() ){
		selectTimeInMillis(millis);
		if(timeline->getMovePlayheadOnDrag()){
			timeline->setCurrentTimeMillis(millis);
		}
		timeline->flagUserChangedValue();
	}
}

void ofxTLDepthImageSequence::mouseReleased(ofMouseEventArgs& args, long millis){
	//default behavior for now
	ofxTLImageTrack::mouseReleased(args, millis);
}

void ofxTLDepthImageSequence::keyPressed(ofKeyEventArgs& args){
	if(hasFocus()){
		if(args.key == OF_KEY_LEFT){
			selectFrame(MAX(getSelectedFrame()-1, 0));
		}
		else if(args.key == OF_KEY_RIGHT){
			selectFrame(MIN(getSelectedFrame()+1, depthImageSequence->getImageArray().size()-1));
		}
	}
}

void ofxTLDepthImageSequence::setAutoUpdate(bool doAuto){
	autoUpdate = doAuto;
}

bool ofxTLDepthImageSequence::getAutoUpdate(){
	return autoUpdate;
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
	
	if(timeStampInMillis != depthImageSequence->getCurrentMilliseconds()){
		int lastFrame = depthImageSequence->getCurrentFrame();
		depthImageSequence->setTimeInMilliseconds( ofClamp(timeStampInMillis+timeOffsetInMillis, 0, depthImageSequence->getDurationInMillis()) );
		frameIsNew |= (lastFrame != depthImageSequence->getCurrentFrame());
		depthImageIsDirty |= frameIsNew;
	}
}

void ofxTLDepthImageSequence::selectFrame(int frame){
	if(!isLoaded()) return;
	
	if(frame != depthImageSequence->getCurrentFrame()){
		depthImageSequence->setFrame(frame);
		frameIsNew = true;
		depthImageIsDirty = true;
	}
}

void ofxTLDepthImageSequence::selectTimeInSeconds(float timeInSeconds){
	selectTimeInMillis(timeInSeconds*1000);
}

unsigned long ofxTLDepthImageSequence::getSelectedTimeInMillis(){
	if(!isLoaded()) return 0;
	return depthImageSequence->getCurrentMilliseconds();
}

int ofxTLDepthImageSequence::frameForTime(long timeInMillis){
	if(!isLoaded()) return 0;
	return depthImageSequence->frameForTime(timeInMillis);
}

ofImage& ofxTLDepthImageSequence::getCurrentDepthImage(){
	if(depthImageIsDirty){
		currentDepthImage = depthImageSequence->getCompressor().convertTo8BitImage(depthImageSequence->getPixels());
		depthImageIsDirty = false;
	}
	return currentDepthImage;
}

unsigned long ofxTLDepthImageSequence::getDurationInMillis(){
	if(!isLoaded()) return 0;
	return depthImageSequence->getDurationInMillis();
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
	bool isnew = frameIsNew;
	frameIsNew = false;
	return isnew;
}

ofPtr<ofxDepthImageSequence> ofxTLDepthImageSequence::getDepthImageSequence(){
	return depthImageSequence;
}

bool ofxTLDepthImageSequence::loadSequence(string seqdir){
	waitForThread(true);
	
	if(depthImageSequence == NULL){
		depthImageSequence = ofPtr<ofxDepthImageSequence>( new ofxDepthImageSequence() );
	}

	bool loadSuccess = depthImageSequence->loadSequence(seqdir);
	if(loadSuccess){
		frameIsNew = true;
		depthImageIsDirty = true;
		startThread();
	}
	return loadSuccess;
}

void ofxTLDepthImageSequence::setSequence(ofxDepthImageSequence& newSequence){
	setSequence(ofPtr<ofxDepthImageSequence>(&newSequence));
}

void ofxTLDepthImageSequence::setSequence(ofPtr<ofxDepthImageSequence> newSequence){
	waitForThread(true);
 	depthImageSequence = newSequence;
	startThread();
}

void ofxTLDepthImageSequence::threadedFunction(){
	while(isThreadRunning()){
		
		backLock.lock();
		ofImage thumbImage;
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
					depthImageSequence->getPixelsAtTime(ofClamp(videoThumbs[i].timestamp+timeOffsetInMillis,
																0, depthImageSequence->getDurationInMillis()), thumbnailDepthRaw);
					thumbImage = depthImageSequence->getCompressor().convertTo8BitImage(thumbnailDepthRaw, false);
					backThumbs[i].create(thumbImage.getPixelsRef());
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


void ofxTLDepthImageSequence::setTimeOffsetToPlayhead(){
	setTimeOffsetInMillis(depthImageSequence->getCurrentMilliseconds() - timeline->getCurrentTimeMillis());
}

void ofxTLDepthImageSequence::setTimeOffsetInMillis(unsigned long millis){
	timeOffsetInMillis = millis;
}

unsigned long ofxTLDepthImageSequence::getTimeOffsetInMillis(){
	return timeOffsetInMillis;
}

bool ofxTLDepthImageSequence::doFramesHaveTimestamps(){
	return depthImageSequence->doFramesHaveTimestamps();
}

void ofxTLDepthImageSequence::exit(ofEventArgs& args){
	waitForThread(true);
}


