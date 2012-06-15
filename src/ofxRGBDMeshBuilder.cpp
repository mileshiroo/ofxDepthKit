//
//  ofxRGBDMeshBuilder.cpp
//  RGBDVisualize
//
//  Created by James George on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "ofxRGBDMeshBuilder.h"
#include <set>

ofxRGBDMeshBuilder::ofxRGBDMeshBuilder(){
    farClip = 5000;
    edgeSnip = 500;
    addColors = false;
    undistortDepthImage = true;
    calibrationSetup = false;
    calculateTextureCoordinates = true;
    normalizeTextureCoordinates = false;
    hasTriangles = false;
    simplify = -1;
    textureScale = ofVec2f(1.0, 1.0);
}

ofxRGBDMeshBuilder::~ofxRGBDMeshBuilder(){
}

bool ofxRGBDMeshBuilder::setup(string calibrationDirectory){
 	if(!ofDirectory(calibrationDirectory).exists()){
		ofLogError("ofxRGBDRenderer --- Calibration directory doesn't exist: " + calibrationDirectory);
		return false;
	}
	
	depthCalibration.load(calibrationDirectory+"/depthCalib.yml");
	rgbCalibration.load(calibrationDirectory+"/rgbCalib.yml");
	
	loadMat(rotationDepthToRGB, calibrationDirectory+"/rotationDepthToRGB.yml");
	loadMat(translationDepthToRGB, calibrationDirectory+"/translationDepthToRGB.yml");
    
    
    Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
	fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	fy = tanf(ofDegToRad(fov.y) / 2) * 2;
    
	principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();

	return (calibrationSetup = true);   
}

int ofxRGBDMeshBuilder::getSimplification(){
    return simplify;
}

void ofxRGBDMeshBuilder::setSimplification(int simplifyLevel){
    if(!calibrationSetup){
        return;
    }
    

    if(simplify == simplifyLevel){
        return;
    }
    
	simplify = simplifyLevel;
	if (simplify <= 0) {
		simplify = 1;
	}
	else if(simplify > 8){
		simplify = 8;
	}
	
	baseIndeces.clear();
	int w = imageSize.width / simplify;
	int h = imageSize.height / simplify;
	for (int y = 0; y < h-1; y++){
		for (int x=0; x < w-1; x++){
			ofIndexType a,b,c;
			a = x+y*w;
			b = x+(y+1)*w;
			c = (x+1)+y*w;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);
			            
			a = (x+1)+y*w;
			b = x+(y+1)*w;
			c = (x+1)+(y+1)*w;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);
        }
	}		
	    
	mesh.clearVertices();
	for (int y = 0; y < imageSize.height; y+=simplify){
		for (int x = 0; x < imageSize.width; x+=simplify){
			mesh.addVertex(ofVec3f(x,y,0));
		}
	}
    
    if(addColors){
        mesh.clearColors();
        for (int y = 0; y < imageSize.height; y+=simplify){
            for (int x = 0; x < imageSize.width; x+=simplify){
                mesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
            }
        }        
    }
    
    if(calculateTextureCoordinates){
        mesh.clearTexCoords();
        for (int y = 0; y < imageSize.height; y+=simplify){
            for (int x = 0; x < imageSize.width; x+=simplify){
                mesh.addTexCoord(ofVec2f(0,0));
            }
        }        
    }
    
    cout << "set simplification level to " << simplifyLevel << " for image size " << imageSize.width << " x " << imageSize.height << " #verts " << mesh.getNumVertices() << endl;
}

void ofxRGBDMeshBuilder::updateMesh(ofShortPixels& depthImage){
    
    if(!calibrationSetup){
     	ofLogError("ofxRGBDMeshBuilder::updateMesh() -- no calibration set up");
        return;
    }
    
    //default
    if(simplify == -1){
        setSimplification(1);
    }
    
	//undistort the current images
    if(undistortDepthImage){
        depthCalibration.undistort( toCv(depthImage), CV_INTER_NN);
    }
    
    //feed the zed values into the mesh
    int vertexIndex = 0;    
    hasTriangles = false;
    unsigned short* ptr = depthImage.getPixels();    
    for(int y = 0; y < imageSize.height; y += simplify) {
        for(int x = 0; x < imageSize.width; x += simplify) {
            mesh.setVertex(vertexIndex++, getWorldPoint(x,y,ptr[y*imageSize.width+x]));
        }
    }

    //set<ofIndexType> calculatedNormals;
    //    if(calculateNormals){
    //        mesh.getNormals().resize(mesh.getVertices().size());
    //    }
    
    mesh.clearIndices();    
    for(int i = 0; i < baseIndeces.size(); i+=3){
        ofVec3f& a = mesh.getVertices()[baseIndeces[i]];
        if(a.z > farClip || a.z < 200){
            continue;
        }        
        
        ofVec3f& b = mesh.getVertices()[baseIndeces[i+1]];
        if(b.z > farClip || b.z < 200){
            continue;
        }
        
        ofVec3f& c = mesh.getVertices()[baseIndeces[i+2]];
        if(c.z > farClip || c.z < 200){
            continue;
        }

        if(fabs(a.z - b.z) > edgeSnip || fabs(a.z - c.z) > edgeSnip){
            continue;
        }
        
        mesh.addTriangle(baseIndeces[i], baseIndeces[i+1], baseIndeces[i+2]);
        hasTriangles = true;
    }

	if(calculateTextureCoordinates){
        generateTextureCoordinates();
	}
    
    //cout << "updated mesh has " << mesh.getNumIndices()/3 << " triangles " << endl;
}

void ofxRGBDMeshBuilder::generateTextureCoordinates(){
    if(!calibrationSetup){
        ofLogError("ofxRGBDRenderer::generateTextureCoordinates -- no calibration set up");
        return;
    }
    
    if(!mesh.hasTexCoords()){
        for (int y = 0; y < imageSize.height; y++){
            for (int x=0; x < imageSize.width; x++){
                mesh.addTexCoord(ofVec2f(0,0));
            }
        }        
    }
    
    Mat pcMat = Mat(toCv(mesh));		
    vector<cv::Point2f> imagePoints;    
    projectPoints(pcMat,
                  rotationDepthToRGB, translationDepthToRGB,
                  rgbCalibration.getDistortedIntrinsics().getCameraMatrix(),
                  rgbCalibration.getDistCoeffs(),
                  imagePoints);
    cv::Size rgbImage = rgbCalibration.getDistortedIntrinsics().getImageSize();
    for(int i = 0; i < imagePoints.size(); i++) {
        ofVec2f texCd = ofVec2f(imagePoints[i].x, imagePoints[i].y);
        texCd /= ofVec2f(rgbImage.width,rgbImage.height);
        texCd += shift;
        texCd *= ofVec2f(rgbImage.width,rgbImage.height) * textureScale;
        mesh.setTexCoord(i, texCd);			
	}
}

ofVec3f ofxRGBDMeshBuilder::getWorldPoint(float x, float y, unsigned short z){
    //return ofVec3f(((principalPoint.x - x) / imageSize.width) * z * fx, ((principalPoint.y - y) / imageSize.height) * z * fy, z);
    return ofVec3f(((x - principalPoint.x) / imageSize.width) * z * fx, 
                   ((y - principalPoint.y) / imageSize.height) * z * fy, z);
}

void ofxRGBDMeshBuilder::draw(ofBaseHasTexture& texture){
    if(!calibrationSetup || !hasTriangles){
        return;
    }
    ofPushMatrix();
    ofScale(1,-1, 1);
    texture.getTextureReference().bind();
    mesh.drawWireframe();
    texture.getTextureReference().unbind();
    ofPopMatrix();
}

void ofxRGBDMeshBuilder::setTextureScaleForImage(ofBaseHasTexture& texture){
    cv::Size rgbImage = rgbCalibration.getDistortedIntrinsics().getImageSize();
    textureScale = ofVec2f(float(texture.getTextureReference().getWidth() / float(rgbImage.width)  ),
                           float(texture.getTextureReference().getHeight()) / float(rgbImage.height) );    
}

void ofxRGBDMeshBuilder::setXYShift(ofVec2f newShift){
    shift = newShift;
}