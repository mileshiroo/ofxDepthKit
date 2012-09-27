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
    
	edgeCull = 4000;
	simplify = -1;

	farClip = 6000;
    meshRotate = ofVec3f(0,0,0);
    
	hasDepthImage = false;
	hasRGBImage = false;

    rendererBound = false;
    currentlyBoundShader = NULL;

	mirror = false;
	calibrationSetup = false;
    
    reloadShader();
    setSimplification(1);

    forceUndistortOff = false;
    addColors = false;
    currentDepthImage = NULL;
    
    simplify = -1;
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

    calibrationSetup = true;
	return true;
}

void ofxRGBDRenderer::setSimplification(int level){
    
    if(simplify == level){
        return;
    }
    
    if(!calibrationSetup){
    	return;    
    }
    
	simplify = level;
	if (simplify <= 0) {
		simplify = 1;
	}
	else if(simplify > 8){
		simplify = 8;
	}
	
    mesh.clearIndices();
	int w = imageSize.width / simplify;
	int h = imageSize.height / simplify;
	for (int y = 0; y < h-1; y++){
		for (int x=0; x < w-1; x++){
			ofIndexType a,b,c;
			a = x+y*w;
			b = x+(y+1)*w;
			c = (x+1)+y*w;
            mesh.addIndex(a);
            mesh.addIndex(b);
            mesh.addIndex(c);
            
			a = (x+1)+y*w;
			b = x+(y+1)*w;
			c = (x+1)+(y+1)*w;
            
            mesh.addIndex(a);
            mesh.addIndex(b);
            mesh.addIndex(c);
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
}

//-----------------------------------------------
int ofxRGBDRenderer::getSimplification(){
	return simplify;
}

void ofxRGBDRenderer::setRGBTexture(ofPtr<ofBaseHasPixels> pix){
    setRGBTexture(*pix.get());
}

//-----------------------------------------------
void ofxRGBDRenderer::setRGBTexture(ofBaseHasPixels& pix) {
	currentRGBImage = &pix;
    if(!undistortedRGBImage.isAllocated() || 
       pix.getPixelsRef().getWidth() != undistortedRGBImage.getWidth() || 
       pix.getPixelsRef().getHeight() != undistortedRGBImage.getHeight()){
        undistortedRGBImage.setFromPixels(pix.getPixelsRef());
    }
	hasRGBImage = true;
}

ofBaseHasPixels& ofxRGBDRenderer::getRGBTexture() {
    return *currentRGBImage;
}

void ofxRGBDRenderer::setDepthImage(ofShortPixels& pix){
    currentDepthImage = &pix;
	if(!undistortedDepthImage.isAllocated()){
		undistortedDepthImage.allocate(imageSize.width,imageSize.height,OF_IMAGE_GRAYSCALE);
	}
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

void ofxRGBDRenderer::update(){
    
	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return;
    }

    if(!calibrationSetup && hasRGBImage){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration for RGB Image");
        return;
    }
    
    if(simplify == -1){
        setSimplification(1);
    }
    
	//undistort the current images
    undistortImages();
    //feed the zed values into the mesh
    unsigned short* ptr = undistortedDepthImage.getPixels();    
    int vertexIndex = 0;

    for (int y = 0; y < imageSize.height; y+=simplify){
		for (int x = 0; x < imageSize.width; x+=simplify){
            mesh.getVertices()[vertexIndex++].z = ptr[y*imageSize.width+x];
        }
    }
}

void ofxRGBDRenderer::undistortImages(){

    if(!forceUndistortOff){
        depthCalibration.undistort( toCv(*currentDepthImage), toCv(undistortedDepthImage), CV_INTER_NN);
        rgbCalibration.undistort( toCv(*currentRGBImage), toCv(undistortedRGBImage) );
        undistortedRGBImage.update();
    }
    else {
        undistortedDepthImage = *currentDepthImage;
		undistortedRGBImage.setFromPixels(currentRGBImage->getPixelsRef());
    }
//    undistortedRGBImage.setFromPixels(currentRGBImage->getPixelsRef());
//    if(!forceUndistortOff){
//        depthCalibration.undistort( toCv(*currentDepthImage), toCv(undistortedDepthImage), CV_INTER_NN);
//    }
//    else {
//        undistortedDepthImage = *currentDepthImage;
//    }
//    undistortedRGBImage.setFromPixels(currentRGBImage->getPixelsRef());
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

void ofxRGBDRenderer::drawProjectionDebug(){
    ofPushStyle();
    ofPushMatrix();
    ofSetColor(255);
    rgbMatrix = (depthToRGBView * rgbProjection);
    glMultMatrixf(rgbMatrix.getInverse().getPtr());
	ofNoFill();
    ofBox(200.0f);
    ofPopMatrix();
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
	
//    if(ofGetKeyPressed('v'))
//        cout << "view " <<depthToRGBView << endl;
//    else if (ofGetKeyPressed('p'))
//        cout << "projection " << rgbProjection << endl;
	//cout << rgbMatrix << endl;

	if(hasRGBImage){
        undistortedRGBImage.getTextureReference().bind();

        shader.begin();	
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
    
    if(hasRGBImage){
        undistortedRGBImage.getTextureReference().unbind();
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
    //rgbMatrix.scale(1, -1, 1);
    
//    Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
//	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();

    ofVec2f dims = ofVec2f(undistortedRGBImage.getTextureReference().getWidth(), 
                           undistortedRGBImage.getTextureReference().getHeight());
    
    
    theShader.setUniform2f("shift", xshift, yshift);
    theShader.setUniform2f("scale", xscale, yscale);
    theShader.setUniform2f("dim", dims.x, dims.y);
    theShader.setUniform2f("principalPoint", principalPoint.x, principalPoint.y);
    theShader.setUniform2f("fov", fx, fy);
    theShader.setUniform2f("imageSize", imageSize.width,imageSize.height);
    theShader.setUniform1f("farClip", farClip);
    theShader.setUniform1i("project", 1);
	
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
