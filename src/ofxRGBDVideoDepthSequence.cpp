/*
 *  ofxRGBDVideoDepthSequence.cpp
 *  RGBDVisualize
 *
 *  Created by James George on 11/18/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxRGBDVideoDepthSequence.h"
#include "ofxXmlSettings.h"

bool pairsort(VideoDepthPair frameA, VideoDepthPair frameB){
	return frameA.videoMillis < frameB.videoMillis;
}

ofxRGBDVideoDepthSequence::ofxRGBDVideoDepthSequence(){
}

ofxRGBDVideoDepthSequence::~ofxRGBDVideoDepthSequence(){
}

void ofxRGBDVideoDepthSequence::savePairingFile(string pairFileXml){
	ofxXmlSettings settings;
	for(int i = 0; i < alignedFrames.size(); i++){
		settings.addTag("pair");
		settings.pushTag("pair", i);
		
		if(alignedFrames[i].isTimeBased){
			settings.addValue("videoMillis", (int)alignedFrames[i].videoMillis);
			settings.addValue("depthMillis", (int)alignedFrames[i].depthMillis);
		}
		else{
			settings.addValue("video", (int)alignedFrames[i].videoMillis);
			settings.addValue("depth", (int)alignedFrames[i].depthMillis);
		}
		settings.popTag();
	}
	settings.saveFile(pairFileXml);	
}

bool ofxRGBDVideoDepthSequence::loadPairingFile(string pairFileXml){
	
    alignedFrames.clear();//AP: Fixed bug loading new comp! 
	ofxXmlSettings settings;
	if(settings.loadFile(pairFileXml)){
		int numPairs = settings.getNumTags("pair");
		for(int i = 0; i < numPairs; i++){
			settings.pushTag("pair", i);
			VideoDepthPair p;
			p.isTimeBased = settings.getNumTags("videoMillis") != 0;
			if(p.isTimeBased){
				p.videoMillis = settings.getValue("videoMillis", 0);
				p.depthMillis = settings.getValue("depthMillis", 0);
			}
			else{
				p.videoMillis = settings.getValue("video", 0);
				p.depthMillis = settings.getValue("depth", 0);
			}
			alignedFrames.push_back(p);			
			settings.popTag();
		}
		return true;
	}
	else{
		ofLogError("ofxRGBDVideoDepthSequence -- error loading file " + pairFileXml);
		return false;
	}	
}

void ofxRGBDVideoDepthSequence::reset(){
    alignedFrames.clear();
}

void ofxRGBDVideoDepthSequence::addAlignedFrames(int videoMillis, int depthMillis){
	VideoDepthPair pair;
	pair.isTimeBased = false;
	pair.videoMillis = videoMillis;
	pair.depthMillis = depthMillis;
	addAlignedPair(pair);
}

void ofxRGBDVideoDepthSequence::addAlignedTime(int videoMillis, int depthMillis){
	cout << "added aligned time of video " << videoMillis << " to depth " << depthMillis << endl;

	VideoDepthPair pair;
	pair.isTimeBased = true;
	pair.videoMillis = videoMillis;
	pair.depthMillis = depthMillis;
	addAlignedPair(pair);
}

bool ofxRGBDVideoDepthSequence::isSequenceTimebased(){
	if(alignedFrames.size() == 0){
		return false;
	}
	return alignedFrames[0].isTimeBased;
}

void ofxRGBDVideoDepthSequence::addAlignedPair(VideoDepthPair pair){
    //exclude dupes
    for(int i = 0; i < alignedFrames.size(); i++){
        if(pair.videoMillis == alignedFrames[i].videoMillis || pair.depthMillis == alignedFrames[i].depthMillis){
            return;
        }
    }
    
	alignedFrames.push_back(pair);
	sort(alignedFrames.begin(), alignedFrames.end(), pairsort);
}

void ofxRGBDVideoDepthSequence::removeAlignedPair(int index){
	alignedFrames.erase(alignedFrames.begin()+index);
}

bool ofxRGBDVideoDepthSequence::ready(){
	return (alignedFrames.size() > 0 &&  alignedFrames[0].isTimeBased) ||
		   (alignedFrames.size() > 1 && !alignedFrames[0].isTimeBased);
}

long ofxRGBDVideoDepthSequence::getDepthMillisForVideoMillis(long videoMillis){

	if(!ready()){
		return 0;
	}

    if(alignedFrames[0].isTimeBased && alignedFrames.size() == 1){
    	return (alignedFrames[0].depthMillis - alignedFrames[0].videoMillis) + videoMillis;  
    }
    
    int startIndex, endIndex;
    if(videoMillis < alignedFrames[0].videoMillis){
        startIndex = 0;
        endIndex = 1;
    }
    if(videoMillis > alignedFrames[alignedFrames.size()-1].videoMillis){
        startIndex = alignedFrames.size()-2;
        endIndex = alignedFrames.size()-1;
    }
    else {
        startIndex = 0;
        endIndex = 1;
        while(videoMillis > alignedFrames[endIndex].videoMillis){
            startIndex++;
            endIndex++;
        }
    }
    
    if(endIndex == alignedFrames.size()){
        startIndex--;
        endIndex--;
    }
    
    long mapping = ofMap(videoMillis, alignedFrames[startIndex].videoMillis, alignedFrames[endIndex].videoMillis,
                        alignedFrames[startIndex].depthMillis, alignedFrames[endIndex].depthMillis, false);	
    //		cout << "looking for video frame " << videoMillis << " mapped to depth " << mapping << " found to be between " << startIndex << " and " << endIndex <<endl;
    return mapping;
}

long ofxRGBDVideoDepthSequence::getVideoMillisForDepthMillis(long depthMillis){
    
	if(!ready()){
		return 0;
	}
    
    if(alignedFrames[0].isTimeBased && alignedFrames.size() == 1){
    	return (alignedFrames[0].videoMillis - alignedFrames[0].depthMillis) + depthMillis;  
    }
    
    int startIndex, endIndex;
    if(depthMillis < alignedFrames[0].depthMillis){
        startIndex = 0;
        endIndex = 1;
    }
    if(depthMillis > alignedFrames[alignedFrames.size()-1].depthMillis){
        startIndex = alignedFrames.size()-2;
        endIndex = alignedFrames.size()-1;
    }
    else {
        startIndex = 0;
        endIndex = 1;
        while(depthMillis > alignedFrames[endIndex].depthMillis){
            startIndex++;
            endIndex++;
        }
    }
    
    if(endIndex == alignedFrames.size()){
        startIndex--;
        endIndex--;
    }
    
    long mapping = ofMap(depthMillis, alignedFrames[startIndex].depthMillis, alignedFrames[endIndex].depthMillis,
                         alignedFrames[startIndex].videoMillis, alignedFrames[endIndex].videoMillis, false);	
    //		cout << "looking for video frame " << videoMillis << " mapped to depth " << mapping << " found to be between " << startIndex << " and " << endIndex <<endl;
    return mapping;
}

ofRange ofxRGBDVideoDepthSequence::getStartAndEndTimes(ofVideoPlayer& player, ofxDepthImageSequence& sequence){
    if(!ready()){
        ofLogError("ofxRGBDVideoDepthSequence::getStartAndEndTimes -- video sequence not ready");
        return ofRange();
    }
    ofRange output;
    output.min = MAX(0, getVideoMillisForDepthMillis(0))/1000.;
    output.max = MIN(player.getDuration(), getVideoMillisForDepthMillis(sequence.getDurationInMillis()) / 1000. );
	return output;
}

vector<VideoDepthPair>& ofxRGBDVideoDepthSequence::getPairs(){
	return alignedFrames;
}
