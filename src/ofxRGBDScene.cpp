//
//  ofxRGBDScene.cpp
//  ScreenLabRenderer
//
//  Created by James George on 4/16/12.
//

#include "ofxRGBDScene.h"

ofxRGBDScene::ofxRGBDScene(){
	clear();	
}

void ofxRGBDScene::clear(){
	hasCalibration = false;
	hasPairings = false;
	hasDepth = false;
	hasColor = false;
	hasXYShift = false;
	hasAlternativeHiResVideo = false;
	
	compressedDepthFrameCount = 0;
	uncompressedDepthFrameCount = 0;
	
	#ifdef TARGET_WIN32
	pathDelim = "\\";
	#else
	pathDelim = "/";
	#endif
}

//static tester to see if folder is a valid bin
bool ofxRGBDScene::isFolderValid(string sourceMediaFolder){
	ofxRGBDScene c;
	return c.loadFromFolder(sourceMediaFolder, false);
}

vector<string> ofxRGBDScene::getValidVideoExtensions(){
	vector<string> extensions;
	extensions.push_back("mov");
	extensions.push_back("mpg");
	extensions.push_back("mepg");
	extensions.push_back("mp4");
	extensions.push_back("avi");
	return extensions;
}

bool ofxRGBDScene::loadFromFolder(string sourceMediaFolder, bool countFrames){
	
	
	clear();
		
	calibrationFolder = "";
	videoPath = "";
	alternativeHiResVideoPath = "";
	pairingsFile = "";
	
	mediaFolder = sourceMediaFolder;

	if(sourceMediaFolder.find("_calibration") != string::npos) {
		ofLogWarning("ofxRGBDScene::loadFromFolder -- Discarding _calibration folder");
		return false;
	}
	if(sourceMediaFolder.find("_Renderbin/") != string::npos){
		ofLogWarning("ofxRGBDScene::loadFromFolder -- Discarding Render Bin");
		return false;
	}
	ofDirectory dataDirectory(mediaFolder);
	if(!dataDirectory.exists()){
		ofLogWarning("ofxRGBDScene::loadFromFolder -- folder " + mediaFolder + " -- Directory doesn't exist.");
		return false;
	}
	
	if(!dataDirectory.isDirectory()){
		ofLogWarning("ofxRGBDScene::loadFromFolder -- folder " + mediaFolder + " -- Isn't a directory!");
		return false;
	}
	
	dataDirectory.listDir();	
	int numFiles = dataDirectory.numFiles();
	if(numFiles == 0){
		ofLogWarning("ofxRGBDScene::loadFromFolder -- folder " + mediaFolder + " -- Directory is empty.");
		return false;		
	}
	vector<string> components = ofSplitString( mediaFolder, pathDelim );
	name = components[components.size()-1];
	
	//////////////////////////////////////////////
	// DEPTH
	//////////////////////////////////////////////

	bool depthFolderFound = false;
	for(int i = 0; i < dataDirectory.numFiles(); i++){
		if(dataDirectory.getFile(i).isDirectory() && (dataDirectory.getName(i).find("depth") != string::npos || dataDirectory.getName(i).find("TAKE") != string::npos) ){
			depthFolder = dataDirectory.getPath(i);
			depthFolderFound = true;
		}
	}
	
	if(!depthFolderFound){
		depthFolder = mediaFolder + pathDelim + "depth" + pathDelim;
	}
	
	ofDirectory depthDirectory = ofDirectory(depthFolder);
	
	//potentially a legacy folder. check to see if there are +20 PNG or raws's 
	//indicating the files need to be moved
	ofDirectory mainDirectory(sourceMediaFolder);
	mainDirectory.allowExt("png");
	mainDirectory.allowExt("raw");
	mainDirectory.listDir();
	if(mainDirectory.numFiles() > 20){
		
		//create the depth directory if we haven't yet
		if(!depthDirectory.exists()){
			 depthDirectory.create();
		}
		 
		 //move all the files from the main folder into the depth directory
		for(int i = 0; i < mainDirectory.numFiles(); i++){
			string destinationPath = ofFilePath::getEnclosingDirectory(mainDirectory.getPath(i)) + "depth" + pathDelim + mainDirectory.getName(i);
			//cout << "ofxRGBDScene -- Legacy Format -- moved to " << destinationPath << endl;
			//mainDirectory.getFile(i).moveTo( destinationPath );
		}
	}
	
	if(depthDirectory.exists()){
		if(countFrames){
			ofDirectory compresseDepthFrames(depthFolder);
			compresseDepthFrames.allowExt("png");
			compressedDepthFrameCount = compresseDepthFrames.listDir();
			
			ofDirectory uncompressedDepthFrames(depthFolder);
			uncompressedDepthFrames.allowExt("raw");
			uncompressedDepthFrameCount = uncompressedDepthFrames.listDir();
			
			totalDepthFrameCount = uncompressedDepthFrameCount + compressedDepthFrameCount;
			hasDepth = (totalDepthFrameCount > 0);
		}
		else {
			//guess it
			hasDepth = true;
		}
	}
	
	//////////////////////////////////////////////
	// END DEPTH
	//////////////////////////////////////////////

	//////////////////////////////////////////////
	// COLOR
	//////////////////////////////////////////////
	string colorFolder = mediaFolder + pathDelim + "color" + pathDelim;
	ofDirectory colorDirectory = ofDirectory(colorFolder);
	ofDirectory mainDirectoryColor = ofDirectory(sourceMediaFolder);
	//TODO: make an xml file for video formats
	mainDirectoryColor.allowExt("mov");
	mainDirectoryColor.allowExt("mpg");
	mainDirectoryColor.allowExt("mepg");
	mainDirectoryColor.allowExt("mp4");
	mainDirectoryColor.listDir();
	
	//move the movies into the color/ dir if they are hanging outside
	if(mainDirectoryColor.numFiles() > 0){
		if(!colorDirectory.exists()){
			colorDirectory.create();
		}
		
		for(int i = 0; i < mainDirectoryColor.numFiles(); i++){
			string destinationPath = ofFilePath::getEnclosingDirectory(mainDirectoryColor.getPath(i)) + "color" + pathDelim + mainDirectoryColor.getName(i);
//			cout << "ofxRGBDScene -- Legacy Format -- moved to " << destinationPath << endl;
//			mainDirectoryColor.getFile(i).moveTo( destinationPath );
		}

	}

	if(colorDirectory.exists()){
		//TODO: make an xml file for video formats
		colorDirectory.allowExt("mpeg");
		colorDirectory.allowExt("mov");
		colorDirectory.allowExt("mpg");
		colorDirectory.allowExt("mp4");
		colorDirectory.listDir();
		
		if(colorDirectory.numFiles() == 0){
			hasColor = false;
		}
		else if(colorDirectory.numFiles() == 1){
			hasColor = true;
			videoPath = colorDirectory.getPath(0);
		}
		else {
			int largestIndex = 0;  
			int smallestIndex = 0;
			uint64_t largestSize = colorDirectory.getFile(0).getSize();
			uint64_t smallestSize = colorDirectory.getFile(0).getSize();
			for(int i = 0; i < colorDirectory.numFiles(); i++){
				uint64_t size = colorDirectory.getFile(i).getSize();
				//cout << colorDirectory.getName(i) << " size is " << size << endl;
				if(largestSize < size){
					largestSize = size;
					largestIndex = i;
				}
				if(size < smallestSize){					
					smallestSize = size;
					smallestIndex = i;
				}
			}
			
			hasColor = true;
			hasAlternativeHiResVideo = true;
			alternativeHiResVideoPath = colorDirectory.getPath(largestIndex);
			videoPath = colorDirectory.getPath(smallestIndex); 
//			cout << "video path is " << videoPath << " alternative is " << alternativeHiResVideoPath << endl;
//			cout << "largest size is " << largestSize << " smallest size is " << smallestSize << endl;
		}
		
	}	
	//////////////////////////////////////////////
	// END COLOR
	//////////////////////////////////////////////

	//////////////////////////////////////////////
	// PAIRINGS FILE
	//////////////////////////////////////////////
	if(hasColor){
		ofDirectory mainDirectoryPairings = ofDirectory(sourceMediaFolder);
		mainDirectoryPairings.allowExt("xml");
		mainDirectoryPairings.listDir();
		for(int i = 0; i < mainDirectoryPairings.numFiles(); i++){
			if(mainDirectoryPairings.getName(i).find("pairings") != string::npos){
				pairingsFile = mainDirectoryPairings.getPath(i);
				hasPairings = true;
			}
			
			if(mainDirectoryPairings.getName(i).find("xyshift") != string::npos){
				xyshiftFile = mainDirectoryPairings.getPath(i);
				hasXYShift = true;
			}
		}

		if(!hasPairings){
			pairingsFile = mediaFolder + pathDelim + "pairings.xml";
		}
		if(!hasXYShift){
			xyshiftFile = mediaFolder + pathDelim + "xyshift.xml";
		}
	}
	//////////////////////////////////////////////
	// END PAIRINGS FILE
	//////////////////////////////////////////////

	
	//////////////////////////////////////////////
	// CALIBRATION
	//////////////////////////////////////////////
	if(hasColor){
		vector<string> calibrationFolders;
		calibrationFolders.push_back(mediaFolder + "_calibration/matrices/");
		calibrationFolders.push_back(mediaFolder + "/calibration/");
		calibrationFolders.push_back(mediaFolder + "/../_calibration/matrices/");
		for(int i = 0; i < calibrationFolders.size(); i++){
			ofStringReplace(calibrationFolders[i], "/", pathDelim);
			ofDirectory calibrationDirectory = ofDirectory(calibrationFolders[i]);
			hasCalibration = calibrationDirectory.exists();
			if(hasCalibration){
				calibrationFolder = calibrationFolders[i];
				break;
			}
		}
	}
	//////////////////////////////////////////////
	// END CALIBRATION
	//////////////////////////////////////////////


	//////////////////////////////////////////////
	// REPORT
	//////////////////////////////////////////////
	bool debug = true;
	if(debug){
		cout << "REPORT FOR " << sourceMediaFolder << endl;
		cout << "has DEPTH? " << (hasDepth ? "YES" : "NO") << endl;
		if(hasDepth){
			cout << "	# COMPRESSED DEPTH FRAMES " << compressedDepthFrameCount << endl;
			if(uncompressedDepthFrameCount > 0){
				cout << "	# UNCOMPRESSED DEPTH FRAMES " << uncompressedDepthFrameCount << endl;				
			}
		}
		
		cout << "has COLOR? " << (hasColor ? "YES" : "NO") << endl;
		if(hasColor){
			cout << "has VIDEO " << (hasColor ? "YES" : "NO") << endl;
			cout << "has HI RES " << (hasAlternativeHiResVideo ? "YES" : "NO") << endl;
			cout << "has CALIBRATION " << (hasCalibration ? "YES" : "NO") << endl;
			cout << "has PAIRINGS " << (hasPairings ? "YES" : "NO") << endl;
			if(hasPairings){
				cout << "pairings! " << pairingsFile << endl;
			}
		}
	}
	//////////////////////////////////////////////
	// END REPORT
	//////////////////////////////////////////////

	return valid();
}

bool ofxRGBDScene::valid(){
	//return (hasDepth && !hasColor) || (hasDepth && hasColor && hasCalibration);
	return (hasDepth && hasColor && hasCalibration);
}
