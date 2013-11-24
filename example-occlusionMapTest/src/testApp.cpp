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
    
	doOcclude = false;
    gui.setup("tests");
	gui.add(xshift.setup("xshift", ofParameter<float>(), -.65, .65));
    gui.add(yshift.setup("yshift", ofParameter<float>(), -.65, .65));
    gui.add(xsimplify.setup("xsimplify", ofParameter<float>(), 1, 8));
    gui.add(ysimplify.setup("ysimplify", ofParameter<float>(), 1, 8));
	gui.add(doOcclude.setup("occlude", ofParameter<bool>()));
	gui.add(scanLines.setup("scanlines", ofParameter<bool>()));
			
    gui.add(loadNew.setup("load new"));

        
    gui.loadFromFile("defaultSettings.xml");
    
    if(xsimplify < 1){
        xsimplify = 1;
    }
    if(ysimplify < 1){
        ysimplify = 1;
    }


	renderer.setShaderPath("shaders/unproject");
	
    //attemping to load the last scene
    loadDefaultScene();
	
	//occlusionMap.allocate(1920, 1080, GL_RGB);
	
	ofFbo::Settings fboSettings;
    fboSettings.width = 1920*.25;
    fboSettings.height = 1080*.25;
    fboSettings.internalformat = GL_RGB32F;
    fboSettings.numSamples = 0;
    fboSettings.useDepth = true;
//    fboSettings.useStencil = true;
//    fboSettings.depthStencilAsTexture = true;
    fboSettings.textureTarget = ofGetUsingArbTex() ? GL_TEXTURE_RECTANGLE_ARB : GL_TEXTURE_2D;
    occlusionMap.allocate( fboSettings );
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
void testApp::renderOcclusionMap(){

	occlusionMap.begin();
	ofClear(0, 0, 0);
	ofPushMatrix();
	ofPushView();
	ofPushStyle();
	glPushAttrib(GL_ENABLE_BIT);
	//ofEnableAlphaBlending();
	//glDisable(GL_DEPTH_TEST);
	
	ofDisableAlphaBlending();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	
	
	renderer.getRGBCalibration().getDistortedIntrinsics().loadProjectionMatrix();
//	ofMultMatrix(renderer.getAdjustedMatrix().getInverse());
	ofMultMatrix(renderer.getAdjustedMatrix());
	
	//ofViewport(1920*.25,1080*.25);
	
	renderer.mirror = false;
	renderer.bindRenderer();
	renderer.getShader().setUniform1i("renderOcclusionMap", 1);
	renderer.getShader().setUniform1i("doOcclude", 0);

	renderer.getMesh().drawFaces();

	renderer.unbindRenderer();
	renderer.mirror = true;
	
	ofPopStyle();
	glPopAttrib();
	ofPopView();
	ofPopMatrix();
	occlusionMap.end();

	
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
    //renderer.setXYShift(ofVec2f(xshift,yshift));
	renderer.colorMatrixRotate.x = xshift;
	renderer.colorMatrixRotate.y = yshift;
    renderer.setSimplification(ofVec2f(xsimplify,ysimplify));


    //update the mesh if there is a new depth frame in the player
    player.update();
    if(player.isFrameNew()){
        renderer.update();
    }
	
	renderOcclusionMap();
}

//--------------------------------------------------------------
void testApp::draw(){
    if(player.isLoaded()){
		if(showOccludeMap){
			occlusionMap.getTextureReference().draw(0, 0);
		}
		else{
			cam.begin();
			
			ofSetColor(255);
			glEnable(GL_DEPTH_TEST);
//			ofEnableBlendMode(OF_BLENDMODE_SCREEN);
			
			if(scanLines){
				ofMesh m;
				ofVec2f simp = renderer.getSimplification();
				for(int y = 0; y < 480; y += simp.y){
					for(int x = 0; x < 640; x += simp.x){
						m.addVertex(ofVec3f(x,y,0));
						m.addVertex(ofVec3f(x+simp.x,y,0));
					}
				}
				
				m.setMode(OF_PRIMITIVE_LINES);
				
				renderer.bindRenderer();
				m.draw();
				renderer.unbindRenderer();
			}
			else{
				//renderer.drawWireFrame();
				renderer.bindRenderer();
				renderer.getShader().setUniformTexture("occlusionTex", occlusionMap.getTextureReference(), 2);
				renderer.getShader().setUniform1i("doOcclude", doOcclude ? 1 : 0);
				renderer.getShader().setUniform1i("renderOcclusionMap", 0);
				renderer.getMesh().draw();
				renderer.unbindRenderer();
			}
			
			glDisable(GL_DEPTH_TEST);
			cam.end();
		}
		
    }

    gui.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key == ' '){
        player.togglePlay();
    }
	
	if(key == 'o'){
		doOcclude = !doOcclude;
	}
	if(key == 'R'){
		renderer.reloadShader();
	}
	
	if(key == 'M'){
		showOccludeMap = !showOccludeMap;
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
