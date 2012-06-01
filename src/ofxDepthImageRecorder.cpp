/*
 *  ofxDepthImageRecorder.cpp
 *  PointcloudWriter
 *
 *  Created by Jim on 10/20/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#include "ofxDepthImageRecorder.h"
#include "ofxMSATimer.h"

#pragma mark Thread Implementation
void ofxRGBDEncoderThread::threadedFunction(){
    shutdown = false;
	while(isThreadRunning()){
		delegate->encoderThreadCallback();
        ofSleepMillis(2);
	}
    shutdown = true;
}

void ofxRGBDRecorderThread::threadedFunction(){
    shutdown = false;
	while(isThreadRunning()){
		delegate->recorderThreadCallback();
        ofSleepMillis(2);

	}
    shutdown = true;
}

#pragma mark Main Implementation
ofxDepthImageRecorder::ofxDepthImageRecorder()
  : recorderThread(this),
	encoderThread(this)
{
	recording = false;
//	lastFramePixs = NULL;
//	encodingBuffer = NULL;
	framesToCompress = 0;
    compressingTakeIndex = 0;
}

ofxDepthImageRecorder::~ofxDepthImageRecorder(){
//	if(lastFramePixs != NULL){
//		delete lastFramePixs;
//	}
}

void ofxDepthImageRecorder::setup(){
    folderCount = 0;
	currentFrame = 0;
	
//	lastFramePixs = new unsigned short[640*480];
    lastFramePixs.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	memset(lastFramePixs.getPixels(), 0, sizeof(unsigned short)*640*480);

    recorderThread.startThread(true, false);
	encoderThread.startThread(true, false);
}

void ofxDepthImageRecorder::setRecordLocation(string directory, string filePrefix){
    if(numFramesWaitingSave() != 0 || numDirectoriesWaitingCompression() != 0){
        return;
    }
    
	targetDirectory = directory;
	ofDirectory dir(directory);
	if(!dir.exists()){
		dir.create(true);
	}
	targetFilePrefix = filePrefix;
	
    updateTakes();
}

vector<ofxRGBDMediaTake*>& ofxDepthImageRecorder::getTakes(){
	return takes;
}

bool ofxDepthImageRecorder::addImage(ofShortPixels& image){
	return addImage(image.getPixels());
}

bool ofxDepthImageRecorder::addImage(unsigned short* image){
	//confirm that it isn't a duplicate of the most recent frame;
	int framebytes = 640*480*sizeof(unsigned short);
	if(0 != memcmp(image, lastFramePixs.getPixels(), framebytes)){
		QueuedFrame frame;
        //TODO use high res timer!!!
		frame.timestamp = msaTimer.getAppTimeMillis() - recordingStartTime;
		frame.pixels = new unsigned short[640*480];
		memcpy(frame.pixels, image, framebytes);
		memcpy(lastFramePixs.getPixels(), image, framebytes);
		
		char filenumber[512];
		sprintf(filenumber, "%05d", currentFrame); 
		
		char millisstring[512];
		sprintf(millisstring, "%010d", frame.timestamp);
		frame.filename = targetFilePrefix + "_" + filenumber +  "_millis_" + millisstring + ".raw";
		frame.directory = targetDirectory +  "/" + currentFolderPrefix + "/depth/";
				
		recorderThread.lock();
		saveQueue.push( frame );
		recorderThread.unlock();
		
		currentFrame++;
		return true;
	}
	return false;
}

int ofxDepthImageRecorder::numFramesWaitingSave(){
	return saveQueue.size();
}

int ofxDepthImageRecorder::numFramesWaitingCompession(){
	return framesToCompress;
}

int ofxDepthImageRecorder::numDirectoriesWaitingCompression(){
	return encodeDirectories.size();
}

void ofxDepthImageRecorder::toggleRecord(){
	recording = !recording;
	if(recording){
		incrementTake();		
	}
	else {
		compressCurrentTake();
	}
}


bool ofxDepthImageRecorder::isRecording(){
	return recording;
}

void ofxDepthImageRecorder::incrementTake(){
	char takeString[1024] ;
	sprintf(takeString, "TAKE_%02d_%02d_%02d_%02d_%02d", ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
    currentFolderPrefix = string(takeString);
    ofDirectory dir(targetDirectory + "/" + currentFolderPrefix);
    
	if(!dir.exists()){
		dir.create(true);
	}
	
    currentFrame = 0;	
    recordingStartTime = msaTimer.getAppTimeMillis();
}

//start converting the current directory
void ofxDepthImageRecorder::compressCurrentTake(){
	if(currentFolderPrefix == ""){
    	ofLogError("ofxDepthImageRecorder::compressCurrentTake -- working directory not set");
        return;
    }
    ofxRGBDMediaTake* t = new ofxRGBDMediaTake();
    t->loadFromFolder(targetDirectory + "/" + currentFolderPrefix);
    /*
    t->depthFolder = targetDirectory + "/" + currentFolderPrefix + "/depth/" ;
    cout << "encoding take " << t->depthFolder << endl;
    t->framesConverted = 0;
    t->depthFrameCount = 1;
     */
    
    encoderThread.lock();
    takes.push_back(t);
    encodeDirectories.push( t );
    encoderThread.unlock();
}

void ofxDepthImageRecorder::updateTakes(){
    for(int i = 0; i < takes.size(); i++){ 
        delete takes[i];
    }
    takes.clear();
    
    cout << "updating takes " << endl;
    
    ofDirectory dir = ofDirectory(targetDirectory);
	dir.listDir();
	dir.sort();
    
	for(int i = 0; i < dir.numFiles(); i++){
        ofxRGBDMediaTake* t = new ofxRGBDMediaTake();
        t->loadFromFolder(dir.getPath(i));
        if(t->valid()){
            takes.push_back(t);
        }
	}
    
	encoderThread.lock();
	for(int i = 0; i < takes.size(); i++){
		encodeDirectories.push( takes[i] );
	}
	encoderThread.unlock();		
}

ofxDepthImageCompressor& ofxDepthImageRecorder::compressorRef(){
    return compressor;
}

void ofxDepthImageRecorder::shutdown(){
	recorderThread.stopThread(true);
	encoderThread.stopThread(true);
}
											  
void ofxDepthImageRecorder::recorderThreadCallback(){

	QueuedFrame frame;
	bool foundFrame = false;
	recorderThread.lock();
	if(saveQueue.size() != 0){
		frame = saveQueue.front();
		saveQueue.pop();
		foundFrame = true;
	}
	recorderThread.unlock();
	
	if(foundFrame){
		char filenumber[512];
		sprintf(filenumber, "%05d", currentFrame); 
		if(compressor.saveToRaw(frame.directory+frame.filename, frame.pixels)){
			delete frame.pixels;
		}
		else {
			//if the save fils push it back on tehs tack
			recorderThread.lock();
			saveQueue.push(frame);
			recorderThread.unlock();
			ofLogError("ofxDepthImageRecorder -- Save Failed! readding to queue");
		}
	}

}

void ofxDepthImageRecorder::encoderThreadCallback(){

    ofxRGBDMediaTake* take = NULL;
	bool foundDir = false;

	encoderThread.lock();
	if(encodeDirectories.size() != 0){
		foundDir = true;
		take = encodeDirectories.front();
		encodeDirectories.pop();
	}
	encoderThread.unlock();

	if(!foundDir) {
    	return;
    }
    
    //start to convert
    if(take->depthFolder == ""){
        ofLogError("ofxDepthImageCompressor -- Take has empty path string");
        return;
    }
    
    ofDirectory rawDir(take->depthFolder);
    if(!rawDir.exists() || !rawDir.isDirectory()){
        ofLogError("ofxDepthImageRecorder::encoderThreadCallback() -- Does not exist or is not directory " + take->depthFolder);
        return;
    }    

    if(!encodingBuffer.isAllocated()){
        //encodingBuffer = new unsigned short[640*480];
        encodingBuffer.allocate(640,480, OF_IMAGE_GRAYSCALE);
    }

    //if(encodingBuffer == NULL){
    
//    ofDirectory convertedDir(take->depthFolder);
//    convertedDir.allowExt("png");
//    convertedDir.listDir();
    
//    take->compressedDepthFrameCount = rawDir.numFiles() + convertedDir.numFiles();
//    take->uncompressedDepthFrameCount = convertedDir.numFiles();
    
    ofLogVerbose("ofxDepthImageCompressor -- Starting to convert " + ofToString(take->uncompressedDepthFrameCount) + " in " + take->depthFolder);
    framesToCompress = take->uncompressedDepthFrameCount;
    rawDir.allowExt("raw");
    rawDir.listDir();
    
    for(int i = 0; i < rawDir.numFiles(); i++){
        
        //don't do this while recording
        while(recording){
//          ofLogWarning("ofxDepthImageRecorder -- paused converting while recording...");
            ofSleepMillis(25);
        }
        
        if(!encoderThread.isThreadRunning()){
            ofLogWarning( "ofxDepthImageRecorder -- Breaking conversion because recorder isn't running");
            break;
        }
        
        string path = rawDir.getPath(i);
        //READ IN THE RAW FILE
        compressor.readDepthFrame(path, encodingBuffer.getPixels());
        //COMPRESS TO PNG
        compressor.saveToCompressedPng(ofFilePath::removeExt(path)+".png", encodingBuffer.getPixels());
        //DELETE the file
        ofFile::removeFile(rawDir.getPath(i));
        //UPDATE COUNTS
        framesToCompress = take->uncompressedDepthFrameCount;
        take->uncompressedDepthFrameCount--;
        take->compressedDepthFrameCount++;
    }
}

