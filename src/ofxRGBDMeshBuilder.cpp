//
//  ofxRGBDMeshBuilder.cpp
//  RGBDVisualize
//
//  Created by James George on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "ofxRGBDMeshBuilder.h"

ofxRGBDMeshBuilder::ofxRGBDMeshBuilder(){
    farClip = 5000;
	nearClip = 300;
    edgeSnip = 500;
    addColors = false;
	depthOnly = false;
	
    undistortDepthImage = true;
    calibrationSetup = false;
    calculateTextureCoordinates = true;
    normalizeTextureCoordinates = false;
    hasTriangles = false;
    simplify = -1;
    textureScale = ofVec2f(1.0, 1.0);
	mirror = false;
	
	currentTexture = NULL;
	currentDepthPixels = NULL;
	
	pivot = ofVec3f(0,0,0);
	worldPosition = ofVec3f(0,0,0);
	worldRotation = ofVec3f(0,0,0);
}

ofxRGBDMeshBuilder::~ofxRGBDMeshBuilder(){
}

bool ofxRGBDMeshBuilder::setDepthOnly(){
	fov.x = 5.7034220279543524e+02;
	fov.y = 5.7034220280129011e+02;
	principalPoint.x = 320;
	principalPoint.y = 240;
	imageSize.width = 640;
	imageSize.height = 480;
	depthOnly = true;
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
    
	fov.x = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(0,0);
	fov.y = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(1,1);
	principalPoint = depthCalibration.getDistortedIntrinsics().getPrincipalPoint();
    
//    Point2d fov = depthCalibration.getDistortedIntrinsics().getFov();
//	angleFov.x = tanf(ofDegToRad(fov.x) / 2) * 2;
//	angleFov.y = tanf(ofDegToRad(fov.y) / 2) * 2;
//    cout << "fx and fy " << fx << " " << fy << endl;
	
	imageSize = depthCalibration.getDistortedIntrinsics().getImageSize();
	depthOnly = false;
	
	return (calibrationSetup = true);   
}

int ofxRGBDMeshBuilder::getSimplification(){
    return simplify;
}

void ofxRGBDMeshBuilder::setSimplification(int simplifyLevel){
    if(!calibrationSetup && !depthOnly){
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
    
//    cout << "set simplification level to " << simplifyLevel << " for image size " << imageSize.width << " x " << imageSize.height << " #verts " << mesh.getNumVertices() << endl;
}

void ofxRGBDMeshBuilder::update(){
	if(currentDepthPixels != NULL){
		update(*currentDepthPixels);
	}
	else {
		ofLogError("ofxRGBDMeshBuilder") << "calling updated without a mesh sent";
	}
}

void ofxRGBDMeshBuilder::update(ofShortPixels& depthImage){
    
    if(!calibrationSetup && !depthOnly){
     	ofLogError("ofxRGBDMeshBuilder::updateMesh() -- no calibration set up");
        return;
    }
    
	if(!depthImage.isAllocated()){
		ofLogError("ofxRGBDMeshBuilder::update") << "depth pix are not allocated";
		return;
	}

	if(depthImage.getWidth() != imageSize.width || depthImage.getHeight() != imageSize.height){
		ofLogError("ofxRGBDMeshBuilder::update") << "depth pix dimensions don't match, provided " << ofVec2f(depthImage.getWidth(), depthImage.getHeight()) << " expecting " << ofVec2f(imageSize.width,imageSize.height);
		return;
	}
    //default
    if(simplify == -1){
        setSimplification(1);
    }
    
	holeFiller.close(depthImage);
	
	//undistort the current images
//    if(undistortDepthImage){
//        depthCalibration.undistort( toCv(depthImage), CV_INTER_NN);
//    }
    
	
    //feed the zed values into the mesh
    int vertexIndex = 0;    
    hasTriangles = false;
    unsigned short* ptr = depthImage.getPixels();    
    for(int y = 0; y < imageSize.height; y += simplify) {
        for(int x = 0; x < imageSize.width; x += simplify) {
            mesh.setVertex(vertexIndex++, getWorldPoint(x,y,ptr[y*imageSize.width+x]));
        }
    }

    mesh.clearIndices();    
    for(int i = 0; i < baseIndeces.size(); i+=3){
        ofVec3f& a = mesh.getVertices()[baseIndeces[i]];
        if(a.z > farClip || a.z < nearClip){
            continue;
        }        
        
        ofVec3f& b = mesh.getVertices()[baseIndeces[i+1]];
        if(b.z > farClip || b.z < nearClip){
            continue;
        }
        
        ofVec3f& c = mesh.getVertices()[baseIndeces[i+2]];
        if(c.z > farClip || c.z < nearClip){
            continue;
        }

        if(fabs(a.z - b.z) > edgeSnip || fabs(a.z - c.z) > edgeSnip){
            continue;
        }
        
        mesh.addTriangle(baseIndeces[i], baseIndeces[i+1], baseIndeces[i+2]);
        hasTriangles = true;
    }

	if(calculateTextureCoordinates && !depthOnly){
        generateTextureCoordinates();
	}
    
    //cout << "updated mesh has " << mesh.getNumIndices()/3 << " triangles " << endl;
}

ofMesh& ofxRGBDMeshBuilder::getMesh(){
    return mesh;
}

void ofxRGBDMeshBuilder::generateTextureCoordinates(){
    if(!calibrationSetup){
        ofLogError("ofxRGBDRenderer::generateTextureCoordinates -- no calibration set up");
        return;
    }
    
    if(!mesh.hasTexCoords()){
        for (int y = 0; y < imageSize.height; y+=simplify){
            for (int x=0; x < imageSize.width; x+=simplify){
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
		if(!mirror){
			texCd.x = 1-texCd.x;
		}
		texCd *= scale;
        texCd += shift;
        texCd *= ofVec2f(rgbImage.width,rgbImage.height) * textureScale;
        mesh.setTexCoord(i, texCd);			
	}
}

ofVec3f ofxRGBDMeshBuilder::getWorldPoint(float x, float y, ofShortPixels& pixels){
	unsigned short z =  pixels.getPixels()[ pixels.getPixelIndex(x, y) ];
    return getWorldPoint(x,y,z);
}

ofVec3f ofxRGBDMeshBuilder::getWorldPoint(float x, float y, unsigned short z){
    //return ofVec3f(((principalPoint.x - x) / imageSize.width) * z * fx, ((principalPoint.y - y) / imageSize.height) * z * fy, z);
//    return ofVec3f(((x - principalPoint.x) / imageSize.width) * z * fx, 
//                   ((y - principalPoint.y) / imageSize.height) * z * fy, z);
	return ofVec3f( (mirror ? 1 : -1) * (x - principalPoint.x) * z / fov.x, (y - principalPoint.y) * z / fov.y, z);	
}


void ofxRGBDMeshBuilder::setPivotToMeshCenter(){

	ofVec3f center(0,0,0);
	int vertsAdded = 0;
	for(int i = 0; i < mesh.getVertices().size(); i++){
		if(mesh.getVertices()[i] != ofVec3f(0,0,0) && mesh.getVertices()[i].z < farClip){
			center += mesh.getVertices()[i];
			vertsAdded++;
		}
	}
	pivot = center / vertsAdded;

//	pivot = mesh.getCentroid();
}

ofxDepthHoleFiller& ofxRGBDMeshBuilder::getHoleFiller(){
	return holeFiller;
}

void ofxRGBDMeshBuilder::setTexture(ofBaseHasTexture& texture){
	currentTexture = &texture;
	setTextureScaleForImage(texture);
}

void ofxRGBDMeshBuilder::setDepthPixels(ofShortPixels& pixels){
	currentDepthPixels = &pixels;
}

void ofxRGBDMeshBuilder::draw(){
	if(currentTexture != NULL){
		draw(*currentTexture);
	}
	else{
//		cout << "drawing wireframe mesh " << endl;
		ofPushMatrix();
		setupDrawMatrices();
		mesh.drawWireframe();
		ofPopMatrix();
	}
}

void ofxRGBDMeshBuilder::setupDrawMatrices(){
	ofTranslate(worldPosition);
	
	ofTranslate(pivot);
	ofRotate(worldRotation.x, 1, 0, 0);
	ofRotate(worldRotation.y, 0, 1, 0);
	ofRotate(worldRotation.z, 0, 0, 1);
	ofTranslate(-pivot);
	
	ofScale(1,-1, 1);
}

void ofxRGBDMeshBuilder::draw(ofBaseHasTexture& texture){
    if(!calibrationSetup || !hasTriangles || depthOnly){
        return;
    }
    ofPushMatrix();
	setupDrawMatrices();
    texture.getTextureReference().bind();
    mesh.drawWireframe();
    texture.getTextureReference().unbind();
    ofPopMatrix();
}

void ofxRGBDMeshBuilder::setTextureScaleForImage(ofBaseHasTexture& texture){
	if(!calibrationSetup){
		ofLogError("ofxRGBDMeshBuilder::setTextureScaleForImage") << "must set up matrices before setting texture scale";
		return;
	}
	
    cv::Size rgbImage = rgbCalibration.getDistortedIntrinsics().getImageSize();
    textureScale = ofVec2f(float(texture.getTextureReference().getWidth() / float(rgbImage.width)  ),
                           float(texture.getTextureReference().getHeight()) / float(rgbImage.height) );    
}

void ofxRGBDMeshBuilder::setXYShift(ofVec2f newShift){
    shift = newShift;
}