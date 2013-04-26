//
//  ofxRGBDCombinedVideoExporter.cpp
//  RGBDVisualize
//
//  Created by James George on 4/26/13.
//
//

#include "ofxRGBDCombinedVideoExporter.h"

ofxRGBDCombinedVideoExporter::ofxRGBDCombinedVideoExporter(){
	minDepth = 0;
	maxDepth = 2500;
}

ofxRGBDCombinedVideoExporter::~ofxRGBDCombinedVideoExporter(){
	
}

ofPixelsRef ofxRGBDCombinedVideoExporter::getPixels(){
	return pixels;
}

void ofxRGBDCombinedVideoExporter::updatePixels(ofxRGBDCPURenderer& mesh, ofBaseHasPixels& colorPixels){
	
	if(!pixels.isAllocated()){
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

ofColor ofxRGBDCombinedVideoExporter::huePixelForDepth(unsigned short x){
	return ofColor::fromHsb(ofMap(x, minDepth, maxDepth, 0, 255), 255, 255);
}