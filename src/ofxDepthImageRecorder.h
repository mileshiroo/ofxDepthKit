/*
 *  ofxKinectPointcloudRecorder.h
 *  PointcloudWriter
 *
 *  Created by Jim on 10/20/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#pragma once
#include "ofMain.h"
#include "ofxDepthImageCompressor.h"
#include "ofxRGBDScene.h"
#include "ofxMSATimer.h"

typedef struct {
	unsigned short* pixels;
	string directory;
	string filename;
	int timestamp;
} QueuedFrame;


//thread classes for callbacks
class ofxDepthImageRecorder;
class ofxRGBDRecorderThread : public ofThread {
public:
	ofxDepthImageRecorder* delegate;
	ofxRGBDRecorderThread(ofxDepthImageRecorder* d) : delegate(d){}	
	void threadedFunction();
};

class ofxRGBDEncoderThread : public ofThread {
public:
	ofxDepthImageRecorder* delegate;
	ofxRGBDEncoderThread(ofxDepthImageRecorder* d) : delegate(d){}
	void threadedFunction();	
};

class ofxDepthImageRecorder {
  public:
	ofxDepthImageRecorder();
	~ofxDepthImageRecorder();

	vector<ofxRGBDScene*>& getScenes();
	
	void setup();
	void toggleRecord();
	bool isRecording();
	
	void setRecordLocation(string directory, string filePrefix);
	bool addImage(ofShortPixels& image);
	bool addImage(unsigned short* image);

	int numFramesWaitingSave();
	int numFramesWaitingCompession();
	int numDirectoriesWaitingCompression();
	
	ofxDepthImageCompressor& getCompressor();
	void shutdown();
	
	unsigned long recordingStartTime; //in millis -- potentially should make this more accurate
	
	void encoderThreadCallback();
	void recorderThreadCallback();
	
  protected:
	
	ofxDepthImageCompressor compressor;
	ofxRGBDRecorderThread recorderThread;
	ofxRGBDEncoderThread encoderThread;
	
	bool recording;
	
	void incrementTake();
	
	//start converting the current directory
	vector<ofxRGBDScene*> takes;
	void compressCurrentTake();
	void updateTakes();
	int compressingTakeIndex;	
	
	int framesToCompress;
	
	//unsigned short* encodingBuffer;
	//unsigned short* lastFramePixs;
	ofShortPixels encodingBuffer;
	ofShortPixels lastFramePixs;
	
	int folderCount;
	string currentFolderPrefix;
	string targetDirectory;
	string targetFilePrefix;
	int currentFrame;
	
	ofxMSATimer msaTimer;
	queue<QueuedFrame> saveQueue;
	//queue<string> encodeDirectories;
	//queue<Take*> encodeDirectories;
	queue<ofxRGBDScene*> encodeDirectories;
};
