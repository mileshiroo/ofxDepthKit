/*
 *  ofxTLVideoDepthAlignmentScrubber.cpp
 *  RGBDPostAlign
 *
 *  Created by James George on 11/16/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxTLVideoDepthAlignmentScrubber.h"
#include "ofxTimeline.h"

ofxTLVideoDepthAlignmentScrubber::ofxTLVideoDepthAlignmentScrubber(){
	videoSequence = NULL;
	depthSequence = NULL;
	selectedPairIndex = -1;
}

ofxTLVideoDepthAlignmentScrubber::~ofxTLVideoDepthAlignmentScrubber(){

}
	
void ofxTLVideoDepthAlignmentScrubber::draw(){
	
	if(!ready()){
		ofPushStyle();
		ofSetColor(255, 100, 0, 30);
		ofRect(bounds);
		ofPopStyle();
		return;
	}
		
	ofPushStyle();
	vector<VideoDepthPair>& alignedFrames = getPairs();
	for(int i = 0; i < alignedFrames.size(); i++){
		int videoMillis;
		if(depthSequence->doFramesHaveTimestamps()){
            float videoPercent = alignedFrames[i].videoMillis / (videoSequence->getPlayer()->getDuration()*1000.0);
			videoMillis = videoPercent * videoSequence->getPlayer()->getTotalNumFrames();
		}
		else{
			videoMillis = alignedFrames[i].videoMillis;
		}
        
		int screenX = screenXForIndex( videoMillis );
		if(i == selectedPairIndex){
			ofSetColor(timeline->getColors().textColor);
		}
		else{
			ofSetColor(timeline->getColors().keyColor);
		}
		
		ofLine(screenX, bounds.y, 
               screenX, bounds.y+bounds.height);
		ofDrawBitmapString("video: " + ofToString(alignedFrames[i].videoMillis), ofPoint(screenX+10, bounds.y+15));
		ofDrawBitmapString("depth: " + ofToString(alignedFrames[i].depthMillis), ofPoint(screenX+10, bounds.y+35));
	}
	
	ofSetColor(0, 125, 255);
	int selectedScreenX = normalizedXtoScreenX(selectedPercent);
	ofLine(selectedScreenX, bounds.y, selectedScreenX, bounds.y+bounds.height);
	ofDrawBitmapString("sel.video: " + ofToString(selectedVideoMillis), ofPoint(selectedScreenX+10, bounds.y+55));
	ofDrawBitmapString("sel.depth: " + ofToString(selectedDepthMillis), ofPoint(selectedScreenX+10, bounds.y+75));
	
	ofPopStyle();
}
//
//void ofxTLVideoDepthAlignmentScrubber::selectPercent(float percent){
//	selectedPercent = percent;
//	selectedVideoMillis = normalizedXtoScreenX(percent, zoomBounds);
//}

void ofxTLVideoDepthAlignmentScrubber::keyPressed(ofKeyEventArgs& args){
	if(args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE ){
		if(selectedPairIndex != -1){
			removeAlignmentPair(selectedPairIndex);
			selectedPairIndex = -1;
		}
	}
}

void ofxTLVideoDepthAlignmentScrubber::mousePressed(ofMouseEventArgs& args, long millis){
	vector<VideoDepthPair>& alignedFrames = getPairs();
	for(int i = 0; i < alignedFrames.size(); i++){
		int videoMillis;
		if(depthSequence->doFramesHaveTimestamps()){
			videoMillis = videoSequence->getPlayer()->getTotalNumFrames() * alignedFrames[i].videoMillis / (videoSequence->getPlayer()->getDuration()*1000.0);
		}
		else{
			videoMillis = alignedFrames[i].videoMillis;
		}
		int screenX = screenXForIndex( videoMillis );
		if(abs(args.x - screenX) < 5){
			selectedPairIndex = i;
			return;;
		}
	}
	
	selectedPairIndex = -1;
}

void ofxTLVideoDepthAlignmentScrubber::mouseMoved(ofMouseEventArgs& args, long millis){
}

void ofxTLVideoDepthAlignmentScrubber::mouseDragged(ofMouseEventArgs& args, long millis){
	if(ready() && bounds.inside(args.x, args.y)){
		selectedPercent = screenXtoNormalizedX(args.x);
		selectedVideoMillis = screenXToMillis(args.x);
		updateSelection();
	}
}

void ofxTLVideoDepthAlignmentScrubber::updateSelection(){

	videoSequence->selectFrame(selectedVideoMillis);
	if(depthSequence->doFramesHaveTimestamps()){
		long selectedVideoTime = 1000*videoSequence->getCurrentTime();
		selectedDepthMillis = depthSequence->frameForTime( pairSequence->getDepthMillisForVideoMillis(selectedVideoTime) );
		depthSequence->selectFrame(selectedDepthMillis);
	}
	else{
		selectedDepthMillis = pairSequence->getDepthMillisForVideoMillis(selectedVideoMillis);
		depthSequence->selectFrame(selectedDepthMillis);
	}
}

void ofxTLVideoDepthAlignmentScrubber::mouseReleased(ofMouseEventArgs& args, long millise){
}

void ofxTLVideoDepthAlignmentScrubber::registerCurrentAlignment(){
	if(depthSequence->doFramesHaveTimestamps()){
        double videoSeconds = videoSequence->getCurrentTime();
        long depthMillis = depthSequence->getSelectedTimeInMillis();
		pairSequence->addAlignedTime(long(1000*videoSeconds), depthMillis);
	}
	else{
		pairSequence->addAlignedFrames(videoSequence->getSelectedFrame(), depthSequence->getSelectedFrame());
	}
	
	save();
}

void ofxTLVideoDepthAlignmentScrubber::removeAlignmentPair(int index){
	
	pairSequence->removeAlignedPair(index);
	save();
}

void ofxTLVideoDepthAlignmentScrubber::save(){
	if(xmlFileName == ""){
		ofLogError("ofxTLVideoDepthAlignmentScrubber -- saving with no save file");
		return;
	}
	pairSequence->savePairingFile(xmlFileName);
}

void ofxTLVideoDepthAlignmentScrubber::load(){
	if(pairSequence == NULL){
		pairSequence = ofPtr<ofxRGBDVideoDepthSequence>( new ofxRGBDVideoDepthSequence() );
	}
	if(xmlFileName == ""){
        pairSequence->reset();
		ofLogError("ofxTLVideoDepthAlignmentScrubber -- loading no save file");
		return;
	}

	pairSequence->loadPairingFile(xmlFileName);
}

vector<VideoDepthPair> & ofxTLVideoDepthAlignmentScrubber::getPairs(){
	return pairSequence->getPairs();
}

void ofxTLVideoDepthAlignmentScrubber::setPairSequence(ofPtr<ofxRGBDVideoDepthSequence> newSequence){
	pairSequence = newSequence;
}

ofPtr<ofxRGBDVideoDepthSequence> ofxTLVideoDepthAlignmentScrubber::getPairSequence(){
	return pairSequence;
}

bool ofxTLVideoDepthAlignmentScrubber::ready(){
	return videoSequence != NULL && depthSequence != NULL && pairSequence->ready();
}


