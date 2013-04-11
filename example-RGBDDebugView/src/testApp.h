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
 * The RGBDToolkit has been developed with support from the STUDIO for Creative Inquiry, Eyebeam, and YCAM InterLab
 */

#pragma once

#include "ofMain.h"
#include "ofxTimeline.h"
#include "ofxXmlSettings.h"
#include "ofxTLVideoTrack.h"
#include "ofxGameCamera.h"
#include "ofxRGBDPlayer.h"
#include "ofxRGBDGPURenderer.h"
#include "ofxTLDepthImageSequence.h"
#include "ofxGui.h"

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
    
    ofxRGBDPlayer player;
    ofxRGBDGPURenderer renderer;
    
    ofxGameCamera cam;
    ofxTimeline timeline;
    ofxTLVideoTrack videoTrack ;
	ofxTLDepthImageSequence depthTrack;
    ofxPanel panel;
	
	ofxToggle showTimeline;
	ofxToggle showDepthProjection;
	ofxToggle showDepthWireframe;
	ofxToggle showRGBProjection;
	ofxFloatSlider rgbTextureSlider;
	ofxFloatSlider xShift;
	ofxFloatSlider yShift;
	ofRectangle cameraRect;
	
    bool loadNewScene();
    bool loadDefaultScene();
    bool loadScene(string takeDirectory);
    
};
