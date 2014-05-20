//
//  ofxRGBDCaptureGui.cpp
//  RGBDCaptureKinect
//
//  Created by James George on 2/26/13.
//
//

//TODO: store camera intrinsic names into the main application folder and allow them to be loaded
//TODO: Allow checkerboard corner dimensions to be set
//TODO: Display checkerboard error for intrinsics

#include "ofxRGBDCaptureGui.h"

ofxRGBDCaptureGui::ofxRGBDCaptureGui(){
	providerSet = false;
	calibrationGenerated = false;
	currentRendererPreviewIndex = 0;
	currentCalibrationImageIndex = 0;
	hoverPreviewDepth = false;
	hoverPreviewIR = false;
	hoverPreviewingCaptured = false;
	depthCameraSelfCalibrated = false;
	hasIncludedBoards = false;
	currentRenderModeObject = NULL;
	backpackMode = false;
}

ofxRGBDCaptureGui::~ofxRGBDCaptureGui(){
	
}

void ofxRGBDCaptureGui::setup(){
	ofEnableSmoothing();

	renderer = &gpuRenderer;
	
	framewidth = 640;
	frameheight = 480;
	thirdWidth = framewidth/3;
	btnheight = 36;
	margin = 12;

	contextHelpTextLarge.loadFont("GUI/NewMedia Fett.ttf", 23);
	contextHelpTextSmall.loadFont("GUI/NewMedia Fett.ttf", 12);

	currentAlignmentPair = new AlignmentPair();
	currentAlignmentPair->included = true;
	alignmentPairs.push_back(currentAlignmentPair);

	timeline.setup();
	timeline.setOffset(ofVec2f(0, frameheight+btnheight*3));
	timeline.addTrack("depth sequence", &depthSequence);
	timeline.setLoopType(OF_LOOP_NORMAL);
	timeline.setSpacebarTogglePlay(false);
	timeline.hide();
	
	//TODO: make timeline colors
	downColor  = ofColor(255, 120, 0);
	idleColor  = ofColor(220, 200, 200);
	hoverColor = ofColor(255*.2, 255*.2, 30*.2);

	confirmedColor = ofColor(100, 230, 120);
	warningColor   = ofColor(255, 200,  0);
	errorColor	 = ofColor(230, 100,  20);

	float totalFrameWidth = framewidth*2;
	float quarterWidth = totalFrameWidth * .25;
	
	btnSetDirectory = new ofxMSAInteractiveObjectWithDelegate();
	btnSetDirectory->setPosAndSize(0, 0, totalFrameWidth, btnheight);
	btnSetDirectory->setLabel("Load Directory");
	btnSetDirectory->fontReference = &contextHelpTextSmall;
	buttonSet.push_back(btnSetDirectory);
	
	btnIntrinsicsTab = new ofxMSAInteractiveObjectWithDelegate();
	btnIntrinsicsTab->setPosAndSize(0, btnheight, quarterWidth, btnheight);
	btnIntrinsicsTab->setLabel("Calibrate Lenses");
	btnIntrinsicsTab->fontReference = &contextHelpTextSmall;
	buttonSet.push_back(btnIntrinsicsTab);
	currentTabObject = btnIntrinsicsTab;
	currentTab = TabIntrinsics;
	tabSet.push_back(btnIntrinsicsTab);
	
	btnExtrinsicsTab = new ofxMSAInteractiveObjectWithDelegate();
	btnExtrinsicsTab->setPosAndSize(quarterWidth, btnheight, quarterWidth, btnheight);
	btnExtrinsicsTab->setLabel("Calibrate Correspondence");
	btnExtrinsicsTab->fontReference = &contextHelpTextSmall;
	buttonSet.push_back(btnExtrinsicsTab);
	tabSet.push_back(btnExtrinsicsTab);
	
	btnRecordTab = new ofxMSAInteractiveObjectWithDelegate();
	btnRecordTab->setPosAndSize(quarterWidth*2, btnheight, quarterWidth, btnheight);
	btnRecordTab->setLabel("Record");
	btnRecordTab->fontReference = &contextHelpTextSmall;
	buttonSet.push_back(btnRecordTab);
	tabSet.push_back(btnRecordTab);
	
	btnPlaybackTab = new ofxMSAInteractiveObjectWithDelegate();
	btnPlaybackTab->setPosAndSize(quarterWidth*3, btnheight, quarterWidth, btnheight);
	btnPlaybackTab->setLabel("Playback");
	btnPlaybackTab->fontReference = &contextHelpTextSmall;
	buttonSet.push_back(btnPlaybackTab);
	tabSet.push_back(btnPlaybackTab);
	
	
	btnRenderBW = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderBW->setPosAndSize(0, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderBW->setLabel("Black & White");
	btnRenderBW->fontReference = &contextHelpTextSmall;
	buttonSet.push_back(btnRenderBW);
	currentRenderModeObject = btnRenderBW;
	currentRenderMode = RenderBW;
	
	btnRenderRainbow = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderRainbow->setPosAndSize(thirdWidth, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderRainbow->fontReference = &contextHelpTextSmall;
	btnRenderRainbow->setLabel("Rainbow");
	buttonSet.push_back(btnRenderRainbow);
	
	btnRenderPointCloud = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderPointCloud->setPosAndSize(thirdWidth*2, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderPointCloud->fontReference = &contextHelpTextSmall;
	btnRenderPointCloud->setLabel("Pointcloud");
	buttonSet.push_back(btnRenderPointCloud);
	
	btnCalibrateDepthCamera =  new ofxMSAInteractiveObjectWithDelegate();
	btnCalibrateDepthCamera->setPosAndSize(0, btnheight*3+frameheight, totalFrameWidth/2, btnheight*2);
	btnCalibrateDepthCamera->setLabel("Self-Calibrate Depth Camera");
	btnCalibrateDepthCamera->fontReference = &contextHelpTextLarge;
	buttonSet.push_back(btnCalibrateDepthCamera);

	btnRGBLoadCalibration =  new ofxMSAInteractiveObjectWithDelegate();
	btnRGBLoadCalibration->setPosAndSize(totalFrameWidth/2, btnheight*3+frameheight, totalFrameWidth/2, btnheight*2);
	btnRGBLoadCalibration->setLabel("Load RGB Calibration Images");
	btnRGBLoadCalibration->fontReference = &contextHelpTextLarge;
	buttonSet.push_back(btnRGBLoadCalibration);
	
	//EXTRINSICS
	btnGenerateCalibration =  new ofxMSAInteractiveObjectWithDelegate();
	btnGenerateCalibration->setPosAndSize(0, btnheight*3+frameheight, totalFrameWidth, btnheight*2);
	btnGenerateCalibration->setLabel("Regenerate RGB/Depth Correspondence");
	btnGenerateCalibration->disableAllEvents();
	btnGenerateCalibration->fontReference = &contextHelpTextLarge;
	buttonSet.push_back(btnGenerateCalibration);
	
	//CAPTURE
	btnToggleRecord = new ofxMSAInteractiveObjectWithDelegate();
	btnToggleRecord->setPosAndSize(0, btnheight*3+frameheight, totalFrameWidth, btnheight*2);
	btnToggleRecord->setLabel("Toggle Record");
	btnToggleRecord->fontReference = &contextHelpTextLarge;
	btnToggleRecord->disableAllEvents();
	buttonSet.push_back(btnToggleRecord);

	for(int i = 0; i < buttonSet.size(); i++){
		buttonSet[i]->setIdleColor(idleColor);
		buttonSet[i]->setDownColor(downColor);
		buttonSet[i]->setHoverColor(hoverColor);
		buttonSet[i]->disableKeyEvents();
		buttonSet[i]->enableMouseEvents();
		buttonSet[i]->setDelegate(this);
	}
	
	previewRectLeft  = ofRectangle(0, btnheight*2, 640, 480);
	previewRectRight = previewRectLeft;
	previewRectRight.x += 640;
	
	ofRegisterMouseEvents(this);
	ofRegisterKeyEvents(this);
	
	ofAddListener(ofEvents().fileDragEvent, this, &ofxRGBDCaptureGui::dragEvent);
	ofAddListener(ofEvents().update, this, &ofxRGBDCaptureGui::update);
	ofAddListener(ofEvents().draw, this, &ofxRGBDCaptureGui::draw);
	ofAddListener(ofEvents().exit, this, &ofxRGBDCaptureGui::exit);
	
	depthSequence.setup();
	
	updateInterfaceForTab(btnIntrinsicsTab);
	updateSceneButtons();
	
	cam.setup();
	pointcloudPreviewCam.setup();

	createRainbowPallet();
	depthImage.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	
	recorder.setup();

	boardColors.push_back( ofColor(232,   0, 122) );
	boardColors.push_back( ofColor(0,   180, 255) );
	boardColors.push_back( ofColor(212, 255,   0) );
	boardColors.push_back( ofColor( 73, 240, 159) );
	
	calibrationPreview.setup(10, 7, squareSize);
	rgbCalibration.setPatternSize(10, 7);
	rgbCalibration.setSquareSize(squareSize);
	rgbCalibration.setSubpixelSize(11);
	
	//pointcloudPreview.setDepthOnly();
	
	checkerboardDimensions.setup();
	checkerboardDimensions.bounds = ofRectangle(contextHelpTextSmall.stringWidth("Square Size (cm)") + previewRectRight.x + 20,
												previewRectRight.getMaxY() + 1, 50,
												contextHelpTextSmall.getLineHeight() + 3);
	checkerboardDimensions.setFont(contextHelpTextSmall);
	ofAddListener(checkerboardDimensions.textChanged, this, &ofxRGBDCaptureGui::squareSizeChanged);
	
	ofxXmlSettings defaults;
	if(defaults.loadFile("defaultBin.xml")){
		loadDirectory(defaults.getValue("bin", ""));
	}
	else{
		loadDirectory("depthframes");
	}
	
	recordOn.loadSound("sound/record_start_short.aif");
	recordOff.loadSound("sound/record_stop.wav");
	
	ofSoundPlayer recordOff;
	
}

void ofxRGBDCaptureGui::setImageProvider(ofxDepthImageProvider* imageProvider){
	depthImageProvider = ofPtr<ofxDepthImageProvider>( imageProvider );
	depthImageProvider->setup();
	
	pointcloudPreview.setDepthImage(depthImageProvider->getRawDepth());
	pointcloudPreview.setDepthOnly();
	
	providerSet = true;
}


void ofxRGBDCaptureGui::update(ofEventArgs& args){
	
	if(currentTab == TabPlayback &&
	   depthSequence.getDepthImageSequence() != NULL &&
	   depthSequence.getDepthImageSequence()->isLoaded() &&
	   depthSequence.isFrameNew())
	{
		updateDepthImage(depthSequence.getDepthImageSequence()->getPixels());
	}
	
	if(!providerSet || !depthImageProvider->deviceFound()){
		return;
	}
	
	depthImageProvider->update();
	if(depthImageProvider->isFrameNew()){
		
		if(currentTab != TabPlayback){
			updateDepthImage(depthImageProvider->getRawDepth());
		}
				
		if(recorder.isRecording()){
			recorder.addImage(depthImageProvider->getRawDepth());
		}
		else if(currentTab == TabExtrinsics){
			calibrationPreview.setTestImage( depthImageProvider->getRawIRImage() );
		}
	}
	
	if(backpackMode && recorder.isRecording() && ofGetFrameNum() % 60 == 0){
		recordOn.setPosition(0);
		recordOn.play();
	}
	
	int framesWaiting = recorder.numFramesWaitingSave();
	//Draw recorder safety click
	if(backpackMode && framesWaiting > 750 && ofGetFrameNum() % 30 == 0){
		recordOff.setPosition(0);
		recordOff.play();
	}

}

void ofxRGBDCaptureGui::setupRenderer(){
	currentRendererPreviewIndex = MIN(currentRendererPreviewIndex, alignmentPairs.size()-1);
	
//	renderer.reloadShader();
//	renderer.setup("rgbCalibRefined.yml",
//				   "depthCalibRefined.yml", "rotationDepthToRGB.yml", "translationDepthToRGB.yml");
	
	cpuRenderer.setup(matrixDirectory);
	gpuRenderer.setup(matrixDirectory);
	
//	gpuRenderer.scale = ofVec2f(2.0, 2.0);
	gpuRenderer.setDepthImage(alignmentPairs[currentRendererPreviewIndex]->depthPixelsRaw);
	gpuRenderer.setRGBTexture(alignmentPairs[currentRendererPreviewIndex]->colorCheckers);
	gpuRenderer.update();
	
//	cpuRenderer.scale = ofVec2f(2.0, 2.0);
	cpuRenderer.setDepthImage(alignmentPairs[currentRendererPreviewIndex]->depthPixelsRaw);
	cpuRenderer.setRGBTexture(alignmentPairs[currentRendererPreviewIndex]->colorCheckers);
	cpuRenderer.update();

}
void ofxRGBDCaptureGui::draw(ofEventArgs& args){
	if(currentTab == TabIntrinsics){
		drawIntrinsics();
	}
	else if(currentTab == TabExtrinsics){
		drawExtrinsics();
	}
	else if(currentTab == TabCapture){
		drawCapture();
	}
	else if(currentTab == TabPlayback) {
		drawPlayback();
	}
	
	ofPushStyle();
	ofRectangle highlightRect = ofRectangle(currentTabObject->x,currentTabObject->y+currentTabObject->height*.75,
											currentTabObject->width,currentTabObject->height*.25);
	ofSetColor(timeline.getColors().highlightColor);
	ofRect(highlightRect);
	ofPopStyle();
	
	if(currentRenderModeObject != NULL && currentTab != TabExtrinsics){
		ofPushStyle();
		ofRectangle highlightRect = ofRectangle(currentRenderModeObject->x,currentRenderModeObject->y+currentRenderModeObject->height*.75,
												currentRenderModeObject->width,currentRenderModeObject->height*.25);
		ofSetColor(timeline.getColors().highlightColor);
		ofRect(highlightRect);
		ofPopStyle();
	}
	
	timeline.draw();

	if(backpackMode){
		ofSetColor(255,0,0,50);
		ofRect(0,0,ofGetWidth(),ofGetHeight());
		
		ofSetColor(255);
		contextHelpTextLarge.drawString("BACKPACK MODE",
										ofGetWidth()/2 - contextHelpTextLarge.stringWidth("BACKPACK MODE")/2,
										ofGetHeight()/2 - contextHelpTextLarge.stringHeight("BACKPACK MODE")/2);
		
	}
}

void ofxRGBDCaptureGui::drawIntrinsics(){
	//left side is Kinect
	bool drawCamera = providerSet && depthImageProvider->deviceFound();
	if(depthCameraSelfCalibrated){
		ofFill();
		ofSetColor(confirmedColor, 50);
		ofRect(*btnCalibrateDepthCamera);
		ofSetColor(255);
		contextHelpTextSmall.drawString("Field of View: " + ofToString(fov.x,2) + " x " + ofToString(fov.y,2) +
										"\nPrincipal Point: " +  ofToString(pp.x,2) + " x " + ofToString(pp.y,2),
										btnCalibrateDepthCamera->x+10,btnCalibrateDepthCamera->y+55);		
	}

	if(drawCamera){
//		ofPushStyle();
//		//DRAW CONFIRMATION RECTANGLE
//		ofPopStyle();
		drawDepthImage(previewRectLeft);
	}
	else{
		ofPushStyle();
		ofSetColor(255, 0, 0);
		contextHelpTextSmall.drawString("Depth sensor not found. Reconnect the device and restart the application.", previewRectLeft.x + 30, previewRectLeft.y + 30);
		ofPopStyle();
	}
	
	//right side RGB
   	if (rgbCalibrationImages.size() > 0) {
		ofPushStyle();
		if(previewRectRight.inside(ofGetMouseX(), ofGetMouseY())){
			currentCalibrationImageIndex = ofMap(ofGetMouseX(), previewRectRight.getMinX(), previewRectRight.getMaxX(), 0, rgbCalibrationImages.size(), true);
			currentCalibrationImageIndex = MIN(currentCalibrationImageIndex,rgbCalibrationImages.size()-1);
		}
		ofRectangle previewRectScaled = ofRectangle(0,0,
													rgbCalibrationImages[currentCalibrationImageIndex].getWidth(),
													rgbCalibrationImages[currentCalibrationImageIndex].getHeight());
		
		previewRectScaled.scaleTo(previewRectRight);
		rgbCalibrationImages[currentCalibrationImageIndex].draw(previewRectScaled);

		if(rgbCalibration.isReady()){
			drawCalibrationNumbers();
			float error = rgbCalibration.getReprojectionError(currentCalibrationImageIndex);
			string currentError = "Total Error: " + ofToString(rgbCalibration.getReprojectionError(),3) + " " + rgbCalibrationFileNames[currentCalibrationImageIndex] + " Error: " + ofToString(error, 3);
			ofSetColor(0);
			ofRect(previewRectRight.x, previewRectRight.getMaxY()-1, contextHelpTextSmall.stringWidth(currentError) + 20, -contextHelpTextSmall.getLineHeight()+10);
			ofSetColor(255);
			contextHelpTextSmall.drawString(currentError, previewRectRight.x+10, previewRectRight.getMaxY()-contextHelpTextSmall.getLineHeight()-5);
			
		}
		ofPopStyle();
	}
	else{
		ofPushStyle();
		string dragText = "Drag & Drop an RGB checkerboard folder or files";
		contextHelpTextSmall.drawString(dragText,
										previewRectRight.getCenter().x - contextHelpTextSmall.getStringBoundingBox(dragText, 0, 0).width/2,
										previewRectRight.y + contextHelpTextSmall.getLineHeight()*2);
		ofNoFill();
		ofRect(previewRectRight);
		ofPopStyle();
	}
	
	drawDimensionsEntry();
}


void ofxRGBDCaptureGui::drawExtrinsics(){
	
	bool drawCamera = providerSet && depthImageProvider->deviceFound();
	int drawX = previewRectLeft.x;
	int drawY = previewRectLeft.y;
	
//	timeline.getFont().drawString("Show the checkerboard to the depth sensor and click to create correspondences", drawX, drawY);
//	drawY += timeline.getFont().getLineHeight();
	
	//TODO draw image in whatever is hovering
	checkerboardDimensions.draw();
	if(drawCamera){
		if(!currentAlignmentPair->depthImage.isAllocated()){
			recorder.getCompressor().convertTo8BitImage( depthImageProvider->getRawDepth() ).draw(currentAlignmentPair->depthImageRect);
			calibrationPreview.draw(currentAlignmentPair->depthImageRect);
		}
		if(!currentAlignmentPair->depthCheckers.isAllocated()){
			depthImageProvider->getRawIRImage().draw(currentAlignmentPair->depthCheckersRect);
			calibrationPreview.draw(currentAlignmentPair->depthCheckersRect);
		}
	}
	for(int i = 0; i < MIN(alignmentPairs.size(), 4); i++){
		
		float subRectWidth  = framewidth*.25;
		float subRectHeight = frameheight*.25;
		
		//DRAW ACTUAL IMAGES
		if(alignmentPairs[i]->depthImage.isAllocated()){
			alignmentPairs[i]->depthImage.draw(alignmentPairs[i]->depthImageRect);
		}
		if(alignmentPairs[i]->depthCheckers.isAllocated()){
			alignmentPairs[i]->depthCheckers.draw(alignmentPairs[i]->depthCheckersRect);
		}
		
		if(alignmentPairs[i]->colorCheckers.isAllocated()){
			alignmentPairs[i]->colorCheckers.draw(alignmentPairs[i]->colorCheckersRect);
		}
		
		//DRAW DELETE
		ofPushStyle();
		ofSetColor(0);
		ofRect(alignmentPairs[i]->deleteRect);
		ofSetColor(255);		
		contextHelpTextSmall.drawString("[X]", alignmentPairs[i]->deleteRect.x + 5,
										alignmentPairs[i]->deleteRect.y + alignmentPairs[i]->deleteRect.height/2 + contextHelpTextSmall.getLineHeight() *.5);
		ofNoFill();
		ofRect(alignmentPairs[i]->deleteRect);
		ofPopStyle();
		
		//DRAW INCLUDE/IGNORE BUTTONS
		if(alignmentPairs[i] != currentAlignmentPair){
			
			ofPushStyle();
			ofSetColor(0);
			ofRect(alignmentPairs[i]->includeRect);
			ofSetColor(255);
			string includeString = alignmentPairs[i]->included ? "IGNORE" : "INCLUDE";
			contextHelpTextSmall.drawString(includeString, alignmentPairs[i]->includeRect.x + 5,
											alignmentPairs[i]->includeRect.y + + alignmentPairs[i]->includeRect.height/2 + contextHelpTextSmall.getLineHeight() * .5);
			ofSetColor(255);
			ofNoFill();
			ofRect(alignmentPairs[i]->includeRect);
			ofPopStyle();
		}
		//DRAW HELPER TEXT
		else {
			contextHelpTextSmall.drawString("depth",
											alignmentPairs[i]->depthImageRect.x + 5,
											alignmentPairs[i]->depthImageRect.y + 5 + contextHelpTextSmall.getLineHeight());
			contextHelpTextSmall.drawString("internal ir",
											alignmentPairs[i]->depthCheckersRect.x + 5,
											alignmentPairs[i]->depthCheckersRect.y + 5 + contextHelpTextSmall.getLineHeight());
			contextHelpTextSmall.drawString("external rgb",
											alignmentPairs[i]->colorCheckersRect.x + 5,
											alignmentPairs[i]->colorCheckersRect.y + 5 + contextHelpTextSmall.getLineHeight());
			
		}
		
		//DRAW IMAGES
		alignmentPairs[i]->deleteRect = ofRectangle(drawX,
													drawY,
													30,subRectHeight);
		
		alignmentPairs[i]->depthImageRect = ofRectangle(drawX+30, drawY, subRectWidth, subRectHeight);
		alignmentPairs[i]->depthCheckersRect = alignmentPairs[i]->depthImageRect;
		alignmentPairs[i]->depthCheckersRect.x += subRectWidth;
		
		alignmentPairs[i]->colorCheckersRect = alignmentPairs[i]->depthCheckersRect;
		alignmentPairs[i]->colorCheckersRect.x += subRectWidth;
		alignmentPairs[i]->colorCheckersRect.width *= (16./9.) / (4./3.); //aspect shift
		
//		alignmentPairs[i]->deleteRect = ofRectangle(alignmentPairs[i]->colorCheckersRect.getMaxX(),
//													drawY,
//													framewidth - alignmentPairs[i]->colorCheckersRect.getMaxX(), subRectHeight/2);
		alignmentPairs[i]->includeRect = alignmentPairs[i]->colorCheckersRect;
		alignmentPairs[i]->includeRect.x += alignmentPairs[i]->colorCheckersRect.width;
		alignmentPairs[i]->includeRect.width = framewidth - alignmentPairs[i]->colorCheckersRect.getMaxX();
		
		ofPushStyle();
		
		
		if(alignmentPairs[i]->included){
			ofNoFill();
			ofSetColor(boardColors[i]);
		}
		else{
			ofFill();
			ofSetColor(128, 200);
		}
		ofRect(alignmentPairs[i]->depthImageRect);
		ofRect(alignmentPairs[i]->depthCheckersRect);
		ofRect(alignmentPairs[i]->colorCheckersRect);

		
		if(hoverRect.getArea() != 0){
			ofNoFill();
			ofxEasingCubic cubic;
			ofSetColor(255, 0, 0, ofMap(sin(ofGetFrameNum()/4.0 ), -1, 1, 50, 255));
			ofRect(hoverRect);
		}
		
		ofPopStyle();
		
		drawY += subRectHeight;
	}
	
	
	//Draw RIGHT side previews
	if(hoverPreviewingCaptured){
		hoverPreviewImage.draw(previewRectRight);
		calibrationPreview.draw(previewRectRight);
	}
	else if(hoverPreviewDepth){
		depthImage.draw(previewRectRight);
		calibrationPreview.draw(previewRectRight);
	}
	else if(hoverPreviewIR){
		depthImageProvider->getRawIRImage().draw(previewRectRight);
		calibrationPreview.draw(previewRectRight);
	}
	else if(calibrationGenerated && hasIncludedBoards){
		ofPushStyle();
		glEnable(GL_DEPTH_TEST);
		cam.begin(previewRectRight);
		
		ofSetColor(255);
		renderer->drawMesh();
		
		ofPushMatrix();
		ofScale(-1, -1, 1);
		ofTranslate(0,0,-4);
		glPointSize(7);
		glEnable(GL_POINT_SMOOTH);
		inlierPoints.drawVertices();
		ofPopMatrix();

		
		cam.end();
		glDisable(GL_DEPTH_TEST);
		
		ofPopStyle();
		
		drawCalibrationNumbers();
	}
	else{
		ofPushStyle();
		ofNoFill();
		ofRect(previewRectRight);
		string dragText = "No Correspondence to Visualize";
		contextHelpTextSmall.drawString(dragText,
										previewRectRight.getCenter().x - contextHelpTextSmall.getStringBoundingBox(dragText, 0, 0).width/2,
										previewRectRight.y + contextHelpTextSmall.getLineHeight()*2);
		ofNoFill();
		
		ofPopStyle();
	}
	
	drawDimensionsEntry();
}

void ofxRGBDCaptureGui::drawDimensionsEntry(){
	
	ofPushStyle();
	ofFill();
	ofSetColor(0);
	ofRect(checkerboardDimensions.bounds);
	
	ofSetColor(255);
	contextHelpTextSmall.drawString("Square Size (cm)", previewRectRight.x, previewRectRight.getMaxY() + contextHelpTextSmall.getLineHeight());
	checkerboardDimensions.draw();
	
	ofSetColor(255);
	ofNoFill();
	ofRect(checkerboardDimensions.bounds);
	
	ofPopStyle();
}

void ofxRGBDCaptureGui::drawDepthImage(ofRectangle& targetRect){
	if(currentRenderMode == RenderPointCloud){
//		pointcloudPreviewCam.begin(targetRect);
//		pointcloudPreview.drawPointCloud();
		//glEnable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(2);
		
		ofMesh mesh;
		for(int y = 0; y < 480; y++){
			for(int x = 0; x < 640; x++){
				//0.104200 ref dist 120.000000
				double ref_pix_size = 0.104200;
				double ref_distance = 120.000000;
//				double wz = depthImageProvider->getRawDepth().getPixels()[y*640+x];
				double wz = pointcloudPreview.getDepthImage().getPixels()[y*640+x];
				double factor = 2 * ref_pix_size * wz / ref_distance;
				double wx = (double)(x - 640/2) * factor;
				double wy = (double)(y - 480/2) * factor;
				mesh.addVertex(ofVec3f(wx,-wy,-wz));
			}
		}
		
		pointcloudPreviewCam.begin(targetRect);
		mesh.drawVertices();
//		pointcloudPreview.drawPointCloud();
		pointcloudPreviewCam.end();
		
//		glDisable(GL_DEPTH_TEST);
//		pointcloudPreviewCam.end();
		//drawPointcloud(depthSequence.getDepthImageSequence()->getPixels(), false);
	}
	else {
		depthImage.draw(targetRect);
	}
}

void ofxRGBDCaptureGui::drawCalibrationNumbers(){
	ofVec2f rgbFov = toOf(rgbCalibration.getDistortedIntrinsics().getFov());
	ofVec2f rgbPP = toOf(rgbCalibration.getDistortedIntrinsics().getPrincipalPoint());
	
	
	ofFill();
	ofSetColor(confirmedColor, 50);
	ofRect(*btnRGBLoadCalibration);
	
	ofSetColor(255);
	contextHelpTextSmall.drawString("Field of View: " + ofToString(rgbFov.x,2) + " x " + ofToString(rgbFov.y,2) +
									"\nPrincipal Point: " +  ofToString(rgbPP.x,2) + " x " + ofToString(rgbPP.y,2),
									btnRGBLoadCalibration->x+10,btnRGBLoadCalibration->y+55);
	
	contextHelpTextSmall.drawString("Distortion Coefficients:\n" +
									ofToString(rgbCalibration.getDistCoeffs().at<double>(0),4) + " " +
									ofToString(rgbCalibration.getDistCoeffs().at<double>(1),4) + " " +
									ofToString(rgbCalibration.getDistCoeffs().at<double>(2),4) + " " +
									ofToString(rgbCalibration.getDistCoeffs().at<double>(3),4),
									btnRGBLoadCalibration->x+previewRectRight.width/2+10,btnRGBLoadCalibration->y+55);
		
	
}

void ofxRGBDCaptureGui::drawCapture(){
	bool drawCamera = providerSet && depthImageProvider->deviceFound();
	if(drawCamera){
		drawDepthImage(previewRectLeft);
	}
	else{
		contextHelpTextSmall.drawString("Depth sensor not found. Reconnect the device and restart the application.", previewRectLeft.x + 30, previewRectLeft.y + 30);
	}
	if(recorder.isRecording()){
		ofPushStyle();
		ofSetColor(255, 0, 0);
		ofNoFill();
		ofSetLineWidth(5);
		
		ofRect(previewRectLeft);
		
		//draw semi transparent record
		ofVec2f recIndicatorPos(previewRectLeft.getTopLeft() + ofVec2f(40,40));	
		if(ofGetFrameNum() % 30 < 15){
			ofFill();
			ofCircle(recIndicatorPos, 6);
			ofNoFill();
			ofCircle(recIndicatorPos, 6);
		}
		ofSetColor(255);
		contextHelpTextSmall.drawString("REC", recIndicatorPos.x+10, recIndicatorPos.y+7);
		ofPopStyle();
	}
	
	int framesWaiting = recorder.numFramesWaitingSave();
	//Draw recorder safety bar
	if(framesWaiting > 0){
		ofPushStyle();
		float width = framesWaiting/2000.0 * btnToggleRecord->width;
		ofFill();
		ofSetColor(255,0, 0);
		ofRect(btnToggleRecord->x,btnToggleRecord->y,width,btnToggleRecord->height);
		//flash the recorder
		if(ofGetFrameNum() % 30 < 15){
			ofSetColor(255, 0, 0, 40);
			ofRect(*btnToggleRecord);
		}
		if(framesWaiting > 200.0){
			ofSetColor(255);
			contextHelpTextSmall.drawString("Warning: " + ofToString(framesWaiting) + " Frames waiting in RAM. Stop recording soon to safely writing them to disk.",
											btnToggleRecord->getBottomLeft().x,
											btnToggleRecord->getBottomLeft().y - 5);
		}
		ofPopStyle();
	}
	
	drawSceneButtons();
}


void ofxRGBDCaptureGui::drawPlayback(){
	
	if(depthSequence.getDepthImageSequence() != NULL &&
	   depthSequence.getDepthImageSequence()->isLoaded())
	{
		drawDepthImage(previewRectLeft);
	}
	else{
		contextHelpTextSmall.drawString("No playback sequence selected.", previewRectLeft.x + 30, previewRectLeft.y + 30);	
	}
	
	drawSceneButtons();
}

void ofxRGBDCaptureGui::drawSceneButtons(){
	
	for(int i = 0; i < btnScenes.size(); i++){
		if(btnScenes[i].isSelected){
			ofPushStyle();
			ofSetColor(timeline.getColors().highlightColor);
			ofRectangle highlighRect(btnScenes[i].button->x,btnScenes[i].button->y,
									 btnScenes[i].button->width, btnScenes[i].button->height*.25);
			
			ofRect(highlighRect);
			ofPopStyle();
		}
		
		ofPushStyle();
		ofSetColor(timeline.getColors().disabledColor);
		float percentComplete = float(btnScenes[i].sceneRef->compressedDepthFrameCount) / float(btnScenes[i].sceneRef->totalDepthFrameCount);
		float processedWidth = btnScenes[i].button->width*percentComplete;
		ofRectangle highlighRect(btnScenes[i].button->x + processedWidth,
								 btnScenes[i].button->y,
								 btnScenes[i].button->width-processedWidth, btnScenes[i].button->height);
		ofRect(highlighRect);
		ofPopStyle();
	}
}

void ofxRGBDCaptureGui::objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y){
	
}

void ofxRGBDCaptureGui::objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y){
	
}

void ofxRGBDCaptureGui::objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button){
	
}

void ofxRGBDCaptureGui::objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button){

	if(backpackMode){
		return;
	}
	
  	if(object == btnSetDirectory){
		loadDirectory();
	}
	else if(object == btnCalibrateDepthCamera){
		refineDepthCalibration();
	}
	else if(object == btnRGBLoadCalibration){
		loadRGBIntrinsicImages();
	}
	else if(object == btnGenerateCalibration){
		generateCorrespondence();
	}
	else if(object == btnToggleRecord){
		toggleRecord();
	}
	else if(object == btnRenderBW){
		currentRenderMode = RenderBW;
		currentRenderModeObject = btnRenderBW;
	}
	else if(object == btnRenderRainbow){
		currentRenderMode = RenderRainbow;
		currentRenderModeObject = btnRenderRainbow;
	}
	else if(object == btnRenderPointCloud){
		pointcloudPreviewCam.setPosition(0, 0, 0);
		pointcloudPreviewCam.setOrientation(ofQuaternion());
		pointcloudPreviewCam.rotate(0, ofVec3f(0,1,0));
		pointcloudPreviewCam.setAnglesFromOrientation();
	
		currentRenderMode = RenderPointCloud;
		currentRenderModeObject = btnRenderPointCloud;

		if(depthSequence.getDepthImageSequence() != NULL &&
		   depthSequence.getDepthImageSequence()->isLoaded())
		{
			updateDepthImage(depthSequence.getDepthImageSequence()->getPixels());
		}
	}
	else if(find(tabSet.begin(),tabSet.end(), object) != tabSet.end()){
		updateInterfaceForTab(object);
	}
	else {
		for(int i = 0; i < btnScenes.size(); i++){
			if(object == btnScenes[i].button){
				if(loadSequenceForPlayback( i )){
					for(int b = 0; b < btnScenes.size(); b++){
						btnScenes[b].isSelected = b == i;
					}
				}
				break;
			}
		}
	}
}

void ofxRGBDCaptureGui::updateInterfaceForTab(ofxMSAInteractiveObject* tab){
	
	btnRGBLoadCalibration->disableAllEvents();
	btnCalibrateDepthCamera->disableAllEvents();
	btnGenerateCalibration->disableAllEvents();
	btnToggleRecord->disableAllEvents();
	checkerboardDimensions.disable();
	timeline.hide();
	timeline.stop();
	
	if(tab == btnIntrinsicsTab){
		disableSceneButtons();
		currentTab = TabIntrinsics;
		currentTabObject = btnIntrinsicsTab;
		btnRGBLoadCalibration->enableAllEvents();
		btnCalibrateDepthCamera->enableAllEvents();
		checkerboardDimensions.enable();
	}
	else if(tab == btnExtrinsicsTab){
		disableSceneButtons();
		currentTab = TabExtrinsics;
		currentTabObject = btnExtrinsicsTab;
		btnGenerateCalibration->enableAllEvents();
		checkerboardDimensions.enable();
	}
	else if(tab == btnRecordTab){
		enableSceneButtons();
		currentTab = TabCapture;
		currentTabObject = btnRecordTab;
		btnToggleRecord->enableAllEvents();
	}
	else if(tab == btnPlaybackTab){
		enableSceneButtons();
		currentTab = TabPlayback;
		currentTabObject = btnPlaybackTab;
		timeline.show();
		if(depthSequence.getDepthImageSequence() != NULL && depthSequence.getDepthImageSequence()->isLoaded()){
			updateDepthImage(depthSequence.getDepthImageSequence()->getPixels());
		}
	}
}

void ofxRGBDCaptureGui::toggleRecord(){
	if(providerSet || !depthImageProvider->deviceFound()){
		recorder.toggleRecord();

		if(backpackMode){
			if(recorder.isRecording()){
				recordOn.setPosition(0);
				recordOn.play();
			}
			else{
				recordOff.setPosition(0);
				recordOff.play();
			}
		}
		updateSceneButtons();
	}
	else {
		ofSystemAlertDialog("No depth sensor to record");
	}
}

void ofxRGBDCaptureGui::loadRGBIntrinsicImages(){
	ofFileDialogResult r = ofSystemLoadDialog("Load RGB Images", true);
	if(r.bSuccess){
		loadRGBIntrinsicImages(r.getPath());
		if(rgbCalibration.isReady()){
			saveRGBIntrinsicImages();
		}
	}
}

void ofxRGBDCaptureGui::loadRGBIntrinsicImages(string filepath){
	ofDirectory dir(filepath);
	if(dir.isDirectory()){
		dir.listDir();
		vector<string> paths;
		for(int i = 0; i < dir.numFiles(); i++){
			paths.push_back(dir.getPath(i));
		}
		loadRGBIntrinsicImages(paths);
	}
}

void ofxRGBDCaptureGui::loadRGBIntrinsicImages(vector<string> filepaths){

	rgbCalibrationImages.clear();
	rgbCalibrationFileNames.clear();
	rgbCalibration.reset();
	
	ofVideoPlayer player;
	ofImage image;
	image.setUseTexture(false);
	player.setUseTexture(false);
	
	for(int i = 0; i < filepaths.size(); i++){
		string filename = filepaths[i];
		string extension = ofToLower(ofFilePath::getFileExt(filename));
		string shortFilename = ofFilePath::getFileName(filename);
		if(ofContains(ofxRGBDScene::getValidVideoExtensions(), extension)){
			
			if(!player.loadMovie(filename)){
				ofSystemAlertDialog("Calibration movie " + shortFilename + " failed to load. Ensure that it uses a supported codec.");
				continue;
			}
			
			player.setPosition(.5);
			player.update();
			image.setFromPixels(player.getPixelsRef());
			image.setImageType(OF_IMAGE_GRAYSCALE);
		}
		else if(extension == "png" || extension == "jpg" || extension == "jpeg"){
			if(!image.loadImage(filename)){
				ofSystemAlertDialog("Calibration image " + shortFilename + " failed to load. Ensure that it is a valid JPG or PNG.");
				continue;
			}
		}
		else{
			ofSystemAlertDialog("Unsupported file type: " + shortFilename + ". Please use mov, png, or jpg");
			continue;
		}
		
		if(!addRGBImageToIntrinsicSet( image, shortFilename )){
			ofSystemAlertDialog("Calibration image " + shortFilename + " failed to find checker board. Ensure that it is well lit and unobscured");
		}
	}
	
	//If we successfully calibrated...
	if(rgbCalibrationImages.size() > 3){
		
		rgbCalibration.calibrate();
		if(rgbCalibration.isReady()){

			rgbCalibration.save(matrixDirectory+"rgbCalibrationBase.yml");

			for(int i = 0; i < rgbCalibrationImages.size(); i++){
				//rgbCalibration.undistort( toCv(rgbCalibrationImages[i]) );
				rgbCalibrationImages[i].setUseTexture(true);
				rgbCalibrationImages[i].update();
			}
		}
	}
	else{
		if(rgbCalibrationImages.size() != 0){
			ofSystemAlertDialog("Too few calibration images provided to create intrinsics. Must have 4 or more.");
		}
	}
}

bool ofxRGBDCaptureGui::addRGBImageToIntrinsicSet(ofImage& image, string fileName){
	
	if(rgbCalibration.add(toCv(image))){
		rgbCalibrationImages.push_back(image);
		rgbCalibrationFileNames.push_back(fileName);
		return true;
	}

	//sometimes big boards fail, try a resized image
	vector<Point2f> pointBuf;
	image.resize(image.getWidth()/2, image.getHeight()/2);
	if(rgbCalibration.findBoard(toCv(image), pointBuf)){
		for(int i = 0; i < pointBuf.size(); i++){
			pointBuf[i] *= 2;
		}
		rgbCalibration.imagePoints.push_back(pointBuf);
		image.resize(image.getWidth()*2, image.getHeight()*2);
		rgbCalibrationImages.push_back(image);
		rgbCalibrationFileNames.push_back(fileName);
		return true;
	}
	
	//both scales failed...
	return false;
}

void ofxRGBDCaptureGui::squareSizeChanged(string& args){
	squareSize = ofToFloat(args);
	if(squareSize <= 0 && !checkerboardDimensions.getIsEditing()){
		squareSize = 2.54;
	}
	rgbCalibration.setSquareSize(squareSize);
	calibrationPreview.getCalibration().setSquareSize(squareSize);
	
	
	ofBuffer buf;
	buf.append(ofToString(squareSize));
	ofBufferToFile(squareSizeFilePath, buf);
	
	checkerboardDimensions.text = ofToString(squareSize, 2);
	
	cout << "Square size changed to " << squareSize << endl;
}

void ofxRGBDCaptureGui::saveRGBIntrinsicImages(){
	
	ofDirectory::removeDirectory(rgbCalibrationDirectory, true);
	ofDirectory::createDirectory(rgbCalibrationDirectory);	
	for(int i = 0; i < rgbCalibrationImages.size(); i++){
		string filePath = ofFilePath::removeExt(rgbCalibrationDirectory + rgbCalibrationFileNames[i]) + ".png";
//		cout << "saving images to file path " << filePath << endl;
		rgbCalibrationImages[i].saveImage( filePath );
	}
}

void ofxRGBDCaptureGui::objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y){
	
}

void ofxRGBDCaptureGui::mousePressed(ofMouseEventArgs& args){
	
}

void ofxRGBDCaptureGui::mouseMoved(ofMouseEventArgs& args){
	cam.applyRotation = cam.applyTranslation = (currentTab == TabExtrinsics && previewRectRight.inside(args.x, args.y));
	pointcloudPreviewCam.applyRotation = pointcloudPreviewCam.applyTranslation = (currentRenderMode == RenderPointCloud && currentTab != TabExtrinsics && previewRectLeft.inside(args.x, args.y));

	hoverRect = ofRectangle();
	//CHECK FOR HOVER IN INTRINSICS
	if(currentTab == TabIntrinsics){
		
	}
	else if(currentTab == TabExtrinsics){
		hoverPreviewDepth =  hoverPreviewIR = hoverPreviewingCaptured = false;
		for(int i = alignmentPairs.size()-1; i >= 0; i--){
			if(alignmentPairs[i]->depthImageRect.inside(args.x,args.y)){
				if(alignmentPairs[i] == currentAlignmentPair && !alignmentPairs[i]->depthImage.isAllocated()){
					hoverPreviewDepth = true;
				}
				else{
					hoverPreviewImage.setFromPixels(alignmentPairs[i]->depthImage.getPixelsRef());
//					calibrationPreview.setTestImage(alignmentPairs[i]->depthCheckers);
					hoverPreviewingCaptured = true;
				}
				hoverRect = alignmentPairs[i]->depthImageRect;				
			}
			if(alignmentPairs[i]->depthCheckersRect.inside(args.x,args.y)){
				if(alignmentPairs[i] == currentAlignmentPair && !alignmentPairs[i]->depthCheckers.isAllocated()){
					hoverPreviewIR = true;
				}
				else{
					hoverPreviewImage.setFromPixels(alignmentPairs[i]->depthCheckers.getPixelsRef());
//					calibrationPreview.setTestImage(alignmentPairs[i]->depthCheckers);
					hoverPreviewingCaptured = true;
				}
				hoverRect = alignmentPairs[i]->depthCheckersRect;
			}
			
			if(alignmentPairs[i]->colorCheckersRect.inside(args.x,args.y)){
				//TODO check for colors to set and make it work within the given size
				hoverRect = alignmentPairs[i]->colorCheckersRect;
			}
		}
	}
}

void ofxRGBDCaptureGui::mouseDragged(ofMouseEventArgs& args){
	
}

void ofxRGBDCaptureGui::mouseReleased(ofMouseEventArgs& args){
	
	if(backpackMode){
		toggleRecord();
		return;
	}
	
	//CHECK FOR HOVER IN INTRINSICS
	if(currentTab == TabIntrinsics){
		
	}
	else if(currentTab == TabExtrinsics){
		//CHECK FOR HOVERING ON EXTRINSICS
		for(int i = alignmentPairs.size()-1; i >= 0; i--){
			//only if it's the current one
			if(alignmentPairs[i] == currentAlignmentPair){
				if(alignmentPairs[i]->depthImageRect.inside(args.x,args.y)){
					//CAPTURE DEPTH IMAGE
					//TODO: ensure depth image has data
					alignmentPairs[i]->depthPixelsRaw = depthImageProvider->getRawDepth();
					alignmentPairs[i]->depthImage = recorder.getCompressor().convertTo8BitImage( alignmentPairs[i]->depthPixelsRaw );
					hoverPreviewImage.setFromPixels(alignmentPairs[i]->depthImage.getPixelsRef());
					//preview the still of what you just clicked
					hoverPreviewDepth = false;
					hoverPreviewingCaptured = true;
					break;
				}
				if(alignmentPairs[i]->depthCheckersRect.inside(args.x,args.y)){
					//CAPTURE IR IMAGE
					vector<Point2f> pointBuf;
					if(rgbCalibration.findBoard(toCv(depthImageProvider->getRawIRImage()), pointBuf)){
						alignmentPairs[i]->depthCheckers.setFromPixels(depthImageProvider->getRawIRImage());
						hoverPreviewImage.setFromPixels(alignmentPairs[i]->depthCheckers.getPixelsRef());
						//preview the still of what you just clicked
						hoverPreviewIR = false;
						hoverPreviewingCaptured = true;
					}
					else{
						ofSystemAlertDialog("No checkerboard found in IR image!");
					}
					break;
				}
				if(alignmentPairs[i]->colorCheckersRect.inside(args.x,args.y)){
					//TODO: load checker movie from dialog box
					break;
				}
			}
					 
			//DELETE CLICKED
			if(alignmentPairs[i]->deleteRect.inside(args.x,args.y)){

				if(alignmentPairs[i] != currentAlignmentPair){
					
					delete alignmentPairs[i];
					alignmentPairs.erase(alignmentPairs.begin() + i);
					if(alignmentPairs.size() <= 1){
						calibrationGenerated = false;
					}
					if(currentRendererPreviewIndex == i){
						previewNextAlignmentPair();
					}
					saveCorrespondenceImages();
					break;
					
				}
				else {
					currentAlignmentPair->colorCheckers.clear();
					currentAlignmentPair->depthCheckers.clear();
					currentAlignmentPair->depthImage.clear();
				}
			}

			//INCLUDED CLICKED
			if(alignmentPairs[i] != currentAlignmentPair && alignmentPairs[i]->includeRect.inside(args.x, args.y)){
				alignmentPairs[i]->included = !alignmentPairs[i]->included;
				hasIncludedBoards = false;
				for(int i = 0; i < alignmentPairs.size()-1; i++){
					hasIncludedBoards |= alignmentPairs[i]->included;
				}
				saveCorrespondenceIncludes();
			}
		}
		
		if(currentAlignmentPair->depthImage.isAllocated() && currentAlignmentPair->depthCheckers.isAllocated()){
			currentAlignmentPair = new AlignmentPair();
			currentAlignmentPair->included = true;
			alignmentPairs.push_back(currentAlignmentPair);
			saveCorrespondenceImages();
		}
	}
}

void ofxRGBDCaptureGui::keyPressed(ofKeyEventArgs& args){
	if(args.key == 'R'){
		gpuRenderer.reloadShader();
	}
}

void ofxRGBDCaptureGui::keyReleased(ofKeyEventArgs& args){
	if(currentTab == TabExtrinsics && calibrationGenerated){
		if(args.key == OF_KEY_RIGHT){
			previewNextAlignmentPair();
		}
		else if(args.key == OF_KEY_LEFT){
			previewPreviousAlignmentPair();
		}
	}
	else if(currentTab == TabCapture){
		if(args.key == ' '){
			cout << "togglign record" << endl;
			toggleRecord();
		}
	}
	else if(currentTab == TabPlayback){
		if(args.key == ' '){
			timeline.togglePlay();
		}
	}
	if(args.key == OF_KEY_UP){
		renderer = &gpuRenderer;
	}
	else if(args.key == OF_KEY_DOWN){
		renderer = &cpuRenderer;
	}
	
	if(args.key == 'M'){
		backpackMode = !backpackMode;
		ofToggleFullscreen();
		if(backpackMode){

		}
		cout << "KEY PRESSED " << backpackMode << endl;
	}
}

void ofxRGBDCaptureGui::previewNextAlignmentPair(){

	if(calibrationGenerated == false || alignmentPairs.size() <= 1){
		return;
	}
	currentRendererPreviewIndex = (currentRendererPreviewIndex + 1) % (alignmentPairs.size()-1);
	
	cpuRenderer.setDepthImage(alignmentPairs[currentRendererPreviewIndex]->depthPixelsRaw);
	gpuRenderer.setDepthImage(alignmentPairs[currentRendererPreviewIndex]->depthPixelsRaw);
	
	cpuRenderer.setRGBTexture(alignmentPairs[currentRendererPreviewIndex]->colorCheckers);
	gpuRenderer.setRGBTexture(alignmentPairs[currentRendererPreviewIndex]->colorCheckers);

	cpuRenderer.update();
	gpuRenderer.update();

}

void ofxRGBDCaptureGui::previewPreviousAlignmentPair(){
	if(calibrationGenerated == false || alignmentPairs.size() <= 1){
		return;
	}
	
	currentRendererPreviewIndex--;
	if(currentRendererPreviewIndex < 0){
		currentRendererPreviewIndex = alignmentPairs.size()-2;
	}
	gpuRenderer.setDepthImage(alignmentPairs[currentRendererPreviewIndex]->depthPixelsRaw);
	gpuRenderer.setRGBTexture(alignmentPairs[currentRendererPreviewIndex]->colorCheckers);

	cpuRenderer.setDepthImage(alignmentPairs[currentRendererPreviewIndex]->depthPixelsRaw);
	cpuRenderer.setRGBTexture(alignmentPairs[currentRendererPreviewIndex]->colorCheckers);

	cpuRenderer.update();	
	gpuRenderer.update();

}

void ofxRGBDCaptureGui::loadDirectory(){
	if(recorder.numFramesWaitingCompession() != 0){

		ofSystemAlertDialog("Cannot change directory while footage is compressing.");
		return;
	}
	
	ofFileDialogResult r = ofSystemLoadDialog("Select Record Directory", true);
	if(r.bSuccess){
		loadDirectory(r.getPath());
	}
}

#pragma mark Load Working Directory
void ofxRGBDCaptureGui::loadDirectory(string path){
	
	if(path == ""){
		ofSystemAlertDialog("The Working directory empty. Resetting to bin/data/depthframes/");
		loadDefaultDirectory();
		return;
	}
	
	ofDirectory workingDirPath(path);
	if(!workingDirPath.exists()){
		ofSystemAlertDialog("The Working directory empty. Resetting to bin/data/depthframes/");
		loadDefaultDirectory();
		return;
	}
	
	clearCorrespondenceImages();
	
	alignmentPairs.clear();
	hasIncludedBoards = false;
	calibrationGenerated = false;
	rgbCalibrationImages.clear();
	rgbCalibrationFileNames.clear();

	workingDirectory = path;
	recorder.setRecordLocation(path, "frame");
	rgbCalibrationDirectory  = workingDirectory+"/_calibration/rgbCalibration/";
	correspondenceDirectory = workingDirectory+"/_calibration/correspondence/";
	matrixDirectory = workingDirectory+"/_calibration/matrices/";
	squareSizeFilePath = workingDirectory+"/_calibration/squaresize.txt";
	
	cam.cameraPositionFile = workingDirectory+"/_calibration/calibrationPreviewCamera.xml";
	cam.loadCameraPosition();
	cam.autosavePosition = true;
	
	pointcloudPreviewCam.cameraPositionFile = workingDirectory+"/_calibration/pointcloudPreviewFile.xml";
	pointcloudPreviewCam.loadCameraPosition();
	pointcloudPreviewCam.autosavePosition = true;
	
	string newSize;
	if(ofFile::doesFileExist(squareSizeFilePath)){
		cout << "found existing size at path " << squareSizeFilePath << endl;
		ofBuffer squareSizebuf = ofBufferFromFile(squareSizeFilePath);
		newSize = squareSizebuf.getText();
	}
	else{
		newSize = "2.54";
	}
	squareSizeChanged(newSize);

	ofDirectory dir = ofDirectory(rgbCalibrationDirectory);
	if(!dir.exists()){
		dir.create(true);
	}
	else{
		loadRGBIntrinsicImages(rgbCalibrationDirectory);
	}

	bool existingRGBCalibration = false;
	dir = ofDirectory(matrixDirectory);
	if(!dir.exists()){
		dir.create(true);
	}
	else{
		if(ofFile::doesFileExist(matrixDirectory + "depthCalib.yml")){
			
			depthCalibrationRefined.load(matrixDirectory + "depthCalib.yml");
			Mat depthCameraMatrix = depthCalibrationRefined.getDistortedIntrinsics().getCameraMatrix();
			fov = ofVec2f(depthCameraMatrix.at<double>(0,0), depthCameraMatrix.at<double>(1,1));
			pp = ofVec2f(depthCameraMatrix.at<double>(0,2),depthCameraMatrix.at<double>(1,2));
			btnCalibrateDepthCamera->setLabel("Depth Camera Self-Calibrated!");
			depthCameraSelfCalibrated = true;
		}
		
		if(ofFile::doesFileExist(matrixDirectory + "rgbCalib.yml")){
			existingRGBCalibration = true;
			rgbCalibration.load(matrixDirectory + "rgbCalib.yml");
		}
	}
	
	dir = ofDirectory(correspondenceDirectory);
	if(!dir.exists()){
		dir.create(true);
	}
	else{
		
		dir.allowExt("png");
		dir.listDir();
		if(dir.numFiles() % 3 != 0){
			ofLogError("_calibration/Correspondence directory may have stray files.");
		}
		
		for(int i = 0; i < dir.numFiles(); i+=3){
			ofxXmlSettings includedImages;
			includedImages.loadFile(correspondenceDirectory + "inclusions.xml");
			string depthCeckersFile  = correspondenceDirectory + ofToString(i/3) + "_checkers.png";
			string colorCheckersFile = correspondenceDirectory + ofToString(i/3) + "_color_checkers.png";
			string depthColorFile  = correspondenceDirectory + ofToString(i/3) + "_depth_pixels.png";
			
			if(ofFile::doesFileExist(depthCeckersFile) &&
			   ofFile::doesFileExist(colorCheckersFile) &&
			   ofFile::doesFileExist(depthColorFile) )
			{
				AlignmentPair* pair = new AlignmentPair();
				recorder.getCompressor().readCompressedPng(depthColorFile, pair->depthPixelsRaw);
				pair->depthImage = recorder.getCompressor().convertTo8BitImage(pair->depthPixelsRaw);
				pair->depthCheckers.loadImage(depthCeckersFile);
				pair->depthCheckers.setImageType(OF_IMAGE_GRAYSCALE);
				pair->colorCheckers.loadImage(colorCheckersFile);
				pair->included = includedImages.getValue("checkers_"+ofToString(i/3), true);
				hasIncludedBoards |= pair->included;
				alignmentPairs.push_back(pair);
			}
			else{
				ofLogError("Missing file in correspondence triplet " + ofToString(i/3));
			}
		}
	}
	
	currentAlignmentPair = new AlignmentPair();
	currentAlignmentPair->included = true;
	alignmentPairs.push_back(currentAlignmentPair);
	
	if(existingRGBCalibration && hasIncludedBoards){
		calibrationGenerated = true;
		//generateCorrespondence();
		setupRenderer();
	}

	btnSetDirectory->setLabel("Working Dir: " + path );
	updateSceneButtons();
	ofxXmlSettings defaults;
	defaults.loadFile("defaultBin.xml");
	defaults.setValue("bin", path);
	defaults.saveFile("defaultBin.xml");
}

void ofxRGBDCaptureGui::clearCorrespondenceImages(){
	
	currentAlignmentPair = NULL;
	for(int i = 0; i < alignmentPairs.size(); i++){
		delete alignmentPairs[i];
	}
	alignmentPairs.clear();
}

void ofxRGBDCaptureGui::loadDefaultDirectory(){
	
	//create it if it doesn't exist
	string defaultDir = "depthframes";
	if(!ofDirectory(defaultDir).exists()){
		ofDirectory(defaultDir).create(true);
	}
	loadDirectory(defaultDir);
}

//--------------------------------------------------------------
bool ofxRGBDCaptureGui::loadSequenceForPlayback( int index ){
	
	if(recorder.getScenes()[index]->uncompressedDepthFrameCount == 0){
		depthSequence.loadSequence( recorder.getScenes()[index]->depthFolder );
		timeline.setDurationInSeconds(depthSequence.getDepthImageSequence()->getDurationInSeconds());
		return true;
	}
	return false;
}

//--------------------------------------------------------------
void ofxRGBDCaptureGui::updateSceneButtons(){
	
	vector<ofxRGBDScene*>& scenes = recorder.getScenes();
	if(scenes.size() == btnScenes.size()){
		return;
	}
	
	for(int i = 0; i < btnScenes.size(); i++){
		btnScenes[i].button->disableAllEvents();
		btnScenes[i].button->enabled = false;
		delete btnScenes[i].button;
	}
	
	btnScenes.clear();
	
	float x = previewRectRight.x;
	float y = previewRectRight.y;
	float takebuttonHeight = previewRectRight.height*.05;
	for(int i = 0; i < scenes.size(); i++){
		SceneButton tb;
		tb.isSelected = false;
		tb.sceneRef = scenes[i];
		
		ofxMSAInteractiveObjectWithDelegate* btnScene = new ofxMSAInteractiveObjectWithDelegate();
		
		btnScene->setPosAndSize(x, y, thirdWidth, btnheight*.66);
		btnScene->setLabel( scenes[i]->name );
		btnScene->setIdleColor(idleColor);
		btnScene->setDownColor(downColor);
		btnScene->setHoverColor(hoverColor);
		btnScene->setDelegate(this);
		btnScene->enableMouseEvents();
		btnScene->disableKeyEvents();
		btnScene->fontReference = &contextHelpTextSmall;
		tb.button = btnScene;
		btnScenes.push_back( tb );
		
		y += takebuttonHeight;
		
		while(y > previewRectRight.getMaxY()-takebuttonHeight){
			y -= previewRectRight.height;
			x += thirdWidth;
		}
		
	}
	
	if(currentTab == TabIntrinsics || currentTab == TabExtrinsics){
		disableSceneButtons();
	}
}

void ofxRGBDCaptureGui::disableSceneButtons(){
	for(int i = 0; i < btnScenes.size(); i++){
		btnScenes[i].button->disableAllEvents();
	}
}

void ofxRGBDCaptureGui::enableSceneButtons(){
	for(int i = 0; i < btnScenes.size(); i++){
		btnScenes[i].button->enableAllEvents();
	}
}

void ofxRGBDCaptureGui::dragEvent(ofDragInfo& dragInfo){
	string filename = dragInfo.files[0];
	string extension = ofToLower(ofFilePath::getFileExt(filename));
	
	bool draggedIntoImageRect = currentTab == TabIntrinsics && previewRectRight.inside( dragInfo.position );
	if(draggedIntoImageRect) {
		loadRGBIntrinsicImages(dragInfo.files);
		if(rgbCalibration.isReady()){
			saveRGBIntrinsicImages();
		}
	}
	
	if(currentTab == TabExtrinsics){
		if(extension == "png" || extension == "jpg" || extension == "jpeg"){ //TODO: make accept all valid files
			for(int i = 0; i < alignmentPairs.size()-1; i++){
				if(alignmentPairs[i]->depthImageRect.inside(dragInfo.position)){
					recorder.getCompressor().readCompressedPng(filename, alignmentPairs[i]->depthPixelsRaw);
					alignmentPairs[i]->depthImage = recorder.getCompressor().convertTo8BitImage(alignmentPairs[i]->depthPixelsRaw);
				}
				if(alignmentPairs[i]->depthCheckersRect.inside(dragInfo.position)){
					alignmentPairs[i]->depthCheckers.loadImage(filename);
					alignmentPairs[i]->depthCheckers.setImageType(OF_IMAGE_GRAYSCALE);
				}
				if(alignmentPairs[i]->colorCheckersRect.inside(dragInfo.position)){
					alignmentPairs[i]->colorCheckers.loadImage(filename);
				}
				
				hasIncludedBoards = true;
			}
			
//			if(currentAlignmentPair->depthImage.isAllocated() && currentAlignmentPair->depthCheckers.isAllocated()){
//				currentAlignmentPair = new AlignmentPair();
//				currentAlignmentPair->included = true;
//				
//				alignmentPairs.push_back(currentAlignmentPair);
//			}
		}
		

		if(ofContains(ofxRGBDScene::getValidVideoExtensions(), extension)){
			for(int i = 0; i < alignmentPairs.size(); i++){
				if(alignmentPairs[i]->colorCheckersRect.inside(dragInfo.position)){
					ofVideoPlayer p;
					p.setUseTexture(false);
					p.loadMovie(filename);
					p.setPosition(.5);
					alignmentPairs[i]->colorCheckers.setFromPixels(p.getPixelsRef());
					alignmentPairs[i]->colorCheckers.setImageType(OF_IMAGE_GRAYSCALE);
					hasIncludedBoards = true;					
				}
			}
		}
	}
}

void ofxRGBDCaptureGui::refineDepthCalibration(){

	if(!depthImageProvider->deviceFound() ){
		ofSystemAlertDialog("No depth sensor found. Plug in the device and restart the application.");
		return;
	}

	if(!ofFile("depthCalibBase.yml").exists()){
		ofSystemAlertDialog("No base calibrationfound. make sure the depthCalibBase.yml file is in the data folder next to the application.");
		return;		
	}

	depthCalibrationBase.load("depthCalibBase.yml");
	int width  = depthImageProvider->getRawIRImage().getWidth();
	int height = depthImageProvider->getRawIRImage().getHeight();
	vector<Point3f> depthObjectCollection;
	vector<Point2f> depthImageCollection;
	//Now with the current image try to create a projection for the depthCamera
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			ofVec3f objectPoint = depthImageProvider->getWorldCoordinateAt(x, y);
			//			cout << "object point is " << objectPoint << endl;
			if(objectPoint.z != 0){
				depthObjectCollection.push_back(Point3f(objectPoint.x,objectPoint.y,objectPoint.z));
				depthImageCollection.push_back(Point2f(x,y));
			}
		}
	}
	
	if(depthObjectCollection.size() == 0){
		ofSystemAlertDialog("No depth points to self calibrate against. Make sure the sensor is connected and looking out into the world.");
		return;
	}
	   
	cout << "depth object points for refinment " << depthObjectCollection.size() << endl;
	
	vector< vector<Point3f> > depthObjectPoints;
	vector< vector<Point2f> > depthImagePoints;
	depthObjectPoints.push_back(depthObjectCollection);
	depthImagePoints.push_back(depthImageCollection);
	
	vector<Mat> depthRotations, depthTranslations; //DUMMY extrinsics
	Mat depthCameraMatrix = depthCalibrationBase.getDistortedIntrinsics().getCameraMatrix();
	Mat depthDistCoeffs = depthCalibrationBase.getDistCoeffs();
	cout << "added depth object points " << depthObjectPoints.size() << " and depth image points " << depthImagePoints.size() << endl;
	cout << "base matrix is " << depthCameraMatrix << " base distortion is " << depthDistCoeffs << endl;
	calibrateCamera(depthObjectPoints, depthImagePoints, cv::Size(width,height), depthCameraMatrix, depthDistCoeffs, depthRotations, depthTranslations, CV_CALIB_USE_INTRINSIC_GUESS);
	
	cout << "Final camera matrix is " << depthCameraMatrix << endl;
	cout << "Final distortion coefficients " << depthDistCoeffs << endl;
	cout << "Depth rotation " << depthRotations[0] << endl;
	cout << "Depth translation " << depthTranslations[0] << endl;
	
	//Intrinsics depthIntrinsics;
	//depthIntrinsics.setup(depthCameraMatrix, cv::Size(640,480));
	
	Intrinsics newDepth;
	newDepth.setup(depthCameraMatrix,cv::Size(width,height));
	
	//TODO impose folder structure
	depthCalibrationRefined.setIntrinsics(newDepth, depthDistCoeffs);

	depthCameraMatrix = depthCalibrationRefined.getDistortedIntrinsics().getCameraMatrix();
	fov = ofVec2f(depthCameraMatrix.at<double>(0,0), depthCameraMatrix.at<double>(1,1));
	pp = ofVec2f(depthCameraMatrix.at<double>(0,2),depthCameraMatrix.at<double>(1,2));
	
	if(abs(320 - pp.x) > 2 || abs(240 - pp.y) > 2 ){
		ofSystemAlertDialog("Self Calibration failed. Make sure the depth camera is pointed out into space and is seeing a wide range of depth values.");
	}
	else{
		cout << "fov " << fov << endl;
		cout << "principal point " << pp << endl;
		btnCalibrateDepthCamera->setLabel("Depth Camera Self-Calibrated!");
		depthCalibrationRefined.save(matrixDirectory + "depthCalib.yml");
		depthCameraSelfCalibrated = true;
	}
}

//TODO: need a way to load correspondence from XML/folder on start up
void ofxRGBDCaptureGui::saveCorrespondenceImages(){
	
	ofDirectory::removeDirectory(correspondenceDirectory, true);
	ofDirectory::createDirectory(correspondenceDirectory);

	for(int i = 0; i < alignmentPairs.size(); i++){
		if(alignmentPairs[i]->depthPixelsRaw.isAllocated() &&
		   alignmentPairs[i]->depthCheckers.isAllocated() &&
		   alignmentPairs[i]->colorCheckers.isAllocated() )
		{
			
			//save the correspondnce images
			recorder.getCompressor().saveToCompressedPng(correspondenceDirectory + ofToString(i) + "_depth_pixels.png",
														 alignmentPairs[i]->depthPixelsRaw.getPixels());
			alignmentPairs[i]->depthCheckers.saveImage(correspondenceDirectory + ofToString(i) + "_checkers.png");
			alignmentPairs[i]->colorCheckers.saveImage(correspondenceDirectory + ofToString(i) + "_color_checkers.png");
		
		}
	}
	
	saveCorrespondenceIncludes();
}

void ofxRGBDCaptureGui::saveCorrespondenceIncludes(){
	ofxXmlSettings inclusions;
	inclusions.loadFile(correspondenceDirectory + "inclusions.xml");
	
	for(int i = 0; i < alignmentPairs.size(); i++){
		if(alignmentPairs[i]->depthPixelsRaw.isAllocated() &&
		   alignmentPairs[i]->depthCheckers.isAllocated() &&
		   alignmentPairs[i]->colorCheckers.isAllocated() )
		{
			inclusions.setValue("checkers_" + ofToString(i), alignmentPairs[i]->included);
		}
	}
	inclusions.saveFile();
}

void ofxRGBDCaptureGui::generateCorrespondence(){
	if(!depthCalibrationRefined.isReady() || !rgbCalibration.isReady()){
		ofSystemAlertDialog("You cannot generate a correspondence before you've calibrated both the RGB and Depth lenses.");
		return;
	}

	//reset camera view
	cam.targetNode.setPosition(0, 0, 0);
	ofQuaternion baseRotate;
	baseRotate.makeRotate(180, 0, 1, 0);
	cam.targetNode.setOrientation(baseRotate);
	cam.targetXRot = 180;
	cam.targetYRot = 0;
	cam.targetZRot = 0;
	cam.applyRotation = true;
	cam.updateRotation();
	
	vector< vector<Point2f> > kinectImagePoints;
	vector< vector<Point2f> > externalRGBPoints;
	vector< vector<ofVec3f> > kinect3dPoints;
	vector< vector<Point3f> > objectPoints;
	
	vector<Point3f> filteredKinectObjectPoints;
	vector<Point2f> filteredExternalImagePoints;
	map<int,int> indexToBoard;
	
	saveCorrespondenceImages();
	
	inlierPoints.clear();
	
	int numAlignmentPairsReady = 0;
	for(int i = 0; i < alignmentPairs.size(); i++){
		if(alignmentPairs[i]->included &&
		   alignmentPairs[i]->depthPixelsRaw.isAllocated() &&
		   alignmentPairs[i]->depthCheckers.isAllocated() &&
		   alignmentPairs[i]->colorCheckers.isAllocated())
		{

			vector<Point2f> kinectPoints;
			bool foundBoard = rgbCalibration.findBoard(toCv(alignmentPairs[i]->depthCheckers), kinectPoints, true);
			if(!foundBoard){
				ofLogError("Correspondence Error") << "depth checkerboard " << (i+1) << " of " << alignmentPairs.size() << " cannot be found " << endl;
				continue;
			}
			
			vector<Point2f> externalPoints;
			//foundBoard = calibrationPreview.getCalibration().findBoard(toCv(alignmentPairs[i]->colorCheckers), externalPoints);
			foundBoard = rgbCalibration.findBoard(toCv(alignmentPairs[i]->colorCheckers), externalPoints, true);
			if(!foundBoard){
				//if we don't find the board, try scaling it to 1/4 size and looking...
				ofImage downscaled;
				downscaled.setUseTexture(false);
				downscaled.setFromPixels(alignmentPairs[i]->colorCheckers);
				downscaled.resize(downscaled.getWidth()/2, downscaled.getHeight()/2);
				
				foundBoard = rgbCalibration.findBoard(toCv(downscaled), externalPoints, true);
				if(!foundBoard){
					ofLogError("Correspondence Error") << "color checkerboard " << (i+1) << " of " << alignmentPairs.size() << " cannot be found " << endl;
					continue;
				}
				for(int x = 0; x < externalPoints.size(); x++){
					externalPoints[x].x *= 2;
					externalPoints[i].y *= 2;
				}
			}
			
			kinectImagePoints.push_back( kinectPoints );
			externalRGBPoints.push_back( externalPoints );
			vector<ofVec3f> new3dPoints;
			for(int j = 0; j < kinectPoints.size(); j++){
				unsigned short* pix = alignmentPairs[i]->depthPixelsRaw.getPixels();
				unsigned short z = pix[int(kinectPoints[j].x) + int(kinectPoints[j].y) * 640];
				ofVec3f worldPoint = depthToWorldFromCalibration(int(kinectPoints[j].x), int(kinectPoints[j].y), z);
				new3dPoints.push_back( worldPoint );
			}
			kinect3dPoints.push_back(new3dPoints);
			
			//treat the external cam as
			objectPoints.push_back( Calibration::createObjectPoints(cv::Size(10,7), squareSize, CHESSBOARD));
		}
	}
	
	if(objectPoints.size() == 0){
		ofSystemAlertDialog("No points to calibrate with found when calibrating");
		return;
	}
	
	int numPointsFound = 0;
	int boardIndex = 0;
	for(int i = 0; i < kinect3dPoints.size(); i++){
		
		while(boardIndex < 4 && !alignmentPairs[boardIndex]->included) boardIndex++;
		
		for(int j = 0; j < kinect3dPoints[i].size(); j++){
			if(kinect3dPoints[i][j].z > 0){
				//compensate for color offset in index -> color
				indexToBoard[filteredKinectObjectPoints.size()] = boardIndex;
				filteredKinectObjectPoints.push_back( toCv(kinect3dPoints[i][j]) );
				filteredExternalImagePoints.push_back( externalRGBPoints[i][j] );
				numPointsFound++;
			}
		}
		
		boardIndex++;
	}
	
	cout << "TRACKING DOWN MISSING 1 ** How many: " << inlierPoints.getVertices().size() << endl;
	
	cout << "Found a total of " << numPointsFound << " for correspondence " << endl;
	vector< vector<Point3f> > cvObjectPoints;
	vector< vector<Point2f> > cvImagePoints;
	
	cvObjectPoints.push_back( filteredKinectObjectPoints );
	cvImagePoints.push_back( filteredExternalImagePoints );
	Mat cameraMatrix;
	rgbCalibration.getDistortedIntrinsics().getCameraMatrix().copyTo(cameraMatrix);
	
	Mat distCoeffs;
	rgbCalibration.getDistCoeffs().copyTo(distCoeffs);
	
	Mat rotationDepthToRGB;
	Mat translationDepthToRGB;
	vector<Mat> rotations, translations;
	cout << "Initial RGB Camera Matrix " << cameraMatrix << endl;
	cout << "Initial RGB Distortion " << distCoeffs << endl;
	cout << "Camera image size " << rgbCalibration.getDistortedIntrinsics().getImageSize().width << " " << rgbCalibration.getDistortedIntrinsics().getImageSize().height << endl;
	
	
/*
	int flags = CV_CALIB_USE_INTRINSIC_GUESS | CV_CALIB_FIX_INTRINSIC | CV_CALIB_FIX_K1;
	double rms  = calibrateCamera(cvObjectPoints, cvImagePoints, rgbCalibration.getDistortedIntrinsics().getImageSize(), cameraMatrix, distCoeffs,rotations,translations, flags); //todo fix distortion
	rotationDepthToRGB = rotations[0];
	translationDepthToRGB = translations[0];
	cout << "RMS IS " << rms << endl;
*/


	Mat inliers;
	solvePnPRansac(filteredKinectObjectPoints, filteredExternalImagePoints,
				   cameraMatrix, distCoeffs,
				   rotationDepthToRGB, translationDepthToRGB,
				   false,//use intrinsics guess
				   100, //iterations
				   1.5, //reprojection error
				   numPointsFound, //min inliers
				   inliers ); //output inliers
	
	cout << "inliers " << inliers.total() << endl;

	for(int i= 0; i < inliers.total(); i++){
		inlierPoints.addColor( boardColors[ indexToBoard[ inliers.at<int>(i) ] ] );
		inlierPoints.addVertex( toOf(filteredKinectObjectPoints[inliers.at<int>(i)]) );
	}

	if(inliers.total() == 0){
		ofSystemAlertDialog("Calibration failed. Try including a different set of checkerboard triplets and trying again.");
		calibrationGenerated = false;
	}
	else{

		saveMat(rotationDepthToRGB, matrixDirectory + "rotationDepthToRGB.yml");
		saveMat(translationDepthToRGB, matrixDirectory + "translationDepthToRGB.yml");
		
		//save refined RGB
		Intrinsics rgbIntrinsics;
		rgbIntrinsics.setup(cameraMatrix, rgbCalibration.getDistortedIntrinsics().getImageSize());
		Calibration rgbCalibrationRefined;
		rgbCalibrationRefined.setIntrinsics(rgbIntrinsics, distCoeffs);
		rgbCalibrationRefined.save( matrixDirectory + "rgbCalib.yml");
		calibrationGenerated = true;
		
		setupRenderer();
	}

	cout << "Final camera matrix is " << cameraMatrix << endl;
	cout << "Final distortion coefficients " << distCoeffs << endl;
	cout << "Depth->RGB rotation " << rotationDepthToRGB << endl;
	cout << "Depth->RGB translation " << translationDepthToRGB << endl;
	
}

ofVec3f ofxRGBDCaptureGui::depthToWorldFromCalibration(int x, int y, unsigned short z){
	//	return ofVec3f(((x - pp.x) / 640) * z * fov.x, ((y - pp.y) / 480) * z * fov.y, z);
	return ofVec3f((x - pp.x) * z / fov.x, (y - pp.y) * z / fov.y, z);
}

void ofxRGBDCaptureGui::exit(ofEventArgs& args){
	calibrationPreview.quit();
	recorder.shutdown();
	if(providerSet){
		depthImageProvider->close();
		providerSet = false;
	}
}

//TODO move this to a shader
//COLORING
//taken from OpenNI
void ofxRGBDCaptureGui::createRainbowPallet() {
	unsigned char r, g, b;
	memset(LUTR, 0, 256);
	memset(LUTG, 0, 256);
	memset(LUTB, 0, 256);
	
	for (int i=1; i<255; i++) {
		if (i<=29) {
			r = (unsigned char)(129.36-i*4.36);
			g = 0;
			b = (unsigned char)255;
		}
		else if (i<=86) {
			r = 0;
			g = (unsigned char)(-133.54+i*4.52);
			b = (unsigned char)255;
		}
		else if (i<=141) {
			r = 0;
			g = (unsigned char)255;
			b = (unsigned char)(665.83-i*4.72);
		}
		else if (i<=199) {
			r = (unsigned char)(-635.26+i*4.47);
			g = (unsigned char)255;
			b = 0;
		}
		else {
			r = (unsigned char)255;
			g = (unsigned char)(1166.81-i*4.57);
			b = 0;
		}
		LUTR[i] = r;
		LUTG[i] = g;
		LUTB[i] = b;
	}
}

void ofxRGBDCaptureGui::updateDepthImage(ofShortPixels& pixels){
	
	if(!pixels.isAllocated()){
		return;
	}
	
	int max_depth = depthImageProvider->maxDepth();
	if(max_depth == 0){
		max_depth = 5000;
	}
	
	//	cout << "updating depth image with max depth of " << max_depth << " render: " << (currentRenderMode == RenderRainbow ? "rainbow" : "b&w") <<  endl;
	if(currentRenderMode == RenderRainbow){
		if(depthImage.getPixelsRef().getImageType() != OF_IMAGE_COLOR){
			depthImage.setImageType(OF_IMAGE_COLOR);
		}
		for(int i = 0; i < 640*480; i++){
			int lookup = pixels.getPixels()[i] / (max_depth / 256);
			//int lookup = ofMap( depthPixels.getPixels()[i], 0, max_depth, 0, 255, true);
			depthImage.getPixels()[(i*3)+0] = LUTR[lookup];
			depthImage.getPixels()[(i*3)+1] = LUTG[lookup];
			depthImage.getPixels()[(i*3)+2] = LUTB[lookup];
		}
	}
	else if(currentRenderMode == RenderBW){
		if(depthImage.getPixelsRef().getImageType() != OF_IMAGE_GRAYSCALE){
			depthImage.setImageType(OF_IMAGE_GRAYSCALE);
		}

		recorder.getCompressor().convertTo8BitImage(pixels, depthImage);
	}
	else{
		pointcloudPreview.setDepthImage(pixels);
		pointcloudPreview.update();
	}
	
	depthImage.update();	
}
