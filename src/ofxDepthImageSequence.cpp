//
//  ofxDepthImageSequence.cpp
//  RGBDVisualize
//
//  Created by James George on 4/16/12.
//  Copyright (c) 2012 FlightPhase. All rights reserved.
//

#include "ofxDepthImageSequence.h"

ofxDepthImageSequence::ofxDepthImageSequence(){
    sequenceLoaded = false;
    framesHaveTimestamps = false;
	currentFrame = 0;
	currentPixelsFrame = 0;
}

ofxDepthImageSequence::~ofxDepthImageSequence(){
    
}

bool ofxDepthImageSequence::loadSequence(string newSequenceDirectory){
    

	ofDirectory sequenceList(newSequenceDirectory);
	if(!sequenceList.exists()){
		ofLogError("ofxDepthImageSequence -- sequence directory " + newSequenceDirectory + " does not exist!");
		return false;
	}
    
	if(sequenceLoaded){
		images.clear();
		sequenceLoaded = false;
	}


	sequenceList.allowExt("png");
	int numFiles = sequenceList.listDir();
	if(numFiles == 0){
		ofLogError("ofxTLDepthImageSequence -- sequence directory " + newSequenceDirectory + " is empty!");
		return false;
	}
	

	bool checkedForTimestamp = false;
	unsigned long firstFrameTimeOffset = 0;
	for(int i = 0; i < numFiles; i++){
        //backwards compat...
		if(sequenceList.getName(i).find("poster") != string::npos){
			ofLogWarning("discarding poster frame " + sequenceList.getPath(i) );
			continue;
		}
		
		if(!checkedForTimestamp){
			framesHaveTimestamps = sequenceList.getName(i).find("millis") != string::npos;
			checkedForTimestamp = true;
			ofLogVerbose("Frames have timestamps? " + string((framesHaveTimestamps ? "yes!" : "no :(")) );
		}
		
		
		images.push_back( DepthImage() );
		DepthImage& img = images[images.size()-1];
		img.path = sequenceList.getPath(i);
		
		if(framesHaveTimestamps){
			vector<string> split = ofSplitString(sequenceList.getName(i), "_", true, true);
			for(int l = 0; l < split.size(); l++){
				if(split[l] == "millis"){
					img.timestamp = ofToInt(split[l+1]);
					if(i == 0){
						firstFrameTimeOffset = img.timestamp;
					}
					img.timestamp -= firstFrameTimeOffset;
				}
			}
		}

		images.push_back( img );
	}
	
	//currentFrame = -1;
    if(framesHaveTimestamps){
	    durationInMillis = images[images.size()-1].timestamp;
    }

	ofLogVerbose("sequence is loaded " + ofToString( images.size() ));
    sequenceDirectory = newSequenceDirectory;
    sequenceLoaded = true;
	setFrame(0);
    updatePixels();
//	startThread();
	return true;
}

string ofxDepthImageSequence::getSequenceDirectory(){
    return sequenceDirectory;
}

long ofxDepthImageSequence::getDurationInMillis(){
    return durationInMillis;
}

float ofxDepthImageSequence::getDurationInSeconds(){
    return durationInMillis / 1000.0;
}

int ofxDepthImageSequence::getTotalNumFrames(){
	return images.size();
}

int ofxDepthImageSequence::getCurrentFrame(){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::getCurrentFrame() -- sequence not loaded");
        return 0;
    }
    return currentFrame;
}

long ofxDepthImageSequence::getCurrentMilliseconds(){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::getCurrentMilliseconds() -- sequence not loaded");
        return 0;
    }
    return images[currentFrame].timestamp;
}

float ofxDepthImageSequence::getCurrentSeconds(){
	return getCurrentMilliseconds()/1000.0;
}

int ofxDepthImageSequence::frameForTime(long timeInMillis){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::frameForTime() -- sequence not loaded");
        return 0;
    }
    
	if(!framesHaveTimestamps){
		ofLogError("ofxTLDepthImageSequence -- can't select frame for time if there are no timestamps");
		return 0;
	}
	
	//TODO: switch to std::lower_bound since data is sorted
	for(int i = 1; i < images.size(); i++){
		if(images[i].timestamp > timeInMillis){
			return i-1;
		}
	}
    
	return images.size()-1;   
}

void ofxDepthImageSequence::setFrame(int frame){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::selectFrame() -- sequence not loaded");
        return;
    }
    
    if(frame < 0 || frame >= images.size()){
        ofLogError("ofxDepthImageSequence::selectFrame() -- frame out of range");
        return;
    }
    
    currentFrame = frame;
	updatePixels();
}

void ofxDepthImageSequence::setTimeInSeconds(float timeInSeconds){
	setTimeInMilliseconds(timeInSeconds*1000);
}

void ofxDepthImageSequence::setTimeInMilliseconds(long timeInMillis){
    
    if(!framesHaveTimestamps){
        ofLogError("ofxDepthImageSequence::selectTime() -- no timestamps!");
        return;        
    }    
    if(timeInMillis < 0 || timeInMillis > durationInMillis){
        ofLogError("ofxDepthImageSequence::selectTime() -- time out of range!");
        return;
    }    
    
    setFrame(frameForTime(timeInMillis));
}

void ofxDepthImageSequence::updatePixels(){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::updatePixels() -- sequence not loaded");
    }
	
	if(currentFrame != currentPixelsFrame || !pixels.isAllocated()){

    	ofxDepthImageCompressor::readCompressedPng(images[currentFrame].path, pixels);
		currentPixelsFrame = currentFrame;
	}
}

ofShortPixels& ofxDepthImageSequence::getPixels(){
    return pixels;
}

void ofxDepthImageSequence::getPixelsAtTime(long timeInMillis, ofShortPixels& outPixels){
    ofxDepthImageCompressor::readCompressedPng(images[frameForTime(timeInMillis)].path, outPixels);
}

vector<DepthImage>& ofxDepthImageSequence::getImageArray(){
    return images;
}

ofxDepthImageCompressor& ofxDepthImageSequence::getCompressor(){
	return compressor;    
}

bool ofxDepthImageSequence::isLoaded(){
    return sequenceLoaded;
}

bool ofxDepthImageSequence::doFramesHaveTimestamps(){
	return framesHaveTimestamps;
}

