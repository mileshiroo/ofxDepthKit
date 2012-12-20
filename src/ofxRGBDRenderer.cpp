/*
 *  ofxRGBDRenderer.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 12/17/11.
 *  Copyright 2011 FlightPhase. All rights reserved.
 *
 */

#include "ofxRGBDRenderer.h"


ofxRGBDRenderer::ofxRGBDRenderer(){
	xshift = 0;
	yshift = 0;
	xscale = 1.0;
    yscale = 1.0;
    flipTexture = false;

	edgeClip = 50;
	simplify = ofVec2f(0,0);

	farClip = 6000;
    meshRotate = ofVec3f(0,0,0);
    
	hasDepthImage = false;
	hasRGBImage = false;

    rendererBound = false;
    currentlyBoundShader = NULL;

	mirror = false;
	calibrationSetup = false;
    
    reloadShader();
    setSimplification(ofVec2f(1,1));

    forceUndistortOff = false;
    addColors = false;
    currentDepthImage = NULL;
    useTexture = true;
}

ofxRGBDRenderer::~ofxRGBDRenderer(){

}

bool ofxRGBDRenderer::setup(string calibrationDirectory){
	
	if(!ofDirectory(calibrationDirectory).exists()){
		ofLogError("ofxRGBDRenderer --- Calibration directory doesn't exist: " + calibrationDirectory);
		return false;
	}
	return setup(calibrationDirectory+"/rgbCalib.yml", calibrationDirectory+"/depthCalib.yml",
		  		calibrationDirectory+"/rotationDepthToRGB.yml", calibrationDirectory+"/translationDepthToRGB.yml");
	
}

bool ofxRGBDRenderer::setup(string rgbIntrinsicsPath, string depthIntrinsicsPath, string rotationPath, string translationPath){
//	rgbCalibration.setFillFrame(false);
//	depthCalibration.setFillFrame(false);
	depthCalibration.load(depthIntrinsicsPath);
	rgbCalibration.load(rgbIntrinsicsPath);
	
	loadMat(rotationDepthToRGB, rotationPath);
	loadMat(translationDepthToRGB, translationPath);
    
    depthToRGBView = ofxCv::makeMatrix(rotationDepthToRGB, translationDepthToRGB);
	
    ofPushView();
//    rgbCalibration.getUndistortedIntrinsics().loadProjectionMatrix();
	rgbCalibration.getDistortedIntrinsics().loadProjectionMatrix();
    glGetFloatv(GL_PROJECTION_MATRIX, rgbProjection.getPtr());
    ofPopView();
	
	ofPushView();
	depthCalibration.getDistortedIntrinsics().loadProjectionMatrix();
	glGetFloatv(GL_PROJECTION_MATRIX, depthProjection.getPtr());
	ofPopView();
	
	rgbMatrix = (depthToRGBView * rgbProjection);
	
//	Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
//	fx = tanf(ofDegToRad(fov.x) / 2) * 2;
//	fy = tanf(ofDegToRad(fov.y) / 2) * 2;
//	fx = depthCalibration.getUndistortedIntrinsics().getCameraMatrix().at<double>(0,0);
//	fy = depthCalibration.getUndistortedIntrinsics().getCameraMatrix().at<double>(1,1);
//	principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
//	imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
	
	forceUndistortOff = true;
	fx = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(0,0);
	fy = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(1,1);
	principalPoint = depthCalibration.getDistortedIntrinsics().getPrincipalPoint();
	imageSize = depthCalibration.getDistortedIntrinsics().getImageSize();
    

//  cout << "successfully loaded calibration: fx + fy is " << fx << " " << fy  << endl;
//	cout << "RGB Camera Matrix is " << rgbCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
//	cout << "RGB Distortion coefficients " << rgbCalibration.getDistCoeffs() << endl;
//	cout << "Depth Camera Matrix is " << depthCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
//	cout << "Depth Distortion coefficients " << depthCalibration.getDistCoeffs() << endl;
//	cout << "RGB->Depth rotation " << rotationDepthToRGB << endl;
//	cout << "RGB->Depth translation " << translationDepthToRGB << endl;
//	cout << "RGB Aspect Ratio " << rgbCalibration.getDistortedIntrinsics().getAspectRatio() << endl;
//	cout << "RGB Focal Length " << rgbCalibration.getDistortedIntrinsics().getFocalLength() << endl;

	ofTextureData texData;
	texData.width = 640;
	texData.height = 480;
	texData.glType = GL_LUMINANCE;
    texData.glTypeInternal = GL_LUMINANCE16;
//    texData.glType = GL_RGB;
	texData.pixelType = GL_UNSIGNED_SHORT;

	depthTexture.allocate(texData);
    depthTexture.bind();
    GLint internalFormat;
    glGetTexLevelParameteriv(texData.textureTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    
    depthTexture.unbind();
	cout << " is depth texture allocated? " << (depthTexture.bAllocated() ? "yes" : "no") << " internal format? " << internalFormat << " vs " << GL_LUMINANCE16 << endl;
    calibrationSetup = true;
	return true;
}

void ofxRGBDRenderer::setSimplification(float simplification){
    setSimplification(ofVec2f(simplification,simplification));
}

void ofxRGBDRenderer::setSimplification(ofVec2f simplification){
    
    if(!calibrationSetup){
    	return;
    }

    if(simplify == simplification){
        return;
    }
    
	simplify = simplification;
    simplify.x = ofClamp(simplify.x, 0, 8);
    simplify.y = ofClamp(simplify.y, 0, 8);
	
    mesh.clearIndices();
    int x = 0;
    int y = 0;
    

    
//    int w = imageSize.width;
    int gw = ceil(imageSize.width / simplify.x);
    int w = gw*simplify.x;
    int h = imageSize.height;
    
	for (float ystep = 0; ystep < h-simplify.y; ystep += simplify.y){
		for (float xstep = 0; xstep < w-simplify.x; xstep += simplify.x){
			ofIndexType a,b,c;
            
			a = x+y*gw;
			b = (x+1)+y*gw;
			c = x+(y+1)*gw;
            mesh.addIndex(a);
            mesh.addIndex(b);
            mesh.addIndex(c);

			a = (x+1)+(y+1)*gw;
			b = x+(y+1)*gw;
			c = (x+1)+(y)*gw;
            mesh.addIndex(a);
            mesh.addIndex(b);
            mesh.addIndex(c);
            
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
}

//-----------------------------------------------
ofVec2f ofxRGBDRenderer::getSimplification(){
	return simplify;
}

//-----------------------------------------------
void ofxRGBDRenderer::setRGBTexture(ofBaseHasTexture& tex){

	currentRGBImage = &tex;
	hasRGBImage = true;
}

ofBaseHasTexture& ofxRGBDRenderer::getRGBTexture() {
    return *currentRGBImage;
}

void ofxRGBDRenderer::setDepthImage(ofShortPixels& pix){
    currentDepthImage = &pix;
	hasDepthImage = true;
}

Calibration& ofxRGBDRenderer::getDepthCalibration(){
	return depthCalibration;
}

Calibration& ofxRGBDRenderer::getRGBCalibration(){
	return rgbCalibration;
}

ofMatrix4x4& ofxRGBDRenderer::getRGBMatrix(){
	return rgbMatrix;
}

ofMatrix4x4& ofxRGBDRenderer::getDepthToRGBTransform(){
	return depthToRGBView;
}

ofTexture& ofxRGBDRenderer::getDepthTexture(){
    return depthTexture;
}

void ofxRGBDRenderer::update(){
    
	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return;
    }

    if(!calibrationSetup && hasRGBImage){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration for RGB Image");
        return;
    }
    
    if(simplify ==  ofVec2f(0,0)){
        setSimplification(ofVec2f(1,1));
    }
    
	depthTexture.loadData(*currentDepthImage);
}

ofVboMesh& ofxRGBDRenderer::getMesh(){
	return mesh;
}

void ofxRGBDRenderer::setXYShift(ofVec2f shift){
    xshift = shift.x;
    yshift = shift.y;
}

void ofxRGBDRenderer::setXYScale(ofVec2f scale){
    xscale = scale.x;
    yscale = scale.y;
}

void ofxRGBDRenderer::reloadShader(){
    meshShader.load("shaders/unproject");
    pointShader.load("shaders/unproject");
}

void ofxRGBDRenderer::drawProjectionDebug(bool showDepth, bool showRGB, float rgbTexturePosition){
    ofPushStyle();
	glEnable(GL_DEPTH_TEST);
	if(showRGB){
		ofPushMatrix();
		ofSetColor(255);
		rgbMatrix = (depthToRGBView * rgbProjection);
		ofScale(1,-1,1);
		glMultMatrixf(rgbMatrix.getInverse().getPtr());
		
		ofNoFill();
		ofSetColor(255,200,10);
		ofBox(1.99f);
		
		//draw texture
		if(rgbTexturePosition > 0){
			ofSetColor(255);
			ofTranslate(0, 0, 1.0 - powf(1-rgbTexturePosition, 2.0));
			undistortedRGBImage.draw(1, 1, -2, -2);
		}
		ofPopMatrix();
	}
	
	if(showDepth){
		ofPushMatrix();
		ofScale(-1,1,-1);
		ofNoFill();
		ofSetColor(10,200,255);
		glMultMatrixf(depthProjection.getInverse().getPtr());
		ofBox(1.99f);
		ofPopMatrix();
	}
	
	glDisable(GL_DEPTH_TEST);
    ofPopStyle();
}

bool ofxRGBDRenderer::bindRenderer(ofShader& shader){

	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return false;
    }
    
    if(!calibrationSetup){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration");
        return false;
    }
	
    ofPushMatrix();
    
    ofScale(1, -1, 1);
    if(!mirror){
	    ofScale(-1, 1, 1);    
    }
    
    ofRotate(meshRotate.x,1,0,0);
    ofRotate(meshRotate.y,0,1,0);
    ofRotate(meshRotate.z,0,0,1);

	if(hasRGBImage){
        shader.begin();
        glActiveTexture(GL_TEXTURE1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glActiveTexture(GL_TEXTURE0);


        setupProjectionUniforms(shader);
        currentlyBoundShader = &shader;
	}
    
    rendererBound = true;
    return true;
}


void ofxRGBDRenderer::unbindRenderer(){
    
    if(!rendererBound){
        ofLogError("ofxRGBDRenderer::unbindRenderer -- called without renderer bound");
     	return;   
    }
    
    if(rendererBound && hasRGBImage){
	
        if(currentlyBoundShader != NULL){
            restortProjection();
            currentlyBoundShader->end();
			currentlyBoundShader = NULL;
        }
        else {
            ofLogError("ofxRGBDRenderer::unbindRenderer -- with no reference to a shader");
        }
	}
    
	ofPopMatrix();
    rendererBound = false;
}

void ofxRGBDRenderer::setupProjectionUniforms(ofShader& theShader){

    rgbMatrix = (depthToRGBView * rgbProjection);
    
    ofVec2f dims = ofVec2f(currentRGBImage->getTextureReference().getWidth(),
                           currentRGBImage->getTextureReference().getHeight());

	theShader.setUniformTexture("colorTex", currentRGBImage->getTextureReference(), 0);
	theShader.setUniformTexture("depthTex", depthTexture, 1);
    theShader.setUniform1i("useTexture", useTexture ? 1 : 0);
    theShader.setUniform1i("flipTexture", flipTexture ? 1 : 0);
    theShader.setUniform2f("shift", xshift, yshift);
    theShader.setUniform2f("scale", xscale, yscale);
    theShader.setUniform2f("dim", dims.x, dims.y);
    theShader.setUniform2f("principalPoint", principalPoint.x, principalPoint.y);
    theShader.setUniform2f("fov", fx, fy);
    theShader.setUniform1f("farClip", farClip);
	theShader.setUniform1f("edgeClip", edgeClip);

    theShader.setUniform1f("xsimplify", simplify.x);
    theShader.setUniform1f("ysimplify", simplify.y);
    
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadMatrixf(rgbMatrix.getPtr());
    glMatrixMode(GL_MODELVIEW);      
    
}

void ofxRGBDRenderer::restortProjection(){

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);   

}

void ofxRGBDRenderer::drawMesh(){
    drawMesh(meshShader);
}

void ofxRGBDRenderer::drawPointCloud(){
    drawPointCloud(pointShader);
}

void ofxRGBDRenderer::drawWireFrame(){
    drawWireFrame(meshShader);
}

void ofxRGBDRenderer::drawMesh(ofShader& shader){
    
    if(bindRenderer(shader)){
		mesh.drawFaces();
        unbindRenderer();
    }
}

void ofxRGBDRenderer::drawPointCloud(ofShader& shader){

    if(bindRenderer(shader)){
	    mesh.drawVertices();
        unbindRenderer();
    }
}

void ofxRGBDRenderer::drawWireFrame(ofShader& shader){

	if(bindRenderer(shader)){
		mesh.drawWireframe();
        unbindRenderer();
    }
}
