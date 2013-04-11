#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	ofBackground(0);
	
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	kinect.open();

	renderer.setDepthImage(kinect.getRawDepthPixelsRef());

	renderer.setDepthOnly();
	
	renderer.setSimplification(ofVec2f(10,10));
}

//--------------------------------------------------------------
void testApp::update(){
	kinect.update();
	
	if(kinect.isFrameNew()) {

		renderer.edgeClip = mouseX*5;
		renderer.farClip = mouseY*2;
		
		renderer.update();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	easyCam.begin();
	
	ofPushMatrix();
	ofTranslate(0,0,-800);
	renderer.drawWireFrame();
	ofPopMatrix();
	
	easyCam.end();
	
	kinect.drawDepth(0, 0, 320,240);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

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