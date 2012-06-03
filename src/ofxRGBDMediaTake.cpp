//
//  ofxRGBDMediaTake.cpp
//  ScreenLabRenderer
//
//  Created by James George on 4/16/12.
//

#include "ofxRGBDMediaTake.h"

ofxRGBDMediaTake::ofxRGBDMediaTake(){
    
    hasCalibration = false;
    hasPairings = false;
    hasDepth = false;
    hasColor = false;
    hasAlternativeHiResVideo = false;
    
    compressedDepthFrameCount = 0;
    uncompressedDepthFrameCount = 0;
}

bool ofxRGBDMediaTake::loadFromFolder(string sourceMediaFolder){
	
	mediaFolder = sourceMediaFolder;
    
    hasPairings = false;
    hasCalibration = false;
    hasDepth = false;
    hasColor = false;
    hasAlternativeHiResVideo = false;
    
    calibrationFolder = "";
    videoPath = "";
    alternativeHiResVideoPath = "";
    pairingsFile = "";
	
    ofDirectory dataDirectory(mediaFolder);
    if(!dataDirectory.exists()){
        ofLogWarning("ofxRGBDMediaTake::loadFromFolder -- folder " + mediaFolder + " -- Directory doesn't exist.");
        return false;
    }
    
	dataDirectory.listDir();    
	int numFiles = dataDirectory.numFiles();
    if(numFiles == 0){
        ofLogWarning("ofxRGBDMediaTake::loadFromFolder -- folder " + mediaFolder + " -- Directory is empty.");
        return false;        
    }

    vector<string> components = ofSplitString( mediaFolder, "/");
    name = components[components.size()-1];
    
    //////////////////////////////////////////////
    // DEPTH
    //////////////////////////////////////////////

    bool depthFolderFound = false;
    for(int i = 0; i < dataDirectory.numFiles(); i++){
        if(dataDirectory.getName(i).find("depth") != string::npos && dataDirectory.getFile(i).isDirectory()){
            depthFolder = dataDirectory.getPath(i);
            depthFolderFound = true;
        }
    }
    if(!depthFolderFound){
	    depthFolder = mediaFolder + "/depth/";
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
            string destinationPath = ofFilePath::getEnclosingDirectory(mainDirectory.getPath(i)) + "depth/" + mainDirectory.getName(i);
            cout << "ofxRGBDMediaTake -- Legacy Format -- moved to " << destinationPath << endl;
        	mainDirectory.getFile(i).moveTo( destinationPath );
        }
    }
    
    if(depthDirectory.exists()){
        
        ofDirectory compresseDepthFrames(depthFolder);
        compresseDepthFrames.allowExt("png");
        compressedDepthFrameCount = compresseDepthFrames.listDir();
        
        ofDirectory uncompressedDepthFrames(depthFolder);
        uncompressedDepthFrames.allowExt("raw");
        uncompressedDepthFrameCount = uncompressedDepthFrames.listDir();
        
        totalDepthFrameCount = uncompressedDepthFrameCount + compressedDepthFrameCount;
        hasDepth = (totalDepthFrameCount > 0);
    }
    
    //////////////////////////////////////////////
    // END DEPTH
    //////////////////////////////////////////////

    //////////////////////////////////////////////
    // COLOR
    //////////////////////////////////////////////
  
    string colorFolder = mediaFolder + "/color/";
    ofDirectory colorDirectory = ofDirectory(colorFolder);
	ofDirectory mainDirectoryColor = ofDirectory(sourceMediaFolder);
    mainDirectoryColor.allowExt("mpeg");
    mainDirectoryColor.allowExt("mov");
    mainDirectoryColor.allowExt("mpg");
    mainDirectoryColor.listDir();
    
    //move the movies into the color/ dir if they are hanging outside
    if(mainDirectoryColor.numFiles() > 0){
        if(!colorDirectory.exists()){
            colorDirectory.create();
        }
        
        for(int i = 0; i < mainDirectoryColor.numFiles(); i++){
            string destinationPath = ofFilePath::getEnclosingDirectory(mainDirectoryColor.getPath(i)) + "color/" + mainDirectoryColor.getName(i);
            cout << "ofxRGBDMediaTake -- Legacy Format -- moved to " << destinationPath << endl;
        	mainDirectoryColor.getFile(i).moveTo( destinationPath );
        }

    }

    if(colorDirectory.exists()){
        colorDirectory.allowExt("mpeg");
        colorDirectory.allowExt("mov");
        colorDirectory.allowExt("mpg");
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
            for(int i = i; i < colorDirectory.numFiles(); i++){
                if(largestSize < colorDirectory.getFile(i).getSize()){
                    largestSize = colorDirectory.getFile(i).getSize();
                    largestIndex = i;
                }
                if(smallestSize > colorDirectory.getFile(i).getSize()){                    
                    smallestSize = colorDirectory.getFile(i).getSize();
                    smallestSize = i;
                }
            }
            
            hasColor = true;
            hasAlternativeHiResVideo = true;
            alternativeHiResVideoPath = colorDirectory.getPath(largestIndex);
            videoPath = colorDirectory.getPath(smallestIndex); 
        }
        
        if(hasColor){
            videoThumbsPath = ofFilePath::removeExt(videoPath);
            if(!ofDirectory(videoThumbsPath).exists()){
                ofDirectory(videoThumbsPath).create(true);
            }
            
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
                break;
            }
        }

        if(!hasPairings){
            pairingsFile = mediaFolder + "/pairings.xml";
        }
    }
    //////////////////////////////////////////////
    // END PAIRINGS FILE
    //////////////////////////////////////////////

    
    //////////////////////////////////////////////
    // CALIBRATION
    //////////////////////////////////////////////
	if(hasColor){
        calibrationFolder = mediaFolder + "/calibration/";
        ofDirectory calibrationDirectory = ofDirectory(calibrationFolder);
        hasCalibration = calibrationDirectory.exists();
        if(!hasCalibration){
            //look for it above!
            calibrationDirectory = ofDirectory(sourceMediaFolder + "../calibration/");
            hasCalibration = calibrationDirectory.exists();
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

    /*
	for(int i = 0; i < numFiles; i++){
		string testFile = dataDirectory.getName(i);
		if(testFile.find("calibration") != string::npos){
			calibrationDirectory = dataDirectory.getPath(i);
            hasCalibrationDirectory = true;
		}
		
//		if(testFile.find("depth") != string::npos || testFile.find("TAKE") != string::npos){
//			depthFolder = dataDirectory.getPath(i);
//            hasDepthFolder = true;
//		}
		
		if(testFile.find("mov") != string::npos || testFile.find("MOV") != string::npos ){
			if(testFile.find("small") == string::npos){
				hiResVideoPath = dataDirectory.getPath(i);
                hasLargeVideoFile = true;
			}
			else {
				lowResVideoPath = dataDirectory.getPath(i);
                hasSmallVideoFile = true;
                videoThumbsPath = ofFilePath::removeExt(lowResVideoPath);
                if(!ofDirectory(videoThumbsPath).exists()){
                    ofDirectory(videoThumbsPath).create(true);
                }
			}
		}		
		
     */
//		if(testFile.find("pairings") != string::npos){
//			pairingsFile = dataDirectory.getPath(i);
//		}
//	}
	
//	if(!hasSmallVideoFile){
//		//ofSystemAlertDialog("Error loading media folder " + mediaFolder + " no Small Video File found.");
//		return false;
//	}
//    
//	if(!hasCalibrationDirectory){
//		//ofSystemAlertDialog("Error loading media folder " + mediaFolder + ". No calibration/ directory found.");
//		return false;	
//	}
//    
//	if(!hasDepthFolder){
//		//ofSystemAlertDialog("Error loading media folder " + mediaFolder + ". No Depth directory found. Make sure the folder containing the depth images has 'depth' or 'take' in the name");
//		return false;	
//	}
//	
//	if(pairingsFile == ""){
//		pairingsFile = ofFilePath::removeExt(lowResVideoPath) + "_pairings.xml";
//	}
  
	return valid();
}

bool ofxRGBDMediaTake::valid(){
//    return  hasCalibrationDirectory && hasDepthFolder && hasSmallVideoFile;
    return (hasDepth && !hasColor) || (hasDepth && hasColor && hasPairings && hasCalibration);
}

//vector<ofxRGBDRenderSettings>& ofxRGBDMediaTake::getRenderSettings() {
//    return renderSettings;
//}
