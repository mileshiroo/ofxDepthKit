/**
 * ofxRGBDepth addon
 *
 * Core addon for programming against the RGBDToolkit API
 * http://www.rgbdtoolkit.com
 *
 * (c) James George 2011-2013 http://www.jamesgeorge.org
 *
 * Developed with support from:
 *	  Frank-Ratchy STUDIO for Creative Inquiry http://studioforcreativeinquiry.org/
 *	  YCAM InterLab http://interlab.ycam.jp/en
 *	  Eyebeam http://eyebeam.org
 */

#include "ofxRGBDRenderer.h"

using namespace ofxCv;
using namespace cv;

ofxRGBDRenderer::ofxRGBDRenderer(){

	flipTexture = false;
	
	shift.x = 0;
	shift.y = 0;
	scale.x = 1.0;
	scale.y = 1.0;
	
	topClip = 0.0;
	bottomClip = 1.0;
	leftClip = 0.0;
	rightClip = 1.0;
	
	nearClip = 1;
	edgeClip = 50;
	farClip = 6000;
	
	simplify = ofVec2f(0,0);
	
	//meshRotate = ofVec3f(0,0,0);
	textureScale = ofVec2f(1.0, 1.0);
	
	hasDepthImage = false;
	hasRGBImage = false;
	
	mirror = false;
	calibrationSetup = false;
	
	addColors = false;
	currentDepthImage = NULL;
	useTexture = true;
	meshGenerated = false;
	
	worldPosition = ofVec3f(0,0,0);
	worldRotation = ofVec3f(0,0,0);
}

ofxRGBDRenderer::~ofxRGBDRenderer(){
	
}

void ofxRGBDRenderer::setDepthOnly(){
	
	//default kinect intrinsics
	depthFOV.x = 5.7034220279543524e+02;
	depthFOV.y = 5.7034220280129011e+02;
	depthPrincipalPoint.x = 320;
	depthPrincipalPoint.y = 240;
	depthImageSize.width = 640;
	depthImageSize.height = 480;
	
	depthOnly = true;
	if(!meshGenerated){
		setSimplification(ofVec2f(1,1));
		meshGenerated = true;
	}	
}

void ofxRGBDRenderer::setDepthOnly(string depthCalibrationPath){
	
	//NO LONGER USED ---
	ofPushView();
	depthCalibration.load(depthCalibrationPath);
	depthCalibration.getDistortedIntrinsics().loadProjectionMatrix();
	glGetFloatv(GL_PROJECTION_MATRIX, depthProjection.getPtr());
	ofPopView();
	///------------------

	depthFOV.x = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(0,0);
	depthFOV.y = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(1,1);
	depthPrincipalPoint = toOf(depthCalibration.getDistortedIntrinsics().getPrincipalPoint());
	depthImageSize = ofRectangle(0, 0,
								 depthCalibration.getDistortedIntrinsics().getImageSize().width,
								 depthCalibration.getDistortedIntrinsics().getImageSize().height);
	depthOnly = true;
	if(!meshGenerated){
		setSimplification(ofVec2f(1,1));
		meshGenerated = true;
	}
	
}


bool ofxRGBDRenderer::setup(string calibrationDirectory){
	
	if(!ofDirectory(calibrationDirectory).exists()){
		ofLogError("ofxRGBDGPURenderer --- Calibration directory doesn't exist: " + calibrationDirectory);
		return false;
	}
	return setup(calibrationDirectory+"rgbCalib.yml", calibrationDirectory+"depthCalib.yml",
				 calibrationDirectory+"rotationDepthToRGB.yml", calibrationDirectory+"translationDepthToRGB.yml");
}

bool ofxRGBDRenderer::setup(string rgbIntrinsicsPath,
							string depthIntrinsicsPath, string rotationPath, string translationPath)
{
	if(!ofFile::doesFileExist(rgbIntrinsicsPath) ||
	   !ofFile::doesFileExist(depthIntrinsicsPath) ||
	   !ofFile::doesFileExist(rotationPath) ||
	   !ofFile::doesFileExist(translationPath))
	{
		ofLogError() << "ofxRGBDRenderer::setup -- Missing one or more matrix files! " << rgbIntrinsicsPath << " " <<
			depthIntrinsicsPath << " " <<
			rotationPath << " " <<
			translationPath;
		return false;
	}
	
	depthCalibration.load(depthIntrinsicsPath);
	rgbCalibration.load(rgbIntrinsicsPath);

	loadMat(rotationDepthToRGB, rotationPath);
	loadMat(translationDepthToRGB, translationPath);

	//NO LONGER USED------
	depthToRGBView = ofxCv::makeMatrix(rotationDepthToRGB, translationDepthToRGB);

	ofPushView();
	rgbCalibration.getDistortedIntrinsics().loadProjectionMatrix();
	glGetFloatv(GL_PROJECTION_MATRIX, rgbProjection.getPtr());
	ofPopView();

	ofPushView();
	depthCalibration.getDistortedIntrinsics().loadProjectionMatrix();
	glGetFloatv(GL_PROJECTION_MATRIX, depthProjection.getPtr());
	ofPopView();
	rgbMatrix = (depthToRGBView * rgbProjection);
	///-------
	
	//	Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
	//	fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	//	fy = tanf(ofDegToRad(fov.y) / 2) * 2;
	//	fx = depthCalibration.getUndistortedIntrinsics().getCameraMatrix().at<double>(0,0);
	//	fy = depthCalibration.getUndistortedIntrinsics().getCameraMatrix().at<double>(1,1);
	//	principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	//	imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();

	//intrinsics
	depthFOV.x = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(0,0);
	depthFOV.y = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(1,1);
	depthPrincipalPoint = toOf(depthCalibration.getDistortedIntrinsics().getPrincipalPoint());
	depthImageSize = ofRectangle(0, 0,
								 depthCalibration.getDistortedIntrinsics().getImageSize().width,
								 depthCalibration.getDistortedIntrinsics().getImageSize().height);

	colorFOV.x = rgbCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(0,0);
	colorFOV.y = rgbCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(1,1);
	colorPrincipalPoint = toOf( rgbCalibration.getDistortedIntrinsics().getPrincipalPoint() );
	colorImageSize = ofRectangle(0,0,
								 rgbCalibration.getDistortedIntrinsics().getImageSize().width,
								 rgbCalibration.getDistortedIntrinsics().getImageSize().height);
	
	//extrinsics
	depthToRGBTranslation = ofVec3f(translationDepthToRGB.at<double>(0,0),
									translationDepthToRGB.at<double>(1,0),
									translationDepthToRGB.at<double>(2,0));
	Mat rx3;

	if(rotationDepthToRGB.rows == 3 && rotationDepthToRGB.cols == 3) {
		cout << "LOADING 3x3 ROTATION!" << endl;
		rotationDepthToRGB.copyTo(rx3);
		float rotation3fv[9] = {
			float(rx3.at<double>(0,0)),float(rx3.at<double>(1,0)),float(rx3.at<double>(2,0)),
			float(rx3.at<double>(0,1)),float(rx3.at<double>(1,1)),float(rx3.at<double>(2,1)),
			float(rx3.at<double>(0,2)),float(rx3.at<double>(1,2)),float(rx3.at<double>(2,2))
		};
		memcpy(depthToRGBRotation, rotation3fv, sizeof(float)*3*3);
	}
	else {
		//openFrameworkds needs a better Matrix3x3 class...
		cv::Rodrigues(rotationDepthToRGB, rx3);
		float rotation3fv[9] = {
			float(rx3.at<double>(0,0)),float(rx3.at<double>(1,0)),float(rx3.at<double>(2,0)),
			float(rx3.at<double>(0,1)),float(rx3.at<double>(1,1)),float(rx3.at<double>(2,1)),
			float(rx3.at<double>(0,2)),float(rx3.at<double>(1,2)),float(rx3.at<double>(2,2))
		};
		memcpy(depthToRGBRotation, rotation3fv, sizeof(float)*3*3);
	}
	
	
	float mat4x4[16] = {
		depthToRGBRotation[0],depthToRGBRotation[1],depthToRGBRotation[2],0,
		depthToRGBRotation[3],depthToRGBRotation[4],depthToRGBRotation[5],0,
		depthToRGBRotation[6],depthToRGBRotation[7],depthToRGBRotation[8],0,
		depthToRGBTranslation.x,depthToRGBTranslation.y,depthToRGBTranslation.z,1
	};
	
	extrinsics = ofMatrix4x4(mat4x4);

	//windows seems to load these differently sometimes
	Mat dis = rgbCalibration.getDistCoeffs();
	if(dis.cols == 1){
		distortionK = ofVec3f(dis.at<double>(0,0),
							  dis.at<double>(1,0),
							  dis.rows == 5 ? dis.at<double>(4,0) : 0);
		distortionP = ofVec2f(dis.at<double>(2,0),dis.at<double>(3,0));
	}
	else if(dis.rows == 1){
		distortionK = ofVec3f(dis.at<double>(0,0),
							  dis.at<double>(0,1),
							  dis.cols == 5 ? dis.at<double>(0,4) : 0);
		distortionP = ofVec2f(dis.at<double>(0,2),dis.at<double>(0,3));	
	}

	//distortionK = ofVec3f(dis.at<double>(0,0),
	//					  dis.at<double>(0,1),
	//					  dis.rows == 5 ? dis.at<double>(0,4) : 0);
	distortionP = ofVec2f(dis.at<double>(2,0),dis.at<double>(3,0));

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
	depthOnly = false;
	if(!meshGenerated){
		setSimplification(ofVec2f(1,1));
		meshGenerated = true;
	}
	
	return true;
}

ofMatrix4x4 ofxRGBDRenderer::getAdjustedMatrix(){
	
	ofMatrix4x4 modMat;
	modMat.rotate(colorMatrixRotate.x, 0, 1, 0);
	modMat.rotate(colorMatrixRotate.y, 1, 0, 0);
	modMat.translate(colorMatrixTranslate.x, colorMatrixTranslate.y, 0);

	return extrinsics * modMat;
}
//-----------------------------------------------
ofVec2f ofxRGBDRenderer::getSimplification(){
	return simplify;
}

//-----------------------------------------------
void ofxRGBDRenderer::drawMesh(){
	draw(OF_MESH_FILL);
}

//-----------------------------------------------
void ofxRGBDRenderer::drawPointCloud(){
	draw(OF_MESH_POINTS);
}

//-----------------------------------------------
void ofxRGBDRenderer::drawWireFrame(){
	draw(OF_MESH_WIREFRAME);
}

//-----------------------------------------------
void ofxRGBDRenderer::setDepthImage(ofShortPixels& pix){
	currentDepthImage = &pix;
	hasDepthImage = true;
}

//-----------------------------------------------
void ofxRGBDRenderer::setRGBTexture(ofBaseHasTexture& tex){
	currentRGBImage = &tex;
	setTextureScaleForImage(tex);
	hasRGBImage = true;
}

//-----------------------------------------------
void ofxRGBDRenderer::setTextureScaleForImage(ofBaseHasTexture& texture){
	if(calibrationSetup){
		cv::Size rgbImage = rgbCalibration.getDistortedIntrinsics().getImageSize();
		textureScale = ofVec2f(float(texture.getTextureReference().getWidth() / float(rgbImage.width)),
							   float(texture.getTextureReference().getHeight()) / float(rgbImage.height) );
	}
}

//-----------------------------------------------
ofBaseHasTexture& ofxRGBDRenderer::getRGBTexture() {
	return *currentRGBImage;
}

//-----------------------------------------------
ofShortPixels& ofxRGBDRenderer::getDepthImage(){
	return *currentDepthImage;
}

//-----------------------------------------------
void ofxRGBDRenderer::setXYShift(ofVec2f newShift){
	shift = newShift;
}

//-----------------------------------------------
void ofxRGBDRenderer::setXYScale(ofVec2f newScale){
	scale = newScale;	
}

ofMesh& ofxRGBDRenderer::getMesh(){
	return mesh;
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

void ofxRGBDRenderer::drawProjectionDebug(bool showDepth, bool showRGB, float rgbTexturePosition){
	ofPushStyle();
	glEnable(GL_DEPTH_TEST);
	if(showRGB){
		ofPushMatrix();
		ofSetColor(255);
		rgbMatrix = (depthToRGBView * rgbProjection);
		ofScale(1,-1,-1);
		glMultMatrixf(rgbMatrix.getInverse().getPtr());
		
		ofNoFill();
		ofSetColor(255,200,10);
		ofBox(1.99f);
		
		//draw texture
		if(rgbTexturePosition > 0){
			ofSetColor(255);
			ofTranslate(0, 0, 1.0 - powf(1-rgbTexturePosition, 2.0));
			currentRGBImage->getTextureReference().draw(1, -1, -2, 2);
		}
		ofPopMatrix();
	}
	
	if(showDepth){
		ofPushMatrix();
		ofScale(1,1,-1);
		ofNoFill();
		ofSetColor(10,200,255);
		glMultMatrixf(depthProjection.getInverse().getPtr());
		ofBox(1.99f);
		ofPopMatrix();
	}
	
	glDisable(GL_DEPTH_TEST);
	ofPopStyle();
}
