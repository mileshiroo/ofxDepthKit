#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	ofBackground(0);
	
	//this xml file was created by the visualizer app
	//exporting combined video. It contains all the info you need
	//to rebuild the pointcloud from the accompanying images
	ofxXmlSettings settings;
	settings.loadFile("_depthProperties.xml");
	
	depthImageWidth = settings.getValue("depth:width", 640);
	depthImageHeight = settings.getValue("depth:height", 480);
	principalPoint.x = settings.getValue("depth:ppx", 320.0);
	principalPoint.y = settings.getValue("depth:ppy", 240.0);
	fov.x = settings.getValue("depth:fovx", 570.34);
	fov.y = settings.getValue("depth:fovy", 570.34);
	maxDepth = settings.getValue("depth:maxDepth",2500);
	minDepth = settings.getValue("depth:minDepth",0);
	
	//let's build a simple point cloud
	//add one vertex to the mesh for each pixel
	for (int y = 0; y < depthImageHeight; y++){
		for (int x = 0; x < depthImageWidth; x++){
			mesh.addVertex(ofVec3f(x,y,0));	// mesh index = x + y*width
			mesh.addTexCoord(ofVec2f(x,y)); //color tex coords
		}
	}
	
	//load the test image of Elliot in this case
	combinedImage.loadImage("save.00130.png");
	
	//now update the mesh to fit the point cloud
	int index = 0;
	for (int y = 0; y < depthImageHeight; y++){
		for (int x = 0; x < depthImageWidth; x++){
			//position accesses the combinedImage in this case to distort the mesh based on the depth camera settings
			mesh.setVertex(index++, pointForPosition(x, y));
		}
	}
	
}

//--------------------------------------------------------------
ofVec3f testApp::pointForPosition(int x, int y){

	//remap the Hue encoding of the png to
	ofColor depthColor = combinedImage.getColor(x, y+depthImageHeight);
	//filter out unsaturated values
	int z;
	if(depthColor.getBrightness() < 200 || depthColor.getSaturation() < 200){
		z = 0;
	}
	else{
		z = ofMap(depthColor.getHue(), 0, 255, minDepth, maxDepth);
	}
	
	//minus 800 here is a fudge to center the pointcloud closer to the origin
	return ofVec3f( (x - principalPoint.x) * z / fov.x, (y - principalPoint.y) * z / fov.y, z - 800);
	
}

//--------------------------------------------------------------
void testApp::update(){
	
}

//--------------------------------------------------------------
void testApp::draw(){
	
	cam.begin();
	
	combinedImage.getTextureReference().bind();
	mesh.drawVertices();
	combinedImage.getTextureReference().unbind();
	
	cam.end();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

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