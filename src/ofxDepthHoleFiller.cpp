/*
 *  ofxDepthHoleFiller.cpp
 *
 */

#include "ofxDepthHoleFiller.h"

using namespace cv;
using namespace ofxCv;

ofxDepthHoleFiller::ofxDepthHoleFiller(){
	enable = true;
	kernelSize = 3;
	iterations = 4;
}

int ofxDepthHoleFiller::setIterations(int newIterations){
	return iterations = ofClamp(newIterations, 1, 20);
}

int ofxDepthHoleFiller::setKernelSize(int newKernelSize){
	kernelSize = ofClamp(newKernelSize, 1, 20);	
	
	if(kernelSize % 2 == 0){
		kernelSize++;
	}
	return kernelSize;
}

int ofxDepthHoleFiller::getIterations(){
	return iterations;
}

int ofxDepthHoleFiller::getKernelSize(){
	return kernelSize;
}

void ofxDepthHoleFiller::close(ofShortPixels& depthPixels){
	if(enable){
		Mat original = toCv(depthPixels);
		Mat filledMask;
		Mat dilated;
		
		Mat m_element_m = getStructuringElement(MORPH_RECT, cv::Size(kernelSize, kernelSize));
		morphologyEx(original, dilated, MORPH_CLOSE, m_element_m, cv::Point(-1,-1), iterations);
		
		cv::compare(original, 0, filledMask, CMP_EQ);
		cv::add(original, dilated, original, filledMask);		
	
	}	
}