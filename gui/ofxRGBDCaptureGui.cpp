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
//TODO: fix up file structure

#include "ofxRGBDCaptureGui.h"

ofxRGBDCaptureGui::ofxRGBDCaptureGui(){
    providerSet = false;
    calibrationGenerated = false;
    currentRendererPreviewIndex = 0;
    currentCalibrationImageIndex = 0;
    hoverPreviewDepth = false;
    hoverPreviewIR = false;
    hoverPreviewingCaptured = false;
}

ofxRGBDCaptureGui::~ofxRGBDCaptureGui(){
    
}

void ofxRGBDCaptureGui::setup(){
    
    framewidth = 640;
	frameheight = 480;
	thirdWidth = framewidth/3;
	btnheight = 36;
    margin = 12;

    currentAlignmentPair = new AlignmentPair();
	alignmentPairs.push_back(currentAlignmentPair);

    timeline.setup();
	timeline.setOffset(ofVec2f(0, frameheight+btnheight*2));
    timeline.addTrack("depth sequence", &depthSequence);
	timeline.setLoopType(OF_LOOP_NORMAL);
    timeline.hide();
    
//    downColor  = ofColor(255, 120, 0);
//	idleColor  = ofColor(220, 200, 200);
//	hoverColor = ofColor(255*.2, 255*.2, 30*.2);
    
    //TODO: make timeline colors
    downColor  = ofColor(255, 120, 0);
	idleColor  = ofColor(220, 200, 200);
	hoverColor = ofColor(255*.2, 255*.2, 30*.2);

    float totalFrameWidth = framewidth*2;
    float quarterWidth = totalFrameWidth * .25;
    
	btnSetDirectory = new ofxMSAInteractiveObjectWithDelegate();
	btnSetDirectory->setPosAndSize(0, 0, totalFrameWidth, btnheight);
	btnSetDirectory->setLabel("Load Directory");
	buttonSet.push_back(btnSetDirectory);
    
    btnIntrinsicsTab = new ofxMSAInteractiveObjectWithDelegate();
	btnIntrinsicsTab->setPosAndSize(0, btnheight, quarterWidth, btnheight);
	btnIntrinsicsTab->setLabel("Calibrate Lenses");
	buttonSet.push_back(btnIntrinsicsTab);
    currentTabObject = btnIntrinsicsTab;
    currentTab = TabIntrinsics;
    tabSet.push_back(btnIntrinsicsTab);
    
    btnExtrinsicsTab = new ofxMSAInteractiveObjectWithDelegate();
	btnExtrinsicsTab->setPosAndSize(quarterWidth, btnheight, quarterWidth, btnheight);
	btnExtrinsicsTab->setLabel("Calibrate Correspondence");
	buttonSet.push_back(btnExtrinsicsTab);
    tabSet.push_back(btnExtrinsicsTab);
    
	btnRecordTab = new ofxMSAInteractiveObjectWithDelegate();
	btnRecordTab->setPosAndSize(quarterWidth*2, btnheight, quarterWidth, btnheight);
	btnRecordTab->setLabel("Record");
	buttonSet.push_back(btnRecordTab);
    tabSet.push_back(btnRecordTab);
    
	btnPlaybackTab = new ofxMSAInteractiveObjectWithDelegate();
	btnPlaybackTab->setPosAndSize(quarterWidth*3, btnheight, quarterWidth, btnheight);
	btnPlaybackTab->setLabel("Playback");
	buttonSet.push_back(btnPlaybackTab);
    tabSet.push_back(btnPlaybackTab);
    
    btnCalibrateDepthCamera =  new ofxMSAInteractiveObjectWithDelegate();
    btnCalibrateDepthCamera->setPosAndSize(0, btnheight*2+frameheight, totalFrameWidth/2, btnheight*2);
    btnCalibrateDepthCamera->setLabel("Self-Calibrate Depth Camera");
    buttonSet.push_back(btnCalibrateDepthCamera);

    btnRGBLoadCalibration =  new ofxMSAInteractiveObjectWithDelegate();
    btnRGBLoadCalibration->setPosAndSize(totalFrameWidth/2, btnheight*2+frameheight, totalFrameWidth/2, btnheight*2);
    btnRGBLoadCalibration->setLabel("Load RGB Calibration Images");
    buttonSet.push_back(btnRGBLoadCalibration);
    
    //EXTRINSICS
    btnGenerateCalibration =  new ofxMSAInteractiveObjectWithDelegate();
    btnGenerateCalibration->setPosAndSize(0, btnheight*2+frameheight, totalFrameWidth, btnheight*2);
    btnGenerateCalibration->setLabel("Regenerate RGB/Depth Correspondence");
    btnGenerateCalibration->disableAllEvents();
    buttonSet.push_back(btnGenerateCalibration);
    
    //CAPTURE
    btnToggleRecord = new ofxMSAInteractiveObjectWithDelegate();
    btnToggleRecord->setPosAndSize(0, btnheight*2+frameheight, totalFrameWidth, btnheight*2);
    btnToggleRecord->setLabel("Toggle Record");
    btnToggleRecord->disableAllEvents();
    buttonSet.push_back(btnToggleRecord);

    for(int i = 0; i < buttonSet.size(); i++){
        buttonSet[i]->setIdleColor(idleColor);
        buttonSet[i]->setDownColor(downColor);
        buttonSet[i]->setHoverColor(hoverColor);
        buttonSet[i]->disableKeyEvents();
        buttonSet[i]->setDelegate(this);
        buttonSet[i]->fontReference = &timeline.getFont();
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
    
	ofxXmlSettings defaults;
	if(defaults.loadFile("defaults.xml")){
		loadDirectory(defaults.getValue("currentDir", ""));
	}
	else{
		loadDirectory("depthframes");
	}
	
	updateSceneButtons();
	
    cam.setup();
	cam.loadCameraPosition();

    createRainbowPallet();
    depthImage.allocate(640, 480, OF_IMAGE_GRAYSCALE);
    
    recorder.setup();

}

void ofxRGBDCaptureGui::setImageProvider(ofxDepthImageProvider* imageProvider){
	depthImageProvider = ofPtr<ofxDepthImageProvider>( imageProvider );
    depthImageProvider->setup();
    providerSet = true;
}


void ofxRGBDCaptureGui::update(ofEventArgs& args){
	if(!providerSet || !depthImageProvider->deviceFound()){
		return;
	}
	
	depthImageProvider->update();
    
	if(depthImageProvider->isFrameNew()){
        
        if(currentTab == TabCapture || currentTab == TabExtrinsics){
            updateDepthImage(depthImageProvider->getRawDepth());
        }

		if(recorder.isRecording()){
			recorder.addImage(depthImageProvider->getRawDepth());
		}
        else if(currentTab == TabExtrinsics || currentTab == TabIntrinsics){
            calibrationPreview.setTestImage( depthImageProvider->getRawIRImage() );
        }
	}
}

void ofxRGBDCaptureGui::setupRenderer(){
    renderer.reloadShader();
	renderer.setup("rgbCalibRefined.yml", "depthCalibRefined.yml", "rotationDepthToRGB.yml", "translationDepthToRGB.yml");
	
	renderer.setDepthImage(alignmentPairs[0]->depthPixelsRaw);
	renderer.setRGBTexture(alignmentPairs[0]->colorCheckers);
    cout << "color pixel dimension  " << alignmentPairs[0]->colorCheckers.getWidth() << " " << alignmentPairs[0]->colorCheckers.getHeight() << endl;
	renderer.update();
    
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
    
    timeline.draw();

}

void ofxRGBDCaptureGui::drawIntrinsics(){
    //left side is Kinect
    bool drawCamera = providerSet && depthImageProvider->deviceFound();
    if(drawCamera){
        depthImageProvider->getRawIRImage().draw( previewRectLeft );
    }
    else{
        ofPushStyle();
        ofSetColor(255, 0, 0);
        timeline.getFont().drawString("Camera not found. Plug and unplug the device and restart the application.", previewRectLeft.x + 30, previewRectLeft.y + 30);
        ofPopStyle();
    }
    
    //right side RGB
   	if (rgbCalibrationImages.size() > 0) {
		if(previewRectRight.inside(ofGetMouseX(), ofGetMouseY())){
			currentCalibrationImageIndex = ofMap(ofGetMouseX(), previewRectRight.getMinX(), previewRectRight.getMaxX(), 0, rgbCalibrationImages.size()-1, true);
		}
		rgbCalibrationImages[currentCalibrationImageIndex].draw(previewRectRight);
	}
	else{
		ofPushStyle();
		ofSetColor(255, 100, 10);
		ofRect(previewRectRight);
		ofPopStyle();
	}
}

void ofxRGBDCaptureGui::drawExtrinsics(){
    
    bool drawCamera = providerSet && depthImageProvider->deviceFound();

	int drawX = previewRectLeft.x;
	int drawY = previewRectLeft.y;
//	timeline.getFont().drawString("Show the checkerboard to the depth sensor and click to create correspondences", drawX, drawY);
//	drawY += timeline.getFont().getLineHeight();
	
	for(int i = 0; i < alignmentPairs.size(); i++){
		ofPushStyle();
		ofSetColor(255);
		ofNoFill();
        
        float subRectWidth  = framewidth*.25;
        float subRectHeight = frameheight*.25;
		alignmentPairs[i]->depthImageRect = ofRectangle(drawX, drawY, subRectWidth, subRectHeight);
		alignmentPairs[i]->depthCheckersRect = alignmentPairs[i]->depthImageRect;
        alignmentPairs[i]->depthCheckersRect.x += subRectWidth;
        
		alignmentPairs[i]->colorCheckersRect = alignmentPairs[i]->depthCheckersRect;
        alignmentPairs[i]->colorCheckersRect.x += subRectWidth;
        alignmentPairs[i]->colorCheckersRect.width *= (16./9.) / (4./3.); //aspect shift
        
		alignmentPairs[i]->deleteRect = ofRectangle(alignmentPairs[i]->colorCheckersRect.getMaxX(),
                                                    drawY,
                                                    framewidth - alignmentPairs[i]->colorCheckersRect.getMaxX(), subRectHeight/2);
        
        alignmentPairs[i]->includeRect = alignmentPairs[i]->deleteRect;
        alignmentPairs[i]->includeRect.y += subRectHeight/2;
                                                    
		ofRect(alignmentPairs[i]->depthImageRect);
		ofRect(alignmentPairs[i]->depthCheckersRect);
		ofRect(alignmentPairs[i]->colorCheckersRect);
        
		ofPopStyle();
        
		if(alignmentPairs[i]->depthImage.isAllocated()){
			alignmentPairs[i]->depthImage.draw(alignmentPairs[i]->depthImageRect);
		}
		if(alignmentPairs[i]->depthCheckers.isAllocated()){
			alignmentPairs[i]->depthCheckers.draw(alignmentPairs[i]->depthCheckersRect);
		}
		
		if(alignmentPairs[i]->colorCheckers.isAllocated()){
			alignmentPairs[i]->colorCheckers.draw(alignmentPairs[i]->colorCheckersRect);
		}
		
        if(alignmentPairs[i] != currentAlignmentPair){
            
            ofPushStyle();
			ofSetColor(255, 100, 100);
			ofRect(alignmentPairs[i]->deleteRect);
            
			ofSetColor(255, 100, 100);
			ofRect(alignmentPairs[i]->includeRect);
			ofPopStyle();

            ofPushStyle();
            ofSetColor(255);
            ofNoFill();
            ofRect(alignmentPairs[i]->deleteRect);
			ofRect(alignmentPairs[i]->includeRect);
            ofPopStyle();
            
		}
        
		drawY += subRectHeight;
	}
	
    
    //TODO draw image in whatever is hovering
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
    else if(calibrationGenerated){
        ofSetColor(255);
        glEnable(GL_DEPTH_TEST);
        cam.begin(previewRectRight);
        renderer.drawMesh();
        cam.end();
        glDisable(GL_DEPTH_TEST);
    }
    else{
        ofPushStyle();
        ofNoFill();
        ofRect(previewRectRight);
        //TODO: add need GENERATE calibration error message
        
        ofPopStyle();
    }
    
}

void ofxRGBDCaptureGui::drawCapture(){
    bool drawCamera = providerSet && depthImageProvider->deviceFound();
    if(drawCamera){
        depthImage.draw(previewRectLeft);
    }
}

void ofxRGBDCaptureGui::drawPlayback(){
    if(currentRenderMode == RenderPointCloud){
        //drawPointcloud(depthSequence.getDepthImageSequence()->getPixels(), false);
    }
    else {
        if(depthSequence.getDepthImageSequence() != NULL && depthSequence.getDepthImageSequence()->isLoaded()){
            updateDepthImage(depthSequence.getDepthImageSequence()->getPixels());
        }
        depthImage.draw(previewRectLeft);
    }    
}

void ofxRGBDCaptureGui::objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y){
    
}

void ofxRGBDCaptureGui::objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y){
    
}

void ofxRGBDCaptureGui::objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button){
    
}

void ofxRGBDCaptureGui::objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button){
  	if(object == btnSetDirectory){
		loadDirectory();
	}
    else if(object == btnCalibrateDepthCamera){
        refineDepthCalibration();
    }
    else if(object == btnRGBLoadCalibration){
        //TODO: Load RGB dialog helper
    }
    else if(object == btnGenerateCalibration){
        generateCorrespondence();
    }
    else if(find(tabSet.begin(),tabSet.end(), object) != tabSet.end()){
        
        btnRGBLoadCalibration->disableAllEvents();
        btnCalibrateDepthCamera->disableAllEvents();
        btnGenerateCalibration->disableAllEvents();
        btnToggleRecord->disableAllEvents();
        timeline.hide();
        
        if(object == btnIntrinsicsTab){
            currentTab = TabIntrinsics;
            currentTabObject = btnIntrinsicsTab;
            btnRGBLoadCalibration->enableAllEvents();
            btnCalibrateDepthCamera->enableAllEvents();
        }
        else if(object == btnExtrinsicsTab){
            currentTab = TabExtrinsics;
            currentTabObject = btnExtrinsicsTab;
            btnGenerateCalibration->enableAllEvents();
        }
        else if(object == btnRecordTab){
            currentTab = TabCapture;
            currentTabObject = btnRecordTab;
            btnToggleRecord->enableAllEvents();
        }
        else if(object == btnPlaybackTab){
            currentTab = TabPlayback;
            currentTabObject = btnPlaybackTab;
            timeline.show();
        }
    }
}

void ofxRGBDCaptureGui::objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y){
    
}

void ofxRGBDCaptureGui::mousePressed(ofMouseEventArgs& args){
    
}

void ofxRGBDCaptureGui::mouseMoved(ofMouseEventArgs& args){
	cam.applyRotation = cam.applyTranslation = (currentTab == TabExtrinsics && previewRectRight.inside(args.x, args.y));

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
                    calibrationPreview.setTestImage(alignmentPairs[i]->depthCheckers);
                    hoverPreviewingCaptured = true;
                }
            }
            if(alignmentPairs[i]->depthCheckersRect.inside(args.x,args.y)){
                if(alignmentPairs[i] == currentAlignmentPair && !alignmentPairs[i]->depthCheckers.isAllocated()){
                    hoverPreviewIR = true;
                }
                else{
                    hoverPreviewImage.setFromPixels(alignmentPairs[i]->depthCheckers.getPixelsRef());
                    calibrationPreview.setTestImage(alignmentPairs[i]->depthCheckers);
                    hoverPreviewingCaptured = true;
                }
            }
            
            if(alignmentPairs[i]->colorCheckersRect.inside(args.x,args.y)){
                //TODO check for colors to set and make it work within the given size
            }
        }
    }
}

void ofxRGBDCaptureGui::mouseDragged(ofMouseEventArgs& args){
    
}

void ofxRGBDCaptureGui::mouseReleased(ofMouseEventArgs& args){
    
    //CHECK FOR HOVER IN INTRINSICS
    if(currentTab == TabIntrinsics){
        
    }
    else if(currentTab == TabExtrinsics){
        //CHECK FOR HOVERING ON EXTRINSICS
        for(int i = alignmentPairs.size()-1; i >= 0; i--){
            if(alignmentPairs[i]->depthImageRect.inside(args.x,args.y)){
                //CAPTURE DEPTH IMAGE
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
                alignmentPairs[i]->depthCheckers.setFromPixels(depthImageProvider->getRawIRImage());
                hoverPreviewImage.setFromPixels(alignmentPairs[i]->depthCheckers.getPixelsRef());
                //preview the still of what you just clicked
                hoverPreviewIR = false;
                hoverPreviewingCaptured = true;
                break;
            }
            if(alignmentPairs[i]->colorCheckersRect.inside(args.x,args.y)){
                //TODO: load checker movie from dialog box
                break;
            }
            if(alignmentPairs[i] != currentAlignmentPair){
                
                if(alignmentPairs[i]->deleteRect.inside(args.x,args.y)){
                    if(currentRendererPreviewIndex == i){
                        //TOOD: fix this problem
                    }

                    delete alignmentPairs[i];
                    alignmentPairs.erase(alignmentPairs.begin() + i);
                    break;
                }
                
                if(alignmentPairs[i]->includeRect.inside(args.x, args.y)){
                    alignmentPairs[i]->included = !alignmentPairs[i]->included;
                }
            }
        }
        
        if(currentAlignmentPair->depthImage.isAllocated() && currentAlignmentPair->depthCheckers.isAllocated()){
            currentAlignmentPair = new AlignmentPair();
            alignmentPairs.push_back(currentAlignmentPair);
        }
    }
}

void ofxRGBDCaptureGui::keyPressed(ofKeyEventArgs& args){
    
}

void ofxRGBDCaptureGui::keyReleased(ofKeyEventArgs& args){
    
}

void ofxRGBDCaptureGui::loadDirectory(){
    if(recorder.numFramesWaitingCompession() != 0){
    	ofSystemAlertDialog("Cannot change directory while files are converting");
		return;
    }
    
	ofFileDialogResult r = ofSystemLoadDialog("Select Record Directory", true);
	if(r.bSuccess){
		loadDirectory(r.getPath());
	}
}

void ofxRGBDCaptureGui::loadDirectory(string path){
    
    if(path == ""){
        ofSystemAlertDialog("The Working directory empty. Resetting to bin/data/");
        loadDefaultDirectory();
        return;
    }
    
    ofDirectory workingDirPath(path);
    if(!workingDirPath.exists()){
        ofSystemAlertDialog("The Working directory empty. Resetting to bin/data/depthframes/");
        loadDefaultDirectory();
        return;
    }
    
    workingDirectory = path;
	recorder.setRecordLocation(path, "frame");
	ofDirectory dir(workingDirectory+"/_calibration/depthCalibration");
	if(!dir.exists()){
		dir.create(true);
	}
//	alignment.loadState(workingDirectory+"/_calibration/alignmentsave.xml");
	
	btnSetDirectory->setLabel(path);
	updateSceneButtons();
	ofxXmlSettings defaults;
	defaults.loadFile("defaults.xml");
	defaults.setValue("currentDir", path);
	defaults.saveFile("defaults.xml");
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
	
	for(int i = 0; i < scenes.size(); i++){
        SceneButton tb;
        tb.isSelected = false;
        tb.sceneRef = scenes[i];
        
		ofxMSAInteractiveObjectWithDelegate* btnScene = new ofxMSAInteractiveObjectWithDelegate();
		float x = framewidth;
		float y = btnheight*.66*i;
		while(y >= btnheight*3+frameheight){
			y -= btnheight*3+frameheight;
			x += thirdWidth;
		}
		
		btnScene->setPosAndSize(x, y, thirdWidth, btnheight*.66);
        btnScene->setLabel( scenes[i]->name );
		btnScene->setIdleColor(idleColor);
		btnScene->setDownColor(downColor);
		btnScene->setHoverColor(hoverColor);
		btnScene->setDelegate(this);
        btnScene->enableMouseEvents();
        btnScene->disableKeyEvents();
        
		tb.button = btnScene;
		btnScenes.push_back( tb );
	}
}

void ofxRGBDCaptureGui::dragEvent(ofDragInfo& dragInfo){
	string filename = dragInfo.files[0];
	string extension = ofToLower(ofFilePath::getFileExt(filename));
    
	bool draggedIntoImageRect =  currentTab == TabIntrinsics && previewRectRight.inside( dragInfo.position );
    
	if(draggedIntoImageRect) {
		cout << "calibrating color camera" << endl;
		//snag shots from all the videos and create a calibration
		if(extension == "yml"){
			rgbCalibration.load(filename);
		}
		else {
			rgbCalibrationImages.clear();
            
            //TODO: check if it's a directory;
            //TODO add search for PNG's
			for(int i = 0; i < dragInfo.files.size(); i++){
				ofVideoPlayer p;
				filename = dragInfo.files[i];

				extension = ofToLower(ofFilePath::getFileExt(filename));
				if(extension == "mov" || extension == "mp4"){
					if(p.loadMovie(filename)){
						ofImage image;
						p.setPosition(.5);
						image.setFromPixels(p.getPixelsRef());
						image.setImageType(OF_IMAGE_GRAYSCALE);
						if(rgbCalibration.add(toCv(image))){
							rgbCalibrationImages.push_back(image);
							cout << filename << " added " << endl;
						}
						else {
                            
                            vector<Point2f> pointBuf;
                            image.resize(image.getWidth()/2, image.getHeight()/2);
                            if(rgbCalibration.findBoard(toCv(image), pointBuf)){
                                ofLogWarning("RGBCalibration") << "Found board on second try " << filename;
                                for(int i = 0; i < pointBuf.size(); i++){
                                    pointBuf[i] *= 2;
                                }
                                rgbCalibration.imagePoints.push_back(pointBuf);
                                image.resize(image.getWidth()*2, image.getHeight()*2);
                                rgbCalibrationImages.push_back(image);
                            }
                            else{
                                ofLogError("RGBCalibration") << "Could not find checkerboard in image " << filename;
                            }
						}
					}
				}

			}
            
            //If we successfully calibrated
			if(rgbCalibrationImages.size() > 3){
				rgbCalibration.calibrate();
				if(rgbCalibration.isReady()){
					rgbCalibration.save("rgbCalibBase.yml");
					for(int i = 0; i < rgbCalibrationImages.size(); i++){
						rgbCalibration.undistort( toCv(rgbCalibrationImages[i]) );
						rgbCalibrationImages[i].update();
					}
				}
			}
		}
	}
    
    if(currentTab == TabExtrinsics){
        if(extension == "png"){
            for(int i = 0; i < alignmentPairs.size(); i++){
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
            }
            
            if(currentAlignmentPair->depthImage.isAllocated() && currentAlignmentPair->depthCheckers.isAllocated()){
                currentAlignmentPair = new AlignmentPair();
                alignmentPairs.push_back(currentAlignmentPair);
            }
        }
        
        if(extension == "mov" || extension == "mp4"){
            for(int i = 0; i < alignmentPairs.size(); i++){
                if(alignmentPairs[i]->colorCheckersRect.inside(dragInfo.position)){
                    ofVideoPlayer p;
                    p.setUseTexture(false);
                    p.loadMovie(filename);
                    p.setPosition(.5);
                    alignmentPairs[i]->colorCheckers.setFromPixels(p.getPixelsRef());
                    alignmentPairs[i]->colorCheckers.setImageType(OF_IMAGE_GRAYSCALE);
                }
            }
        }
    }
}


void ofxRGBDCaptureGui::refineDepthCalibration(){
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
	depthCalibrationRefined.save("depthCalibRefined.yml");
    
    depthCameraMatrix = depthCalibrationRefined.getDistortedIntrinsics().getCameraMatrix();
    fov = ofVec2f(depthCameraMatrix.at<double>(0,0), depthCameraMatrix.at<double>(1,1));
    cout << "fov " << fov << endl;
    pp = ofVec2f(depthCameraMatrix.at<double>(0,2),depthCameraMatrix.at<double>(1,2));
    cout << "principle point " << pp << endl;    
}

void ofxRGBDCaptureGui::generateCorrespondence(){
    if(!depthCalibrationRefined.isReady() || !rgbCalibration.isReady()){
        ofSystemAlertDialog("You cannot generate a correspondence before you've calibrated both the RGB and Kinect cameras");
		return;
	}
    
	kinect3dPoints.clear();
	kinectImagePoints.clear();
	externalRGBPoints.clear();
	objectPoints.clear();
	filteredKinectObjectPoints.clear();
	filteredExternalImagePoints.clear();
    
	int numAlignmentPairsReady = 0;
	for(int i = 0; i < alignmentPairs.size(); i++){
		if(alignmentPairs[i]->depthPixelsRaw.isAllocated() &&
		   alignmentPairs[i]->depthCheckers.isAllocated() &&
		   alignmentPairs[i]->colorCheckers.isAllocated())
		{
            
            //save the correspondnce images
			recorder.getCompressor().saveToCompressedPng("correspondence_"+ofToString(i)+"_depth_pixels.png", alignmentPairs[i]->depthPixelsRaw.getPixels());
			alignmentPairs[i]->depthCheckers.saveImage("correspondence_"+ofToString(i)+"_checkers.png");
			alignmentPairs[i]->colorCheckers.saveImage("correspondence_"+ofToString(i)+"_color_checkers.png");
            
			vector<Point2f> kinectPoints;
			bool foundBoard = depthCalibrationRefined.findBoard(toCv(alignmentPairs[i]->depthCheckers), kinectPoints);
			if(!foundBoard){
				ofLogError("Correspondence Error") << "depth checkerboard " << (i+1) << " of " << alignmentPairs.size() << " cannot be found " << endl;
				continue;
			}
			
			vector<Point2f> externalPoints;
			foundBoard = calibrationPreview.getCalibration().findBoard(toCv(alignmentPairs[i]->colorCheckers), externalPoints);
			if(!foundBoard){
                //if we don't find the board, try scaling it to 1/4 size and looking...
                ofImage downscaled;
                downscaled.setUseTexture(false);
                downscaled.setFromPixels(alignmentPairs[i]->colorCheckers);
                downscaled.resize(downscaled.getWidth()/2, downscaled.getHeight()/2);
                
                foundBoard = calibrationPreview.getCalibration().findBoard(toCv(downscaled), externalPoints);
                if(!foundBoard){
                    ofLogError("Correspondence Error") << "color checkerboard " << (i+1) << " of " << alignmentPairs.size() << " cannot be found " << endl;
                    continue;
                }
                for(int i = 0; i < externalPoints.size(); i++){
                    externalPoints[i].x *= 2;
                    externalPoints[i].y *= 2;
                }
			}
			kinectImagePoints.push_back( kinectPoints );
			externalRGBPoints.push_back( externalPoints );
			vector<ofVec3f> new3dPoints;
			for(int j = 0; j < kinectPoints.size(); j++){
				unsigned short z = alignmentPairs[i]->depthPixelsRaw[kinectPoints[j].x+kinectPoints[j].y*640];
				ofVec3f worldPoint = depthToWorldFromCalibration(kinectPoints[j].x, kinectPoints[j].y, z);
				new3dPoints.push_back( worldPoint );
			}
			kinect3dPoints.push_back(new3dPoints);
			
			//treat the external cam as
			objectPoints.push_back( Calibration::createObjectPoints(cv::Size(10,7), 3.2, CHESSBOARD));
			
		}
	}
    
    if(objectPoints.size() == 0){
        return;
    }
	
	int numPointsFound = 0;
	for(int i = 0; i < kinect3dPoints.size(); i++){
		for(int j = 0; j < kinect3dPoints[i].size(); j++){
			if(kinect3dPoints[i][j].z > 0){
                //				cout << "World point is " << kinect3dPoints[i][j] << " for checkerboard image point " << externalRGBPoints[i][j] << endl;
                
				filteredKinectObjectPoints.push_back( toCv(kinect3dPoints[i][j]) );
				filteredExternalImagePoints.push_back( externalRGBPoints[i][j] );
				numPointsFound++;
			}
		}
	}
	
	cout << "Found a total of " << numPointsFound << " for correspondence " << endl;
	vector< vector<Point3f> > cvObjectPoints;
	vector< vector<Point2f> > cvImagePoints;
    
	cvObjectPoints.push_back( filteredKinectObjectPoints );
	cvImagePoints.push_back( filteredExternalImagePoints );
	
	Mat cameraMatrix = rgbCalibration.getDistortedIntrinsics().getCameraMatrix();
	Mat distCoeffs = rgbCalibration.getDistCoeffs();
	Mat rotationDepthToRGB;
	Mat translationDepthToRGB;
    
	vector<Mat> rotations, translations;
	cout << "Initial RGB Camera Matrix " << cameraMatrix << endl;
	cout << "Initial RGB Distortion " << distCoeffs << endl;
	cout << "Camera image size " << rgbCalibration.getDistortedIntrinsics().getImageSize().width << " " << rgbCalibration.getDistortedIntrinsics().getImageSize().height << endl;
    //	int flags = CV_CALIB_FIX_INTRINSIC;
    //	int flags = CV_CALIB_USE_INTRINSIC_GUESS | CV_CALIB_FIX_INTRINSIC;
	//double rms  = calibrateCamera(cvObjectPoints, cvImagePoints, cv::Size(rgbCalibration.getDistortedIntrinsics().getImageSize()), cameraMatrix, distCoeffs,rotations,translations, flags); //todo fix distortion
    Mat inliers;
	solvePnPRansac(filteredKinectObjectPoints, filteredExternalImagePoints,
                   cameraMatrix, distCoeffs,
                   rotationDepthToRGB, translationDepthToRGB,
                   false,//use intrinsics guess
                   100, //iterations
                   3, //reprojection error
                   numPointsFound, //min inliers
                   inliers ); //output inliers
    cout << "inliers " << inliers.total() << endl;
    //    cout << inl.size() << endl;
    
    //	rotationDepthToRGB = rotations[0];
    //	translationDepthToRGB = translations[0];
	
	//cvFindExtrinsicCameraParams2(cvOjectPoints[0], cvImagePoints[0], cameraMatrix, distCoeffs,rotations,translations);
    //	cout << "Calibrate RMS is " << rms << endl;
	cout << "Final camera matrix is " << cameraMatrix << endl;
	cout << "Final distortion coefficients " << distCoeffs << endl;
	cout << "RGB->Depth rotation " << rotationDepthToRGB << endl;
	cout << "RGB->Detph translation " << translationDepthToRGB << endl;
	
	saveMat(rotationDepthToRGB, "rotationDepthToRGB.yml");
	saveMat(translationDepthToRGB, "translationDepthToRGB.yml");
	
	//save refined RGB
	Intrinsics rgbIntrinsics;
	rgbIntrinsics.setup(cameraMatrix, rgbCalibration.getDistortedIntrinsics().getImageSize());
	Calibration rgbCalibrationRefined;
	rgbCalibrationRefined.setIntrinsics(rgbIntrinsics, distCoeffs);
	rgbCalibrationRefined.save("rgbCalibRefined.yml");
	
	setupRenderer();

}

ofVec3f ofxRGBDCaptureGui::depthToWorldFromCalibration(int x, int y, unsigned short z){
    //    return ofVec3f(((x - pp.x) / 640) * z * fov.x, ((y - pp.y) / 480) * z * fov.y, z);
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
    //    cout << "updating depth image with max depth of " << max_depth << " render: " << (currentRenderMode == RenderRainbow ? "rainbow" : "b&w") <<  endl;
    if(currentRenderMode == RenderRainbow){
        for(int i = 0; i < 640*480; i++){
            int lookup = pixels.getPixels()[i] / (max_depth / 256);
            //int lookup = ofMap( depthPixels.getPixels()[i], 0, max_depth, 0, 255, true);
            depthImage.getPixels()[(i*3)+0] = LUTR[lookup];
            depthImage.getPixels()[(i*3)+1] = LUTG[lookup];
            depthImage.getPixels()[(i*3)+2] = LUTB[lookup];
        }
    }
    else{
        recorder.getCompressor().convertTo8BitImage(pixels, depthImage);
    }
    
    depthImage.update();	
}
