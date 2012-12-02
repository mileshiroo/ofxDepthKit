/**
 * Example - Scene With Timeline
 * This example shows how to create a basic RGB+D scene
 * with a timeline to scrub through the video.
 *
 * It requiers ofxGameCamera https://github.com/Flightphase/ofxGameCamera 
 * and ofxTimeline https://github.com/Flightphase/ofxTimeline in addition to ofxRGBDepth
 *
 * James George 2012 
 * Released under the MIT License
 *
 * The RGBDToolkit has been developed with support from the STUDIO for Creative Inquiry and Eyebeam
 */

#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(255*.15);
    
    //set up a standard timeline with a default duration to begin with
    timeline.setup();
    timeline.getColors().load("defaultColors.xml");
	timeline.setOffset(ofVec2f(0,0));
    timeline.setDurationInFrames(300);
    timeline.setPageName("Main");
    
    //set up a video timeline element so we can see the video frames
    timeline.addTrack("Video", &videoTrack);
    timeline.addTrack("Depth", &depthTrack);
	
//	timeline.addCurves("xshift", "xshift.xml", ofRange(-.15, .15));
//  timeline.addCurves("yshift", "yshift.xml", ofRange(-.15, .15));
//  timeline.addCurves("farclip", "farclip.xml", ofRange(500, 6000));
	
	panel.setup("options");
	panel.add(showDepthProjection.setup("show depth projection", ofxParameter<bool>()));
	panel.add(showDepthWireframe.setup("show wireframe", ofxParameter<bool>()));
	panel.add(showRGBProjection.setup("show rgb projection", ofxParameter<bool>()));
	panel.add(rgbTextureSlider.setup("rgb texture projection", ofxParameter<float>(), 0, 1));
//	panel.add(xShift.setup("x shift",ofxParameter<float>(), -.15, .15));
//	panel.add(yShift.setup("y shift",ofxParameter<float>(), -.15, .15));

	xShift = 0.0045;
	yShift = 0.03;
	rgbTextureSlider = 0;
	
	
    //set up the game camera
    cam.setup();
	cam.dampen = true;
    cam.speed = 20;
    cam.autosavePosition = true;
    cam.targetNode.setPosition(ofVec3f());
    cam.targetNode.setOrientation(ofQuaternion());
    cam.targetXRot = -180;
    cam.targetYRot = 0;
    cam.rotationZ = 0;    

	player.updateVideoPlayer = false;
    //load 
    loadDefaultScene();
	
 	ofToggleFullscreen();
}

bool testApp::loadNewScene(){
    ofFileDialogResult r = ofSystemLoadDialog("Select a Scene", true);
    if(r.bSuccess){
        return loadScene(r.getPath());
    }
    return false;
}

bool testApp::loadDefaultScene(){
    ofxXmlSettings settings;
    if(settings.loadFile("RGBDSimpleSceneDefaults.xml")){
        if(!loadScene(settings.getValue("defaultScene", ""))){
            return loadNewScene();
        }
        return true;
    }
    return loadNewScene();
}

bool testApp::loadScene(string takeDirectory){
    if(player.setup(takeDirectory)){
        ofxXmlSettings settings;
        settings.loadFile("RGBDSimpleSceneDefaults.xml");
        settings.setValue("defaultScene", player.getScene().mediaFolder);
        settings.saveFile();

		
		renderer.farClip = 1250;
        renderer.setup(player.getScene().calibrationFolder);
        renderer.setRGBTexture(player.getVideoPlayer());
        renderer.setDepthImage(player.getDepthPixels());
		renderer.setSimplification(2);

        videoTrack.setPlayer(player.getVideoPlayer());
        depthTrack.setSequence(player.getDepthSequence());
        timeline.setDurationInFrames(player.getDurationInFrames());
		timeline.setTimecontrolTrack(&videoTrack);
		timeline.setFrameRate(player.getVideoPlayer()->getTotalNumFrames()/player.getVideoPlayer()->getDuration());
        return true;
    }
    return false;
}

//--------------------------------------------------------------
void testApp::update(){
    //don't rotate the camera if you are in the timeline
   // cam.applyRotation = cam.applyTranslation = !timeline.getDrawRect().inside(mouseX,mouseY);
    cam.applyRotation = cam.applyTranslation = cameraRect.inside(mouseX, mouseY);
    //apply the shift and clip parameters
//    renderer.farClip = timeline.getValue("farclip");
	
    player.update();
    if(player.isFrameNew() ||
	   renderer.xshift != xShift ||
	   renderer.yshift != yShift)
	{
		renderer.xshift = xShift;
		renderer.yshift = yShift;
        renderer.update();

    }
	
	cameraRect = ofRectangle(220, timeline.getBottomLeft().y,
							 ofGetWidth()-220, ofGetHeight() - timeline.getBottomLeft().y);
	
}

//--------------------------------------------------------------
void testApp::draw(){
    if(player.isLoaded()){
		ofPushStyle();
		ofSetColor(0);
		ofRect(cameraRect);
		ofPopStyle();
		
        cam.begin(cameraRect);
        glEnable(GL_DEPTH_TEST);
		if(showDepthWireframe){
			renderer.useTexture = rgbTextureSlider > .8;
			renderer.drawWireFrame();
		}
        glDisable(GL_DEPTH_TEST);
		renderer.drawProjectionDebug(showDepthProjection, showRGBProjection,rgbTextureSlider);
        cam.end();
    }

	
	timeline.draw();

	ofRectangle sideViewDraw(0, panel.getShape().getBottom(),panel.getShape().width, ofGetHeight() - panel.getShape().getBottom());
	ofRectangle colorDebug = ofRectangle(0,0,player.getVideoPlayer()->getWidth(),player.getVideoPlayer()->getHeight());
	colorDebug.scaleTo(sideViewDraw);
	colorDebug.y = panel.getShape().getBottom();
	ofRectangle depthDebug = ofRectangle(0,0,640,480);
	depthDebug.scaleTo(sideViewDraw);
	depthDebug.y = colorDebug.getBottom();
	
//	player.getVideoPlayer()->draw(colorDebug);
//	depthTrack.getCurrentDepthImage().draw(depthDebug);
//	player.getDepthSequence()->getCompressor().convertTo8BitImage(player.getDepthPixels()).draw(depthDebug);
	
	panel.setPosition(timeline.getBottomLeft());
	panel.draw();
    //ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), 10, ofGetHeight()-30);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key == ' '){
        timeline.togglePlay();
    }
	
	if(key == 'f'){
		ofToggleFullscreen();
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}