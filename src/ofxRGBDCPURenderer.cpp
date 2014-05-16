//
//  ofxRGBDCPURenderer.cpp
//  RGBDVisualize
//
//  Created by James George on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "ofxRGBDCPURenderer.h"

using namespace ofxCv;

ofxRGBDCPURenderer::ofxRGBDCPURenderer(){
	
	depthOnly = false;


	calibrationSetup = false;
	calculateTextureCoordinates = true;
	normalizeTextureCoordinates = false;
	hasTriangles = false;
	
	textureScale = ofVec2f(1.0, 1.0);
	scale = ofVec2f(1,1);
	
	cacheValidVertices = false;
	
	center = ofVec3f(0,0,0);
	pivot = ofVec3f(0,0,0);
}

ofxRGBDCPURenderer::~ofxRGBDCPURenderer(){
}

void ofxRGBDCPURenderer::setSimplification(ofVec2f simplification){
	
	if(!calibrationSetup && !depthOnly){
		return;
	}
	
	if(simplify == simplification){	
		return;
	}
	
	if(simplification.x <= 0  || simplification.y <= 0){
		return;
	}

	simplify = simplification;
	simplify.x = ofClamp(simplify.x, 0, 8);
	simplify.y = ofClamp(simplify.y, 0, 8);
	
	baseIndeces.clear();
	int x = 0;
	int y = 0;
	
	int gw = ceil(depthImageSize.width / simplify.x);
	int w = gw*simplify.x;
	int h = depthImageSize.height;
	
	for (float ystep = 0; ystep < h-simplify.y; ystep += simplify.y){
		for (float xstep = 0; xstep < w-simplify.x; xstep += simplify.x){
			ofIndexType a,b,c;
			a = x+y*gw;
			b = (x+1)+y*gw;
			c = x+(y+1)*gw;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);
			
			a = (x+1)+(y+1)*gw;
			b = x+(y+1)*gw;
			c = (x+1)+(y)*gw;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);
			x++;
		}
		
		y++;
		x = 0;
	}
	
	indexToPixelCoord.clear();
	mesh.clearVertices();
	for (float y = 0; y < depthImageSize.height; y += simplify.y){
		for (float x = 0; x < depthImageSize.width; x += simplify.x){
			indexToPixelCoord[ mesh.getVertices().size() ] = make_pair(x,y);
			mesh.addVertex(ofVec3f(x,y,0));
		}
	}
	
	if(addColors){
		mesh.clearColors();
		for (float y = 0; y < depthImageSize.height; y += simplify.y){
			for (float x = 0; x < depthImageSize.width; x += simplify.x){
				mesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
			}
		}		
	}
	
	if(calculateTextureCoordinates){
		mesh.clearTexCoords();
		for (float y = 0; y < depthImageSize.height; y+=simplify.y){
			for (float x = 0; x < depthImageSize.width; x+=simplify.x){
				mesh.addTexCoord(ofVec2f(0,0));
			}
		}
	}
	meshGenerated = true;
//	cout << "set simplification level to " << simplifyLevel << " for image size " << imageSize.width << " x " << imageSize.height << " #verts " << mesh.getNumVertices() << endl;
}

void ofxRGBDCPURenderer::update(){
	
	if(!calibrationSetup && !depthOnly){
	 	ofLogError("ofxRGBDCPURenderer::updateMesh() -- no calibration set up");
		return;
	}
	
	if(currentDepthImage == NULL || !currentDepthImage->isAllocated()){
		ofLogError("ofxRGBDCPURenderer::update") << "depth pix are not allocated";
		return;
	}

	if(currentDepthImage->getWidth() != depthImageSize.width ||
	   currentDepthImage->getHeight() != depthImageSize.height)
	{
		ofLogError("ofxRGBDCPURenderer::update") << "depth pix dimensions don't match, provided " << ofVec2f(currentDepthImage->getWidth(), currentDepthImage->getHeight()) << " expecting " << ofVec2f(depthImageSize.width,depthImageSize.height);
		return;
	}
	
	
	//feed the zed values into the mesh
	int vertexIndex = 0;
	hasTriangles = false;
	validVertIndices.clear();
	int discardedPoints = 0;
	unsigned short* ptr = currentDepthImage->getPixels();
	for(float y = 0; y < depthImageSize.height; y += simplify.y) {
		for(float x = 0; x < depthImageSize.width; x += simplify.x) {
			
			ofVec3f point;
			unsigned short z = ptr[int(y) * int(depthImageSize.width) + int(x)];
			if(x >= depthImageSize.width*leftClip &&
			   x <= depthImageSize.width*rightClip &&
			   y >= depthImageSize.height*topClip &&
			   y <= depthImageSize.height*bottomClip &&
			   z > nearClip &&
			   z <= farClip)
			{
				point = getWorldPoint(x,y,z);
				if(cacheValidVertices){
					validVertIndices.push_back(vertexIndex);
				}
			}
			else {
				discardedPoints++;
				//cout << "discarded point at " << x << " " << y << endl;
				point = ofVec3f(0,0,0);
			}
			mesh.setVertex(vertexIndex++, point);
		}
	}

	mesh.clearIndices();
	for(int i = 0; i < baseIndeces.size(); i+=3){

		ofVec3f& a = mesh.getVertices()[baseIndeces[i]];
		if(a.z == 0) continue;
		ofVec3f& b = mesh.getVertices()[baseIndeces[i+1]];
		if(b.z == 0) continue;
		ofVec3f& c = mesh.getVertices()[baseIndeces[i+2]];
		if(c.z == 0) continue;
		
		if(fabs(a.z - b.z) > edgeClip || fabs(a.z - c.z) > edgeClip){
			continue;
		}

		mesh.addTriangle(baseIndeces[i], baseIndeces[i+1], baseIndeces[i+2]);
		hasTriangles = true;
	}

//	cout << "has triangles? " << mesh.getNumIndices() << endl;
	if(hasRGBImage && calculateTextureCoordinates && !depthOnly){
		generateTextureCoordinates();
	}
	
	//cout << "updated mesh has " << mesh.getNumIndices()/3 << " triangles " << endl;
}

void ofxRGBDCPURenderer::getReducedMesh(ofMesh& mesh,ofMatrix4x4 mat){
	getReducedMesh(mesh,false,false,false,mat);
}

void ofxRGBDCPURenderer::getReducedMesh(ofMesh& reducedMesh,
										bool normalizeTextureCoords,
										bool flipTextureX,
										bool flipTextureY,
										ofMatrix4x4 matrix)
{

	if(!cacheValidVertices){
		ofLogError("ofxRGBDCPURenderer::getReducedMesh -- Must cache valid verts to get the reduced mesh");
	}
	ofIndexType initialIndex = reducedMesh.getNumVertices();
	map<ofIndexType, ofIndexType> vertMapping;
	for(int i = 0; i < validVertIndices.size(); i++){
		vertMapping[ validVertIndices[i] ] = i;
		//reducedMesh.addVertex(  mesh.getVertices()[ validVertIndices[i] ] * matrix);
		reducedMesh.addVertex(  matrix.preMult( mesh.getVertices()[ validVertIndices[i] ] ) );
		if(mesh.hasTexCoords() && calculateTextureCoordinates && currentRGBImage != NULL){
			ofVec2f& coord = mesh.getTexCoords()[ validVertIndices[i] ] ;
			if(flipTextureX){
				coord.x = currentRGBImage->getTextureReference().getWidth() - coord.x;
			}
			if(flipTextureY){
				coord.y = currentRGBImage->getTextureReference().getHeight() - coord.y;
			}
			
			if(normalizeTextureCoords){
				coord /= ofVec2f(currentRGBImage->getTextureReference().getWidth(),
								 currentRGBImage->getTextureReference().getHeight() );
			}
			
			reducedMesh.addTexCoord(coord);
			
		}
		if(mesh.hasColors()){
			reducedMesh.addColor( mesh.getColors()[ validVertIndices[i] ] );
		}
	}
	
	for(int i = 0; i < mesh.getNumIndices(); i++){
		reducedMesh.addIndex( initialIndex + vertMapping[ mesh.getIndex(i) ] );
	}
}

void ofxRGBDCPURenderer::generateTextureCoordinates(){
	generateTextureCoordinates(mesh.getVertices(), mesh.getTexCoords());
}

ofVec2f ofxRGBDCPURenderer::getTextureCoordinateForPoint(ofVec3f point){
	vector<ofVec3f> points;
	vector<ofVec2f> tex;
	points.push_back(point);
	generateTextureCoordinates(points, tex);
	return tex[0];
}

void ofxRGBDCPURenderer::generateTextureCoordinates(vector<ofVec3f>& points, vector<ofVec2f>& texCoords){
	if(!calibrationSetup){
		ofLogError("ofxRGBDRenderer::generateTextureCoordinates -- no calibration set up");
		return;
	}
	
	if(points.size() == 0){
		ofLogError("ofxRGBDRenderer::generateTextureCoordinates -- no points to generate");
		return;
	}

	texCoords.clear();
	texCoords.resize(points.size());
	for(int i = 0; i < points.size(); i++) {

		ofVec4f worldPos = ofVec4f(points[i].x,points[i].y,points[i].z, 1.0);
		
		//http://opencv.willowgarage.com/documentation/camera_calibration_and_3d_reconstruction.html
		ofVec2f dim = ofVec2f(colorImageSize.width, colorImageSize.height);
		//vec3 projection = colorRotate * pos.xyz + colorTranslate + ;
		ofVec2f oldShift = shift*dim / textureScale;
		ofVec4f projection = worldPos * getAdjustedMatrix() + ofVec4f(oldShift.x,oldShift.y,0,0);
		if(projection.z != 0.0) {
			
			ofVec2f xyp = ofVec2f(projection.x, projection.y) / projection.z;
			float r2 = powf(xyp.x, 2.0) + powf(xyp.y, 2.0);
			float r4 = r2*r2;
			float r6 = r4*r2;
			ofVec2f xypp = xyp;
			xypp.x = xyp.x * (1.0 + distortionK.x*r2 + distortionK.y*r4 + distortionK.z*r6) + 2.0*distortionP.x * xyp.x * xyp.y + distortionP.y*(r2 + 2.0 * powf(xyp.x,2.0) );
			xypp.y = xyp.y * (1.0 + distortionK.x*r2 + distortionK.y*r4 + distortionK.z*r6) + distortionP.x * (r2 + 2.0*powf(xyp.y, 2.0) ) + 2.0*distortionP.y*xyp.x*xyp.y;
			ofVec2f uv = (colorFOV * xypp + colorPrincipalPoint);
			uv = ((uv-dim/2.0) * scale) + dim/2.0;
			uv *= textureScale;
			texCoords[i].x = ofClamp(uv.x, 0, currentRGBImage->getTextureReference().getWidth()-1);
			texCoords[i].y = ofClamp(uv.y, 0, currentRGBImage->getTextureReference().getHeight()-1);
		}
		else {
			texCoords[i] = ofVec2f(0,0);
		}
	}

	/*
	 vec4 projection = extrinsics * pos + vec4(shift*dim / textureScale,0,0);
	 
	 if(projection.z != 0.0) {
	 
	 vec2 xyp = projection.xy / projection.z;
	 float r2 = pow(xyp.x, 2.0) + pow(xyp.y, 2.0);
	 float r4 = r2*r2;
	 float r6 = r4*r2;
	 vec2 xypp = xyp;
	 xypp.x = xyp.x * (1.0 + dK.x*r2 + dK.y*r4 + dK.z*r6) + 2.0*dP.x * xyp.x * xyp.y + dP.y*(r2 + 2.0 * pow(xyp.x,2.0) );
	 xypp.y = xyp.y * (1.0 + dK.x*r2 + dK.y*r4 + dK.z*r6) + dP.x * (r2 + 2.0*pow(xyp.y, 2.0) ) + 2.0*dP.y*xyp.x*xyp.y;
	 vec2 uv = (colorFOV * xypp + colorPP) * textureScale;
	 texCd.xy = ((uv-dim/2.0) * scale) + dim/2.0;
	 }
	 */
	
	/*
	projectPoints(objectPoints,
				  rotationDepthToRGB, translationDepthToRGB,
				  rgbCalibration.getDistortedIntrinsics().getCameraMatrix(),
				  rgbCalibration.getDistCoeffs(),
				  imagePoints);
	
	//TODO turn into matrix transform!
	cv::Size rgbImage = rgbCalibration.getDistortedIntrinsics().getImageSize();
	ofVec2f dims = ofVec2f(rgbImage.width,rgbImage.height);
	
//	cout << "REPORT:: " << endl;
//	cout << "	IMAGE SIZE:	" << dims << endl;
//	cout << "	SHIFT:		" << shift << endl;
//	cout << "	SCALE:		" << scale << endl;
//	cout << "	TEX SCALE:	" << textureScale << endl;
//	cout << "	DISTORTION " << rgbCalibration.getDistCoeffs() << endl;
//	cout << "	CAM MATRIX " << rgbCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
//	cout << "::END REPORT" << endl;
	
	for(int i = 0; i < imagePoints.size(); i++) {
		ofVec2f texCd = ofVec2f(imagePoints[i].x, imagePoints[i].y);
		texCd /= dims;
		texCd += shift;
		texCd = ( (texCd - .5)*scale) + .5;
		texCd *= dims * textureScale;
		texCd.x = ofClamp(texCd.x, 0, currentRGBImage->getTextureReference().getWidth()-1);
		texCd.y = ofClamp(texCd.y, 0, currentRGBImage->getTextureReference().getHeight()-1);
		texCoords.push_back(texCd);
	}
	 */
}

ofVec3f ofxRGBDCPURenderer::getWorldPoint(ofVec2f uv){
	return getWorldPoint(uv.x,uv.y);
}

ofVec3f ofxRGBDCPURenderer::getWorldPoint(float x, float y){
	if(currentDepthImage == NULL) return ofVec3f(0,0,0);
	return getWorldPoint(x,y,*currentDepthImage);
}

ofVec3f ofxRGBDCPURenderer::getWorldPoint(ofVec2f uv, ofShortPixels& pixels){
	return getWorldPoint(uv.x, uv.y, pixels);
}

ofVec3f ofxRGBDCPURenderer::getWorldPoint(float x, float y, ofShortPixels& pixels){
	unsigned short z =  pixels.getPixels()[ pixels.getPixelIndex(x, y) ];
	return getWorldPoint(x,y,z);
}

ofVec3f ofxRGBDCPURenderer::getWorldPoint(float x, float y, unsigned short z){
	return ofVec3f( (x - depthPrincipalPoint.x) * z / depthFOV.x,
					(y - depthPrincipalPoint.y) * z / depthFOV.y, z);
}

pair<int,int> ofxRGBDCPURenderer::getPixelLocationForIndex(ofIndexType index){
	return indexToPixelCoord[index];
}

void ofxRGBDCPURenderer::updateCenter(){
	center = ofVec3f(0,0,0);
	int vertsAdded = 0;
	for(int i = 0; i < mesh.getVertices().size(); i++){
		if(mesh.getVertices()[i] != ofVec3f(0,0,0) && mesh.getVertices()[i].z < farClip){
			center += mesh.getVertices()[i];
			vertsAdded++;
		}
	}
	center /= vertsAdded;
	center.y *= -1;
}

void ofxRGBDCPURenderer::setPivotToMeshCenter(){
	updateCenter();
	pivot = center;
}

void ofxRGBDCPURenderer::setupDrawMatrices(){
	ofTranslate(worldPosition);
	
	ofTranslate(pivot);
	ofRotate(worldRotation.x, 1, 0, 0);
	ofRotate(worldRotation.y, 0, 1, 0);
	ofRotate(worldRotation.z, 0, 0, 1);
	ofTranslate(-pivot);
	
	ofScale(1,-1, 1);
	if(!mirror){
		ofScale(-1,1, 1);
	}
}

void ofxRGBDCPURenderer::draw(ofPolyRenderMode drawMode){
	
	if(!calibrationSetup && !depthOnly){
		ofLogError("ofxRGBDCPURenderer::draw -- Failed. Calibration is not set up");
		return;
	}
	
	if(!hasTriangles){
//		ofLogError("ofxRGBDCPURenderer::draw -- Failed. Mesh has no geometry");
		return;
	}

	ofPushMatrix();
	setupDrawMatrices();
	if(!depthOnly && currentRGBImage != NULL){
		currentRGBImage->getTextureReference().bind();
	}
	
	switch(drawMode){
		case OF_MESH_POINTS:
			mesh.drawVertices(); break;
		case OF_MESH_WIREFRAME:
			mesh.drawWireframe(); break;
		case OF_MESH_FILL:
			mesh.drawFaces();
			break;
	}
	
	if(!depthOnly && currentRGBImage != NULL){
		currentRGBImage->getTextureReference().unbind();
	}
	
	ofPopMatrix();
}
