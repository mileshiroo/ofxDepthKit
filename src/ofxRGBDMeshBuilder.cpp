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
	nearClip = 20;
    edgeClip = 500;
    addColors = false;
	depthOnly = false;
	
	topClip = 0.0;
	bottomClip = 1.0;
	leftClip = 0.0;
	rightClip = 1.0;

    undistortDepthImage = true;
    calibrationSetup = false;
    calculateTextureCoordinates = true;
    normalizeTextureCoordinates = false;
    hasTriangles = false;
    
    setSimplification(ofVec2f(1,1));
    textureScale = ofVec2f(1.0, 1.0);
	mirror = false;
	scale = ofVec2f(1,1);
	
	currentTexture = NULL;
	currentDepthPixels = NULL;

	cacheValidVertices = false;
	
	pivot = ofVec3f(0,0,0);
	worldPosition = ofVec3f(0,0,0);
	worldRotation = ofVec3f(0,0,0);
}

ofxRGBDMeshBuilder::~ofxRGBDMeshBuilder(){
}

void ofxRGBDMeshBuilder::setDepthOnly(){
    
    //default kinect intrinsics
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
    
//  Point2d fov = depthCalibration.getDistortedIntrinsics().getFov();
//	angleFov.x = tanf(ofDegToRad(fov.x) / 2) * 2;
//	angleFov.y = tanf(ofDegToRad(fov.y) / 2) * 2;
//  cout << "fx and fy " << fx << " " << fy << endl;
	
	imageSize = depthCalibration.getDistortedIntrinsics().getImageSize();
	depthOnly = false;
	
	return (calibrationSetup = true);   
}

ofVec2f ofxRGBDMeshBuilder::getSimplification(){
    return simplify;
}

void ofxRGBDMeshBuilder::setSimplification(ofVec2f simplification){
    if(!calibrationSetup && !depthOnly){
        return;
    }
    
    if(simplify == simplification){
        return;
    }
    
	simplify = simplification;
    simplify.x = ofClamp(simplify.x, 0, 8);
    simplify.y = ofClamp(simplify.y, 0, 8);
	
    baseIndeces.clear();
    int x = 0;
    int y = 0;
    
    int gw = ceil(imageSize.width / simplify.x);
    int w = gw*simplify.x;
    int h = imageSize.height;
    
	for (float ystep = 0; ystep < h-simplify.y; ystep += simplify.y){
		for (float xstep = 0; xstep < w-simplify.x; xstep += simplify.x){
			ofIndexType a,b,c;
			c = x+y*gw;
			b = (x+1)+y*gw;
			a = x+(y+1)*gw;
            baseIndeces.push_back(a);
            baseIndeces.push_back(b);
            baseIndeces.push_back(c);
            
			c = (x+1)+(y+1)*gw;
			b = x+(y+1)*gw;
			a = (x+1)+(y)*gw;
            baseIndeces.push_back(a);
            baseIndeces.push_back(b);
            baseIndeces.push_back(c);
            x++;
		}
        
        y++;
        x = 0;
	}
    
	mesh.clearVertices();
	for (float y = 0; y < imageSize.height; y += simplify.y){
		for (float x = 0; x < imageSize.width; x += simplify.x){
			mesh.addVertex(ofVec3f(x,y,0));
		}
	}
    
    if(addColors){
        mesh.clearColors();
        for (float y = 0; y < imageSize.height; y += simplify.y){
            for (float x = 0; x < imageSize.width; x += simplify.x){
                mesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
            }
        }        
    }
    
    if(calculateTextureCoordinates){
        mesh.clearTexCoords();
        for (float y = 0; y < imageSize.height; y+=simplify.y){
            for (float x = 0; x < imageSize.width; x+=simplify.x){
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
    
    
	holeFiller.close(depthImage);
		
    //feed the zed values into the mesh
    int vertexIndex = 0;
    hasTriangles = false;
	validVertIndices.clear();
    unsigned short* ptr = depthImage.getPixels();    
    for(float y = 0; y < imageSize.height; y += simplify.y) {
        for(float x = 0; x < imageSize.width; x += simplify.x) {
			
			ofVec3f point;
			unsigned short z = ptr[int(y)*imageSize.width+int(x)];
			if(x >= imageSize.width*leftClip &&
			   x <= imageSize.width*rightClip &&
			   y >= imageSize.height*topClip &&
			   y <= imageSize.height*bottomClip &&
			   z >= nearClip &&
			   z <= farClip)
			{
				point = getWorldPoint(x,y,z);
				if(cacheValidVertices){
					validVertIndices.push_back(vertexIndex);
				}
			}
			else {
				point = ofVec3f(0,0,0);
			}
			mesh.setVertex(vertexIndex++, point);
        }
    }
    
    mesh.clearIndices();
    for(int i = 0; i < baseIndeces.size(); i+=3){

        ofVec3f& a = mesh.getVertices()[baseIndeces[i]];
        if(a.z == 0) continue;
        ofVec3f& b = mesh.getVertices()[baseIndeces[i+1]];
        if(b.z == 0) continue;
        ofVec3f& c = mesh.getVertices()[baseIndeces[i+2]];
        if(c.z == 0) continue;
        
        if(fabs(a.z - b.z) > edgeClip || fabs(a.z - c.z) > edgeClip){
            continue;
        }

        mesh.addTriangle(baseIndeces[i], baseIndeces[i+1], baseIndeces[i+2]);
        hasTriangles = true;
    }

	//cout << "has triangles? " << mesh.getNumIndices() << endl;
	if(calculateTextureCoordinates && !depthOnly){
        generateTextureCoordinates();
	}
    
    //cout << "updated mesh has " << mesh.getNumIndices()/3 << " triangles " << endl;
}

ofMesh& ofxRGBDMeshBuilder::getMesh(){
    return mesh;
}

ofMesh ofxRGBDMeshBuilder::getReducedMesh(bool normalizeTextureCoords, ofVec3f vertexScale, bool flipTextureX, bool flipTextureY){
    if(!cacheValidVertices){
        ofLogError("ofxRGBDMeshBuilder::getReducedMesh -- Must cache valid verts to get the reduced mesh");
    }
    ofMesh reducedMesh;
    map<ofIndexType, ofIndexType> vertMapping;
    for(int i = 0; i < validVertIndices.size(); i++){
        vertMapping[ validVertIndices[i] ] = i;
        reducedMesh.addVertex( mesh.getVertices()[ validVertIndices[i] ] * vertexScale);
		if(mesh.hasTexCoords() && calculateTextureCoordinates && currentTexture != NULL){
            ofVec2f& coord = mesh.getTexCoords()[ validVertIndices[i] ] ;
            if(flipTextureX){
                coord.x = currentTexture->getTextureReference().getWidth() - coord.x;
            }
            if(flipTextureY){
                coord.y = currentTexture->getTextureReference().getHeight() - coord.y;
            }
            
            if(normalizeTextureCoords){
				coord /= ofVec2f(currentTexture->getTextureReference().getWidth(),
                                 currentTexture->getTextureReference().getHeight() );
            }
            
            reducedMesh.addTexCoord(coord);
            
        }
        if(mesh.hasColors()){
            reducedMesh.addColor( mesh.getColors()[ validVertIndices[i] ] );
        }
    }
    
    for(int i = 0; i < mesh.getNumIndices(); i++){
        reducedMesh.addIndex( vertMapping[ mesh.getIndex(i) ] );
    }
    return reducedMesh;
}

void ofxRGBDMeshBuilder::generateTextureCoordinates(){
	generateTextureCoordinates(mesh.getVertices(), mesh.getTexCoords());
}

ofVec2f ofxRGBDMeshBuilder::getTextureCoordinateForPoint(ofVec3f point){
	vector<ofVec3f> points;
	vector<ofVec2f> tex;
	points.push_back(point);
	generateTextureCoordinates(points, tex);
	return tex[0];
}

void ofxRGBDMeshBuilder::generateTextureCoordinates(vector<ofVec3f>& points, vector<ofVec2f>& texCoords){
    if(!calibrationSetup){
        ofLogError("ofxRGBDRenderer::generateTextureCoordinates -- no calibration set up");
        return;
    }
    
	if(points.size() == 0){
        ofLogError("ofxRGBDRenderer::generateTextureCoordinates -- no points to generate");
        return;
	}

	texCoords.clear();
    Mat pcMat = Mat(toCv(points));
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
		texCd *= scale;
        texCd += shift;
		texCd.x = ofClamp(texCd.x,0.0,1.0);
		texCd.y = ofClamp(texCd.y,0.0,1.0);
        texCd *= ofVec2f(rgbImage.width,rgbImage.height) * textureScale;

		texCoords.push_back(texCd);
	}
}

ofVec3f ofxRGBDMeshBuilder::getWorldPoint(float x, float y){
	if(currentDepthPixels == NULL) return ofVec3f(0,0,0);
	return getWorldPoint(x,y,*currentDepthPixels);
}

ofVec3f ofxRGBDMeshBuilder::getWorldPoint(float x, float y, ofShortPixels& pixels){
	unsigned short z =  pixels.getPixels()[ pixels.getPixelIndex(x, y) ];
    return getWorldPoint(x,y,z);
}

ofVec3f ofxRGBDMeshBuilder::getWorldPoint(float x, float y, unsigned short z){
	return ofVec3f( (x - principalPoint.x) * z / fov.x, (y - principalPoint.y) * z / fov.y, z);
}

void ofxRGBDMeshBuilder::updateCenter(){
	center = ofVec3f(0,0,0);
	int vertsAdded = 0;
	for(int i = 0; i < mesh.getVertices().size(); i++){
		if(mesh.getVertices()[i] != ofVec3f(0,0,0) && mesh.getVertices()[i].z < farClip){
			center += mesh.getVertices()[i];
			vertsAdded++;
		}
	}
	center /= vertsAdded;
	center.y *= -1;
}

void ofxRGBDMeshBuilder::setPivotToMeshCenter(){
	updateCenter();
	pivot = center;
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
	else {
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
	if(!mirror){
		ofScale(-1,1, 1);
	}
}

void ofxRGBDMeshBuilder::draw(ofBaseHasTexture& texture){
    if(!calibrationSetup){
        ofLogError("ofxRGBDMeshBuilder::draw -- Failed. Calibration is not set up");
        return;
    }
    if(!hasTriangles){
        ofLogError("ofxRGBDMeshBuilder::draw -- Failed. Mesh has no geometry");
        return;
    }
    if(depthOnly){
        ofLogError("ofxRGBDMeshBuilder::draw -- Failed. MeshBuilder is set to depth only");
        return;
    }
    
    ofPushMatrix();
	setupDrawMatrices();
    texture.getTextureReference().bind();
	mesh.draw();
    texture.getTextureReference().unbind();
    ofPopMatrix();
}

void ofxRGBDMeshBuilder::setTextureScaleForImage(ofBaseHasTexture& texture){
	if(!calibrationSetup){
		ofLogError("ofxRGBDMeshBuilder::setTextureScaleForImage") << "must set up matrices before setting texture scale";
		return;
	}
	
    cv::Size rgbImage = rgbCalibration.getDistortedIntrinsics().getImageSize();
    textureScale = ofVec2f(float(texture.getTextureReference().getWidth() / float(rgbImage.width)),
                           float(texture.getTextureReference().getHeight()) / float(rgbImage.height) );    
}

void ofxRGBDMeshBuilder::setXYShift(ofVec2f newShift){
    shift = newShift;
}
