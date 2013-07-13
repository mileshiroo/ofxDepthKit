//
//  ofxRGBDCombinedVideoExporter.cpp
//  RGBDVisualize
//
//  Created by James George on 4/26/13.
//
//

#include "ofxRGBDCombinedVideoExporter.h"
#include "ofRange.h"

ofxRGBDCombinedVideoExporter::ofxRGBDCombinedVideoExporter(){
	minDepth = 0;
	maxDepth = 2500;
	oneToOne = true;
	frameSize = 720;
}

ofxRGBDCombinedVideoExporter::~ofxRGBDCombinedVideoExporter(){
	
}

ofPixelsRef ofxRGBDCombinedVideoExporter::getPixels(){
	return pixels;
}

void ofxRGBDCombinedVideoExporter::updatePixels(ofxRGBDCPURenderer& mesh, ofBaseHasPixels& colorPixels){
	
	if(oneToOne){
		
		if(!pixels.isAllocated() ||
		   pixels.getWidth() != 640 ||
		   pixels.getHeight() != 480*2)
		{
			pixels.allocate(640, 480*2, OF_IMAGE_COLOR_ALPHA);
		}
		
		memset(pixels.getPixels(), 0, pixels.getWidth()*pixels.getHeight()*4);
		for(int i = 0; i < mesh.validVertIndices.size(); i++){
			
			pair<int, int> depthPixelCoord = mesh.getPixelLocationForIndex( mesh.validVertIndices[i] );
			
			//set rgb color
			ofVec2f texcoord = mesh.getMesh().getTexCoords()[ mesh.validVertIndices[i] ];
			ofColor rgbPixelColor = colorPixels.getPixelsRef().getColor(texcoord.x, texcoord.y);
			pixels.setColor(depthPixelCoord.first, depthPixelCoord.second, rgbPixelColor);
			
			//set depth color
			int depthPixelIndex = mesh.getDepthImage().getPixelIndex(depthPixelCoord.first, depthPixelCoord.second);
			ofColor depthPixelColor = huePixelForDepth(mesh.getDepthImage().getPixels()[depthPixelIndex]);
			pixels.setColor(depthPixelCoord.first, depthPixelCoord.second + + pixels.getHeight()/2, depthPixelColor);
		}
	}
	else {
		ofRectangle videoRectangle = ofRectangle(0,0,frameSize * 16./9.,frameSize);
		if(!pixels.isAllocated() ||
		   pixels.getWidth() != videoRectangle.getWidth() + 640 ||
		   pixels.getHeight() != videoRectangle.getHeight())
		{
			pixels.allocate(videoRectangle.getWidth() + 640, videoRectangle.getHeight(), OF_IMAGE_COLOR);
		}

		memset(pixels.getPixels(), 0, pixels.getWidth()*pixels.getHeight()*3);
		//COPY video pixels into buffer
		ofPixels resizedVideoPixels = colorPixels.getPixelsRef();
		resizedVideoPixels.resize(videoRectangle.width, videoRectangle.height, OF_INTERPOLATE_BICUBIC);
		resizedVideoPixels.pasteInto(pixels, 0, 0);
		
		ofShortPixels& p = mesh.getDepthImage();
		ofRectangle depthBox;
		depthBox.x = videoRectangle.getWidth();
		depthBox.y = 0;
		depthBox.width = p.getWidth();
		depthBox.height = p.getHeight();
		
		for(int y = mesh.topClip*depthBox.height; y < mesh.bottomClip*depthBox.height; y++){
			for(int x = mesh.leftClip*depthBox.width; x < mesh.rightClip*depthBox.width; x++){
				pixels.setColor(x + depthBox.x,
								y + depthBox.y,
								huePixelForDepth(p.getPixels()[ p.getPixelIndex(x, y)]));
			}
		}
	}
}

ofColor ofxRGBDCombinedVideoExporter::huePixelForDepth(unsigned short z){
	if(z == 0 || z < minDepth || z > maxDepth ) return 0;
	return ofColor::fromHsb(ofMap(z, minDepth, maxDepth, 0, 255, true), 255, 255);
}

