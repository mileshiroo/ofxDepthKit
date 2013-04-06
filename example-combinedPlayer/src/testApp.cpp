/**
 * Example - Mesh Builder
 * This example shows how to create a RGBD Mesh on the CPU
 *
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
    
    //set up the game camera
    cam.setup();
    cam.speed = 20;
    cam.autosavePosition = true;
    cam.targetNode.setPosition(ofVec3f());
    cam.targetNode.setOrientation(ofQuaternion());
    cam.targetXRot = -180;
    cam.targetYRot = 0;
    cam.rotationZ = 0;    
    
	xsimplify = 1;
    ysimplify = 1;
    xshift = 0;
    yshift = 0;
    
    gui.setup("tests");
    gui.add(xshift.setup("xshift", ofxParameter<float>(), -.15, .15));
    gui.add(yshift.setup("yshift", ofxParameter<float>(), -.15, .15));
    gui.add(xsimplify.setup("xsimplify", ofxParameter<float>(), 1, 8));
    gui.add(ysimplify.setup("ysimplify", ofxParameter<float>(), 1, 8));
    gui.add(loadNew.setup("load new"));
    gui.add(flipTexture.setup("flip texture", ofxParameter<bool>()));
        
    gui.loadFromFile("defaultSettings.xml");
    
    if(xsimplify < 1){
        xsimplify = 1;
    }
    if(ysimplify < 1){
        ysimplify = 1;
    }
    
    //attemping to load the last scene
    loadDefaultScene();
}

//--------------------------------------------------------------
bool testApp::loadNewScene(){
    ofFileDialogResult r = ofSystemLoadDialog("Select a Scene", true);
    if(r.bSuccess){
        return loadScene(r.getPath());
    }
    return false;
}

//--------------------------------------------------------------
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

//--------------------------------------------------------------
bool testApp::loadScene(string takeDirectory){
    if(player.setup(takeDirectory)){
        ofxXmlSettings settings;
        settings.loadFile("RGBDSimpleSceneDefaults.xml");
        settings.setValue("defaultScene", player.getScene().mediaFolder);
        settings.saveFile();
        renderer.setup(player.getScene().calibrationFolder);
        
        //populate
        player.getVideoPlayer()->setPosition(.5);
        player.update();
        
        renderer.setXYShift(player.getXYShift());
		renderer.setRGBTexture(*player.getVideoPlayer());
		renderer.setDepthImage(player.getDepthPixels());
        
        return true;
    }
    return false;
}

//--------------------------------------------------------------
void testApp::update(){
    if(loadNew){
        loadNewScene();
    }
    
    //copy any GUI changes into the mesh
    renderer.setXYShift(ofVec2f(xshift,yshift));
    renderer.setSimplification(ofVec2f(xsimplify,ysimplify));
    renderer.flipTexture = flipTexture;

    //update the mesh if there is a new depth frame in the player
    player.update();
    if(player.isFrameNew()){
        renderer.update();
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    if(player.isLoaded()){
        cam.begin();
		ofSetColor(255);
        glEnable(GL_DEPTH_TEST);
		ofEnableBlendMode(OF_BLENDMODE_SCREEN);
        renderer.drawWireFrame();        
		glDisable(GL_DEPTH_TEST);
        cam.end();
    }

    gui.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key == ' '){
        player.togglePlay();
    }
}

//--------------------------------------------------------------
void testApp::exit(){
    gui.saveToFile("defaultSettings.xml");
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
	ofDirectory dir(dragInfo.files[0]);
	if( dir.isDirectory() && ofxRGBDScene::isFolderValid(dragInfo.files[0]) ){
		loadScene(dragInfo.files[0]);
	}
}
