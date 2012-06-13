/*
 *  ofxRGBDRenderer.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 12/17/11.
 *  Copyright 2011 FlightPhase. All rights reserved.
 *
 */

#include "ofxRGBDRenderer.h"
#include <set>

ofxRGBDRenderer::ofxRGBDRenderer(){
	xshift = 0;
	yshift = 0;
	
	edgeCull = 4000;
	simplify = 1;

	farClip = 6000;
    meshRotate = ofVec3f(0,0,0);
    
    calculateNormals = false;
    
	hasDepthImage = false;
	hasRGBImage = false;

//    shaderBound = false;
    rendererBound = false;
    currentlyBoundShader = NULL;

	mirror = false;
	calibrationSetup = false;
    
    reloadShader();
    setSimplification(1);
    
  //  hasVerts = false;
    forceUndistortOff = false;
    addColors = false;
    calculateTextureCoordinates = false;
    currentDepthImage = NULL;
}

ofxRGBDRenderer::~ofxRGBDRenderer(){

}

bool ofxRGBDRenderer::setup(string calibrationDirectory){
	
	if(!ofDirectory(calibrationDirectory).exists()){
		ofLogError("ofxRGBDRenderer --- Calibration directory doesn't exist: " + calibrationDirectory);
		return false;
	}
	
	depthCalibration.load(calibrationDirectory+"/depthCalib.yml");
	rgbCalibration.load(calibrationDirectory+"/rgbCalib.yml");
	
	loadMat(rotationDepthToRGB, calibrationDirectory+"/rotationDepthToRGB.yml");
	loadMat(translationDepthToRGB, calibrationDirectory+"/translationDepthToRGB.yml");
    
    depthToRGBView = ofxCv::makeMatrix(rotationDepthToRGB, translationDepthToRGB);

    ofPushView();
    rgbCalibration.getUndistortedIntrinsics().loadProjectionMatrix();
    glGetFloatv(GL_PROJECTION_MATRIX, rgbProjection.getPtr());
    ofPopView();

    Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
	fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	fy = tanf(ofDegToRad(fov.y) / 2) * 2;

    calibrationSetup = true;
	return true;
}

void ofxRGBDRenderer::setSimplification(int level){
	simplify = level;
	if (simplify <= 0) {
		simplify = 1;
	}
	else if(simplify > 8){
		simplify = 8;
	}
	
    simplify = 1;
    
	baseIndeces.clear();
    simpleMesh.clearIndices();
	int w = 640 / simplify;
	int h = 480 / simplify;
	for (int y = 0; y < h-1; y++){
		for (int x=0; x < w-1; x++){
			ofIndexType a,b,c;
			a = x+y*w;
			b = x+(y+1)*w;
			c = (x+1)+y*w;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);
			
            simpleMesh.addIndex(a);
            simpleMesh.addIndex(b);
            simpleMesh.addIndex(c);
            
			a = (x+1)+y*w;
			b = x+(y+1)*w;
			c = (x+1)+(y+1)*w;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);
            
            simpleMesh.addIndex(a);
            simpleMesh.addIndex(b);
            simpleMesh.addIndex(c);
		}
	}		
	
	indexMap.clear();
	for (int y = 0; y < 480; y+=simplify){
		for (int x=0; x < 640; x+=simplify){
			IndexMap m;
            m.valid = false;
			indexMap.push_back(m);
		}
	}

	simpleMesh.clearVertices();
	for (int y = 0; y < 480; y++){
		for (int x=0; x < 640; x++){
			simpleMesh.addVertex(ofVec3f(x,y,0));
		}
	}
    
    if(addColors){
        simpleMesh.clearColors();
        for (int y = 0; y < 480; y++){
            for (int x=0; x < 640; x++){
                simpleMesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
            }
        }        
    }
    
    if(calculateTextureCoordinates){
        simpleMesh.clearTexCoords();
        for (int y = 0; y < 480; y++){
            for (int x=0; x < 640; x++){
                simpleMesh.addTexCoord(ofVec2f(0,0));
            }
        }        
    }
    
	//cout << "AFTER SETUP base indeces? " << baseIndeces.size() << " index map? " << indexMap.size() << endl;
}

//-----------------------------------------------
int ofxRGBDRenderer::getSimplification(){
	return simplify;
}

//-----------------------------------------------
void ofxRGBDRenderer::setRGBTexture(ofBaseHasPixels& pix) {
	currentRGBImage = &pix;
    undistortedRGBImage.setFromPixels(pix.getPixelsRef());
	hasRGBImage = true;
}

ofBaseHasPixels& ofxRGBDRenderer::getRGBTexture() {
    return *currentRGBImage;
}

void ofxRGBDRenderer::setDepthImage(ofShortPixels& pix){
    currentDepthImage = &pix;
	if(!undistortedDepthImage.isAllocated()){
		undistortedDepthImage.allocate(640,480,OF_IMAGE_GRAYSCALE);
	}
	hasDepthImage = true;
}

Calibration& ofxRGBDRenderer::getDepthCalibration(){
	return depthCalibration;
}

Calibration& ofxRGBDRenderer::getRGBCalibration(){
	return rgbCalibration;
}

void ofxRGBDRenderer::update(){
    
	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return;
    }

    if(!calibrationSetup && hasRGBImage){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration for RGB Image");
        return;
    }
    
	//undistort the current images
    undistortImages();

    //feed the zed values into the mesh
    unsigned short* ptr = undistortedDepthImage.getPixels();    
    for(int i = 0; i < simpleMesh.getNumVertices(); i++){
	    simpleMesh.getVertices()[i].z = (*ptr++);
    }

    
    //TODO: convert this back into a way to generate the mesh on the CPUb
//	int start = ofGetElapsedTimeMillis();
//	Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
//	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
    
//    forceUndistortOff = true;
	
    //start
//	int imageIndex = 0;
//	int vertexIndex = 0;
//	float xReal,yReal;
//    int indexPointer = 0;
//    int vertexPointer = 0;

    
//	for(int y = 0; y < h; y += simplify) {
//		for(int x = 0; x < w; x += simplify) {
//            
//            //simpleMesh.setVertex(y*w+x, ofVec3f(x,y,undistortedDepthImage.getPixels()[y*w+x]));
//        }
//    }
    
	/*
	for(int y = 0; y < h; y += simplify) {
		for(int x = 0; x < w; x += simplify) {
            vertexPointer = y*w+x;
			unsigned short z = undistortedDepthImage.getPixels()[y*w+x];
			IndexMap& indx = indexMap[indexPointer];
			if(z != 0 && z < farClip){
				xReal = (((float)principalPoint.x - x) / imageSize.width) * z * fx;
				yReal = (((float)y - principalPoint.y) / imageSize.height) * z * fy;
                //yReal = (((float)principalPoint.y - y) / imageSize.height) * z * fy;
                indx.vertexIndex = vertexPointer;
				indx.valid = true;
                simpleMesh.setVertex(vertexPointer, ofVec3f(xReal, yReal, z));
                hasVerts = true;
			}
			else {
				indx.valid = false;
			}
            indexPointer++;
		}
	}
    //end
    */

    /*
    if(debug && !hasVerts) cout << "warning no verts with far clip " << farClip << endl; 
	if(debug) cout << "unprojection " << simpleMesh.getVertices().size() << " took " << ofGetElapsedTimeMillis() - start << endl;
	
	simpleMesh.clearIndices();
	set<ofIndexType> calculatedNormals;
	start = ofGetElapsedTimeMillis();
    if(calculateNormals){
		simpleMesh.getNormals().resize(simpleMesh.getVertices().size());
	}
    
	for(int i = 0; i < baseIndeces.size(); i+=3){
		if(indexMap[baseIndeces[i]].valid &&
		   indexMap[baseIndeces[i+1]].valid &&
		   indexMap[baseIndeces[i+2]].valid){
			
			ofVec3f a,b,c;
			a = simpleMesh.getVertices()[indexMap[baseIndeces[i]].vertexIndex]; 
			b = simpleMesh.getVertices()[indexMap[baseIndeces[i+1]].vertexIndex]; 
			c = simpleMesh.getVertices()[indexMap[baseIndeces[i+2]].vertexIndex]; 
			if(fabs(a.z - b.z) < edgeCull && fabs(a.z - c.z) < edgeCull){
				simpleMesh.addTriangle(indexMap[baseIndeces[i]].vertexIndex, 
									   indexMap[baseIndeces[i+1]].vertexIndex,
									   indexMap[baseIndeces[i+2]].vertexIndex);
				
				if(calculateNormals && calculatedNormals.find(indexMap[baseIndeces[i]].vertexIndex) == calculatedNormals.end()){
					//calculate normal
					simpleMesh.setNormal(indexMap[baseIndeces[i]].vertexIndex, (b-a).getCrossed(b-c).getNormalized());
					calculatedNormals.insert(indexMap[baseIndeces[i]].vertexIndex);
				}
			}
		}
	}
	*/
    
	//if(debug) cout << "indexing  " << simpleMesh.getIndices().size() << " took " << ofGetElapsedTimeMillis() - start << endl;
    
    //normally this is done in the shader, but sometimes we want to have them on the CPU for doing special processing
//	if(calculateTextureCoordinates){
//        generateTextureCoordinates();
//		if(debug) cout << "gen tex coords took " << (ofGetElapsedTimeMillis() - start) << endl;
//	}
}

void ofxRGBDRenderer::undistortImages(){
    if(!forceUndistortOff){
        depthCalibration.undistort( toCv(*currentDepthImage), toCv(undistortedDepthImage), CV_INTER_NN);
        rgbCalibration.undistort( toCv(*currentRGBImage), toCv(undistortedRGBImage) );
        undistortedRGBImage.update();
    }
    else {
        undistortedDepthImage = *currentDepthImage;
        undistortedRGBImage.setFromPixels(currentRGBImage->getPixelsRef());
    }
}

ofVec3f ofxRGBDRenderer::getWoldPoint(int x, int y){
    Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
    unsigned short z = undistortedDepthImage.getPixels()[y*imageSize.width+x];

    if(z != 0 && z < farClip){
        return ofVec3f((((float)principalPoint.x - x) / imageSize.width) * z * fx,
                       (((float)principalPoint.y - y) / imageSize.height) * z * fy,z);
    }
    return ofVec3f(0,0,0);
}

void ofxRGBDRenderer::generateTextureCoordinates(){
    if(!calibrationSetup){
        ofLogError("ofxRGBDRenderer::generateTextureCoordinates -- no calibration set up");
        return;
    }
    
    if(!simpleMesh.hasTexCoords()){
        for (int y = 0; y < 640*480; y++){
            simpleMesh.addTexCoord(ofVec2f(0,0));
        } 
    }
    
    Mat pcMat = Mat(toCv(simpleMesh));		
    imagePoints.clear();
       
   projectPoints(pcMat,
                 rotationDepthToRGB, translationDepthToRGB,
                 rgbCalibration.getDistortedIntrinsics().getCameraMatrix(),
                 rgbCalibration.getDistCoeffs(),
                 imagePoints);
    
    for(int i = 0; i < imagePoints.size(); i++) {
	    //TODO account for fudge factor that the shader listens to
        simpleMesh.setTexCoord(i, ofVec2f(imagePoints[i].x, imagePoints[i].y));			
	}
}

bool ofxRGBDRenderer::isVertexValid(int index){
    return indexMap[index].valid;
}

int ofxRGBDRenderer::vertexIndex(int sequenceIndex){
	return indexMap[sequenceIndex].vertexIndex;    
}

int ofxRGBDRenderer::getTotalPoints(){
	return indexMap.size();
}
ofMesh& ofxRGBDRenderer::getMesh(){
	return simpleMesh;
}

void ofxRGBDRenderer::reloadShader(){
//    shader.load("shaders/unproject");
    meshShader.setGeometryInputType(GL_TRIANGLES);
    meshShader.setGeometryOutputType(GL_TRIANGLE_STRIP);
    meshShader.setGeometryOutputCount(3);
    meshShader.load("shaders/unproject.vert",
                	"shaders/unproject.frag",
                	"shaders/unproject.geom");
    
    pointShader.load("shaders/unproject");
}

void ofxRGBDRenderer::drawProjectionDebug(){
    ofPushStyle();
    ofPushMatrix();
    ofSetColor(255);
    rgbMatrix = (depthToRGBView * rgbProjection);
    glMultMatrixf(rgbMatrix.getInverse().getPtr());
	ofNoFill();
    ofBox(200.0f);
    ofPopMatrix();
    ofPopStyle();
}

bool ofxRGBDRenderer::bindRenderer(ofShader& shader){

	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return false;
    }
    
    if(!calibrationSetup){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration");
        return false;
    }
	
    ofPushMatrix();
    
    ofScale(1, -1, 1);
    if(mirror){
	    ofScale(-1, 1, 1);    
    }
    
    ofRotate(meshRotate.x,1,0,0);
    ofRotate(meshRotate.y,0,1,0);
    ofRotate(meshRotate.z,0,0,1);
	
//    if(ofGetKeyPressed('v'))
//        cout << "view " <<depthToRGBView << endl;
//    else if (ofGetKeyPressed('p'))
//        cout << "projection " << rgbProjection << endl;
	//cout << rgbMatrix << endl;

	if(hasRGBImage){
        undistortedRGBImage.getTextureReference().bind();
        shader.begin();	
        setupProjectionUniforms(shader);
        currentlyBoundShader = &shader;
	}
    
    rendererBound = true;
    return true;
}


void ofxRGBDRenderer::unbindRenderer(){
    
    if(!rendererBound){
        ofLogError("ofxRGBDRenderer::unbindRenderer -- called without renderer bound");
     	return;   
    }
    
    if(hasRGBImage){
        undistortedRGBImage.getTextureReference().unbind();
        if(currentlyBoundShader != NULL){
            restortProjection();
            currentlyBoundShader->end();
			currentlyBoundShader = NULL;
        }
        else {
            ofLogError("ofxRGBDRenderer::unbindRenderer -- with no reference to a shader");
        }
	}
    
	ofPopMatrix();
    rendererBound = false;
}

void ofxRGBDRenderer::setupProjectionUniforms(ofShader& theShader){

    rgbMatrix = (depthToRGBView * rgbProjection);
    //rgbMatrix.scale(1, -1, 1);
    
    Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();

    ofVec2f dims = ofVec2f(undistortedRGBImage.getTextureReference().getWidth(), 
                           undistortedRGBImage.getTextureReference().getHeight());
    
//    cout << "Shift is " << xshift << " " << yshift << endl;
    
    theShader.setUniform2f("fudge", xshift, yshift);
    theShader.setUniform2f("dim", dims.x, dims.y);
    theShader.setUniform2f("principalPoint", principalPoint.x, principalPoint.y);
    theShader.setUniform2f("fov", fx, fy);
    theShader.setUniform2f("imageSize", imageSize.width,imageSize.height);
    theShader.setUniform1f("farClip", farClip);
    
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadMatrixf(rgbMatrix.getPtr());
    glMatrixMode(GL_MODELVIEW);      
}

void ofxRGBDRenderer::restortProjection(){
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);   
}

void ofxRGBDRenderer::drawMesh(){
    drawMesh(meshShader);
}

void ofxRGBDRenderer::drawPointCloud(){
    drawPointCloud(pointShader);
}

void ofxRGBDRenderer::drawWireFrame(){
    drawWireFrame(meshShader);
}

void ofxRGBDRenderer::drawMesh(ofShader& shader){
    
    if(bindRenderer(shader)){
		simpleMesh.drawFaces();
        unbindRenderer();
    }
}

void ofxRGBDRenderer::drawPointCloud(ofShader& shader){

    if(bindRenderer(shader)){
	    simpleMesh.drawVertices();
        unbindRenderer();
    }
}

void ofxRGBDRenderer::drawWireFrame(ofShader& shader){

	if(bindRenderer(shader)){
		simpleMesh.drawWireframe();
        unbindRenderer();
    }
}

//ofTexture& ofxRGBDRenderer::getTextureReference(){
//	return currentRGBImage->getTextureReference();
//}
