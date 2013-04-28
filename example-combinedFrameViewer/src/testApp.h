#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class testApp : public ofBaseApp{
  public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
	
	ofEasyCam cam;
	
	ofVboMesh mesh;
	ofImage combinedImage;
	
	ofVec2f fov;
	ofVec2f principalPoint;
	float depthImageWidth;
	float depthImageHeight;
	float minDepth;
	float maxDepth;
	
	
	//given a position on the depth image x, y
	//return a point in the 640 x 480 range in 3d
	ofVec3f pointForPosition(int x, int y);
};
