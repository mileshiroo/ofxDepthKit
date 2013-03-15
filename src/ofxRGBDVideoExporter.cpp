#include "ofxRGBDVideoExporter.h"

ofxRGBDVideoExporter::ofxRGBDVideoExporter(){
	minDepth = 0;
	maxDepth = 0;

	renderer = NULL;
	player = NULL;

	videoRectangle = ofRectangle(0,0,1280,720);
	
}

ofxRGBDVideoExporter::~ofxRGBDVideoExporter(){
	
}

void ofxRGBDVideoExporter::setRenderer(ofxRGBDCPURenderer* renderer){
	this->renderer = renderer;
}

void ofxRGBDVideoExporter::setPlayer(ofxRGBDPlayer* player){
	this->player = player;
}

void ofxRGBDVideoExporter::render(string outputPath, string clipName){
	
	if(player == NULL){
		ofLogError("ofxRGBDVideoExporter::render -- player is null");
		return;
	}

//	if(renderer == NULL){
//		ofLogError("ofxRGBDVideoExporter::render -- renderer is null");
//		return;
//	}

	outputImage.allocate(videoRectangle.getWidth() + 640, videoRectangle.getHeight(), OF_IMAGE_COLOR);
	
	for(int i = inoutPoint.min; i < inoutPoint.max; i++){
		
		//COPY video pixels into buffer
		ofPixels temp = player->getVideoPlayer()->getPixelsRef();
		temp.resize(videoRectangle.width, videoRectangle.height);
		temp.pasteInto(outputImage, 0, 0);

		ofShortPixels& p = player->getDepthPixels();
		for(int y = 0; y < p.getHeight(); y++){
			for(int x = 0; x < p.getWidth(); x++){
				outputImage.setColor(videoRectangle.getWidth() + x, y, getColorForZDepth(p.getPixels()[ p.getPixelIndex(x, y)] ));
			}
		}
		
		char filename[1024];
		sprintf(filename, "%s/%s%05d.png", outputPath.c_str(), clipName.c_str(), player->getVideoPlayer()->getCurrentFrame());
		ofSaveImage(outputImage, filename);
		
		player->getVideoPlayer()->nextFrame();
		player->getVideoPlayer()->update();
		player->update();
	}
}

ofColor ofxRGBDVideoExporter::getColorForZDepth(unsigned short z){
	if(z > maxDepth || z < minDepth){
		return ofColor(0,0,0);
	}
	
	float colorPoint = ofMap(z, minDepth, maxDepth, 0, 255, true);
	return ofColor::fromHsb(colorPoint, 255, 255);
}
