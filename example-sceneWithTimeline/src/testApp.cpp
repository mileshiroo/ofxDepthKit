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
    ofBackground(25);
    
    //set up a standard timeline with a default duration to begin with
    timeline.setup();
    timeline.getColors().loadColors("defaultColors.xml");
	timeline.setOffset(ofVec2f(0,0));
    timeline.setDurationInFrames(300);
    timeline.setPageName("Main");
    
    //set up a video timeline element so we can see the video frames
    videoTimelineElement = new ofxTLVideoPlayer();
    timeline.addElement("Video", videoTimelineElement);
	timeline.addKeyframes("xshift", "xshift.xml", ofRange(-.15, .15));
    timeline.addKeyframes("yshift", "yshift.xml", ofRange(-.15, .15));
    timeline.addKeyframes("farclip", "farclip.xml", ofRange(500, 6000));
    
    //set up the game camera
    cam.setup();
    cam.speed = 20;
    cam.autosavePosition = true;
    cam.targetNode.setPosition(ofVec3f());
    cam.targetNode.setOrientation(ofQuaternion());
    cam.targetXRot = -180;
    cam.targetYRot = 0;
    cam.rotationZ = 0;    

    //load 
    loadDefaultScene();
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
        return loadScene(settings.getValue("defaultScene", ""));
    }
    return loadNewScene();
}

bool testApp::loadScene(string takeDirectory){
    if(player.setup(takeDirectory)){
        ofxXmlSettings settings;
        settings.loadFile("RGBDSimpleSceneDefaults.xml");
        settings.setValue("defaultScene", player.getScene().mediaFolder);
        settings.saveFile();

        videoTimelineElement->setVideoPlayer(player.getVideoPlayer(), player.getScene().videoThumbsPath);
        timeline.setDurationInFrames(player.getDurationInFrames());
    }
}

//--------------------------------------------------------------
void testApp::update(){
    cam.applyRotation = cam.applyTranslation = !timeline.getDrawRect().inside(mouseX,mouseY);
    player.getRenderer().xshift = timeline.getKeyframeValue("xshift");
	player.getRenderer().yshift = timeline.getKeyframeValue("yshift");
    player.getRenderer().farClip = timeline.getKeyframeValue("farclip");
    player.update();
}

//--------------------------------------------------------------
void testApp::draw(){
    if(player.isLoaded()){
        cam.begin();
        glEnable(GL_DEPTH_TEST);
        player.drawWireframe();
        glDisable(GL_DEPTH_TEST);
        cam.end();
    }

    timeline.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key == ' '){
        player.togglePlay();
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
	timeline.setWidth(w);
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}