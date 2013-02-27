
#pragma once

#include "ofMain.h"
#include "ofxMSAInteractiveObjectDelegate.h"
#include "ofxGameCamera.h"
#include "ofxTimeline.h"
#include "ofxTLDepthImageSequence.h"
#include "ofxDepthImageCompressor.h"
#include "ofxDepthImageProvider.h"
#include "ofxDepthImageRecorder.h"
#include "ofxCvCheckerboardPreview.h"
#include "ofxRGBDAlignment.h"
#include "ofxDepthImageProvider.h"
#include "ofxRGBDScene.h"
#include "ofxRGBDGPURenderer.h"

typedef enum {
    TabIntrinsics,
	TabExtrinsics,
	TabCapture,
	TabPlayback
} RecorderTab;

typedef enum {
	RenderBW,
	RenderRainbow,
	RenderPointCloud
} DepthRenderMode;

typedef struct {
	ofxRGBDScene* sceneRef;
    ofxMSAInteractiveObjectWithDelegate* button;
    bool isSelected;
} SceneButton;

typedef struct{
	ofRectangle depthImageRect;
	ofImage depthImage;
	ofShortPixels depthPixelsRaw;
    
	ofRectangle depthCheckersRect;
	ofImage depthCheckers;
    
	ofRectangle colorCheckersRect;
	ofImage colorCheckers;
    
	ofRectangle includeRect;
	ofRectangle deleteRect;
    bool included;
    
} AlignmentPair;

class ofxRGBDCaptureGui : public ofxMSAInteractiveObjectDelegate {
public:
	ofxRGBDCaptureGui();
    ~ofxRGBDCaptureGui();

    void setup();
    void setImageProvider(ofxDepthImageProvider* imageProvider);
    void update(ofEventArgs& args);
  	void draw(ofEventArgs& args);

    void mousePressed(ofMouseEventArgs& args);
    void mouseMoved(ofMouseEventArgs& args);
    void mouseDragged(ofMouseEventArgs& args);
    void mouseReleased(ofMouseEventArgs& args);
    
    void keyPressed(ofKeyEventArgs& args);
    void keyReleased(ofKeyEventArgs& args);
    
    void objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y);
	void objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y);
	void objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button);
	void objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button);
	void objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y);
    
    void dragEvent(ofDragInfo& dragInfo);
    
    void exit(ofEventArgs& args);
    
  protected:
    ofPtr<ofxDepthImageProvider> depthImageProvider;
    ofxTimeline timeline;
	ofxTLDepthImageSequence depthSequence;
    ofxDepthImageRecorder recorder;
	ofxCvCheckerboardPreview calibrationPreview;
   	ofxGameCamera cam;
    
    bool providerSet;

    string workingDirectory;

    RecorderTab currentTab;
	DepthRenderMode currentRenderMode;

    ofRectangle previewRectLeft;
    ofRectangle previewRectRight;
 
    ofxMSAInteractiveObjectWithDelegate* currentTabObject;
    vector<ofxMSAInteractiveObjectWithDelegate*> buttonSet; //all non scene buttons
    
    //TOP LEVEL FOLDER
    ofxMSAInteractiveObjectWithDelegate* btnSetDirectory;
    
    //TABS
	ofxMSAInteractiveObjectWithDelegate* btnIntrinsicsTab;
    ofxMSAInteractiveObjectWithDelegate* btnExtrinsicsTab;
	ofxMSAInteractiveObjectWithDelegate* btnRecordTab;
	ofxMSAInteractiveObjectWithDelegate* btnPlaybackTab;

    //INTRINSICS
    ofxMSAInteractiveObjectWithDelegate* btnRGBLoadCalibration;
    ofxMSAInteractiveObjectWithDelegate* btnCalibrateDepthCamera;
    
    //EXTRINSICS
    ofxMSAInteractiveObjectWithDelegate* btnGenerateCalibration;
    
    //CAPTURE
    ofxMSAInteractiveObjectWithDelegate* btnToggleRecord;

    vector<ofxMSAInteractiveObjectWithDelegate*> tabSet;
    vector<SceneButton> btnScenes;
    
    //main drawing functions
    void drawIntrinsics();
    void drawExtrinsics();
    void drawCapture();
    void drawPlayback();
    
    void loadDirectory();
	void loadDirectory(string path);
	void loadDefaultDirectory();
    
	bool loadSequenceForPlayback( int index );
	void updateSceneButtons();
	
	void toggleRecord();

    ofColor downColor;
	ofColor idleColor;
	ofColor hoverColor;
    
	float framewidth;
	float frameheight;
	float thirdWidth;
	float btnheight;
	float sceneWidth;
	float margin;


    //Preview
    void updateDepthImage(ofShortPixels& pixels);
    ofImage depthImage;
    void createRainbowPallet();
	unsigned char LUTR[256];
	unsigned char LUTG[256];
	unsigned char LUTB[256];
    
    //INTRINSICS
	Calibration rgbCalibration;
    Calibration depthCalibrationBase;
	Calibration depthCalibrationRefined;

    vector<ofImage> rgbCalibrationImages;
    int currentCalibrationImageIndex;
    
    void refineDepthCalibration();
    
    //EXTRINSICS
    vector<AlignmentPair*> alignmentPairs;
	AlignmentPair* currentAlignmentPair;
    void generateCorrespondence();
    
    ofVec2f fov;
    ofVec2f pp;
    ofVec3f depthToWorldFromCalibration(int x, int y, unsigned short z);
    
    vector< vector<Point2f> > kinectImagePoints;
    vector< vector<Point2f> > externalRGBPoints;
    vector< vector<ofVec3f> > kinect3dPoints;
    vector< vector<Point3f> > objectPoints;
    
    vector<Point3f> filteredKinectObjectPoints;
	vector<Point2f> filteredExternalImagePoints;
    
    
    //CALIBRATION PREVIEW
    void setupRenderer();
    bool calibrationGenerated;
    int currentRendererPreviewIndex;
    ofxRGBDGPURenderer renderer;
    
};