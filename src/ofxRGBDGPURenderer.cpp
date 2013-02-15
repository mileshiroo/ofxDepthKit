/**
 * ofxRGBDepth addon
 *
 * Core addon for programming against the RGBDToolkit API
 * http://www.rgbdtoolkit.com
 *  http://github.com/obviousjim
 *
 * (c) James George 2011-2013 http://www.jamesgeorge.org
 *
 * Developed with support from:
 *      Frank-Ratchy STUDIO for Creative Inquiry http://studioforcreativeinquiry.org/
 *      YCAM InterLab http://interlab.ycam.jp/en
 *      Eyebeam http://eyebeam.org
 */

#include "ofxRGBDGPURenderer.h"
using namespace ofxCv;
using namespace cv;

ofxRGBDGPURenderer::ofxRGBDGPURenderer()
    : ofxRGBDRenderer()
{
    rendererBound = false;
    setShaderPath("shaders/unproject");
}

ofxRGBDGPURenderer::~ofxRGBDGPURenderer(){

}
/*
bool ofxRGBDGPURenderer::setup(string calibrationDirectory){
	
	if(!ofDirectory(calibrationDirectory).exists()){
		ofLogError("ofxRGBDGPURenderer --- Calibration directory doesn't exist: " + calibrationDirectory);
		return false;
	}
	return setup(calibrationDirectory+"/rgbCalib.yml", calibrationDirectory+"/depthCalib.yml",
		  		calibrationDirectory+"/rotationDepthToRGB.yml", calibrationDirectory+"/translationDepthToRGB.yml");
	
}

bool ofxRGBDGPURenderer::setup(string rgbIntrinsicsPath, string depthIntrinsicsPath, string rotationPath, string translationPath){
//	rgbCalibration.setFillFrame(false);
//	depthCalibration.setFillFrame(false);
	depthCalibration.load(depthIntrinsicsPath);
	rgbCalibration.load(rgbIntrinsicsPath);
	
	loadMat(rotationDepthToRGB, rotationPath);
	loadMat(translationDepthToRGB, translationPath);
    
    depthToRGBView = ofxCv::makeMatrix(rotationDepthToRGB, translationDepthToRGB);
	
    ofPushView();
	rgbCalibration.getDistortedIntrinsics().loadProjectionMatrix();
    glGetFloatv(GL_PROJECTION_MATRIX, rgbProjection.getPtr());
	ofPopView();
	
	ofPushView();
	depthCalibration.getDistortedIntrinsics().loadProjectionMatrix();
	glGetFloatv(GL_PROJECTION_MATRIX, depthProjection.getPtr());
	ofPopView();
	
	rgbMatrix = (depthToRGBView * rgbProjection);
	
//	Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
//	fx = tanf(ofDegToRad(fov.x) / 2) * 2;
//	fy = tanf(ofDegToRad(fov.y) / 2) * 2;
//	fx = depthCalibration.getUndistortedIntrinsics().getCameraMatrix().at<double>(0,0);
//	fy = depthCalibration.getUndistortedIntrinsics().getCameraMatrix().at<double>(1,1);
//	principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
//	imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
	
    fx = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(0,0);
	fy = depthCalibration.getDistortedIntrinsics().getCameraMatrix().at<double>(1,1);
	principalPoint = depthCalibration.getDistortedIntrinsics().getPrincipalPoint();
	imageSize = depthCalibration.getDistortedIntrinsics().getImageSize();
    

//  cout << "successfully loaded calibration: fx + fy is " << fx << " " << fy  << endl;
//	cout << "RGB Camera Matrix is " << rgbCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
//	cout << "RGB Distortion coefficients " << rgbCalibration.getDistCoeffs() << endl;
//	cout << "Depth Camera Matrix is " << depthCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
//	cout << "Depth Distortion coefficients " << depthCalibration.getDistCoeffs() << endl;
//	cout << "RGB->Depth rotation " << rotationDepthToRGB << endl;
//	cout << "RGB->Depth translation " << translationDepthToRGB << endl;
//	cout << "RGB Aspect Ratio " << rgbCalibration.getDistortedIntrinsics().getAspectRatio() << endl;
//	cout << "RGB Focal Length " << rgbCalibration.getDistortedIntrinsics().getFocalLength() << endl;

    calibrationSetup = true;
	return true;
}
*/

//void ofxRGBDGPURenderer::setSimplification(float simplification){
//    setSimplification(ofVec2f(simplification,simplification));
//}

void ofxRGBDGPURenderer::setSimplification(ofVec2f simplification){
    
    if(!calibrationSetup){
    	return;
    }

    if(simplify == simplification){
        return;
    }
    
    if(simplification.x <= 0  || simplification.y <= 0){
        return;
    }
    
	simplify = simplification;
//    simplify.x = ofClamp(simplify.x, 0, 8);
//    simplify.y = ofClamp(simplify.y, 0, 8);
	
    mesh.clearIndices();
    int x = 0;
    int y = 0;
    
//    int w = imageSize.width;
    int gw = ceil(imageSize.width / simplify.x);
    int w = gw*simplify.x;
    int h = imageSize.height;
    
	for (float ystep = 0; ystep < h-simplify.y; ystep += simplify.y){
		for (float xstep = 0; xstep < w-simplify.x; xstep += simplify.x){
			ofIndexType a,b,c;
            
			a = x+y*gw;
			b = (x+1)+y*gw;
			c = x+(y+1)*gw;
            mesh.addIndex(a);
            mesh.addIndex(b);
            mesh.addIndex(c);

			a = (x+1)+(y+1)*gw;
			b = x+(y+1)*gw;
			c = (x+1)+(y)*gw;
            mesh.addIndex(a);
            mesh.addIndex(b);
            mesh.addIndex(c);
            
            x++;
		}
        
        y++;
        x = 0;
	}
    
	mesh.clearVertices();
	for (float y = 0; y < imageSize.height; y += simplify.y){
		for (float x = 0; x < imageSize.width; x += simplify.x){
			mesh.addVertex(ofVec3f(x,y,0));
		}
	}

    if(addColors){
        mesh.clearColors();
        for (float y = 0; y < imageSize.height; y += simplify.y){
            for (float x = 0; x < imageSize.width; x += simplify.x){
                mesh.addColor(ofFloatColor(1.0,1.0,1.0,1.0));
            }
        }        
    }
    
    meshGenerated = true;
}

////-----------------------------------------------
//ofVec2f ofxRGBDGPURenderer::getSimplification(){
//	return simplify;
//}

////-----------------------------------------------
//void ofxRGBDGPURenderer::setRGBTexture(ofBaseHasTexture& tex){
//
//	currentRGBImage = &tex;
//	hasRGBImage = true;
//}

//ofBaseHasTexture& ofxRGBDGPURenderer::getRGBTexture() {
//    return *currentRGBImage;
//}

void ofxRGBDGPURenderer::setDepthImage(ofShortPixels& pix){
    ofxRGBDRenderer::setDepthImage(pix);
    
    if(!depthTexture.isAllocated() ||
       depthTexture.getWidth() != pix.getWidth() ||
       depthTexture.getHeight() != pix.getHeight())
    {
        ofTextureData texData;
        texData.width = pix.getWidth();
        texData.height = pix.getHeight();
        texData.glType = GL_LUMINANCE;
        texData.glTypeInternal = GL_LUMINANCE16;
        texData.pixelType = GL_UNSIGNED_SHORT;
        
        depthTexture.allocate(texData);
        depthTexture.bind();
        GLint internalFormat;
        glGetTexLevelParameteriv(texData.textureTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
        depthTexture.unbind();
        
//        cout << " is depth texture allocated? " << (depthTexture.bAllocated() ? "yes" : "no") << " internal format? " << internalFormat << " vs " << GL_LUMINANCE16 << endl;
    }
    
//    currentDepthImage = &pix;
//	hasDepthImage = true;
}

//Calibration& ofxRGBDGPURenderer::getDepthCalibration(){
//	return depthCalibration;
//}
//
//Calibration& ofxRGBDGPURenderer::getRGBCalibration(){
//	return rgbCalibration;
//}
//
//ofMatrix4x4& ofxRGBDGPURenderer::getRGBMatrix(){
//	return rgbMatrix;
//}
//
//ofMatrix4x4& ofxRGBDGPURenderer::getDepthToRGBTransform(){
//	return depthToRGBView;
//}
//

ofTexture& ofxRGBDGPURenderer::getDepthTexture(){
    return depthTexture;
}

void ofxRGBDGPURenderer::update(){
    
	if(!hasDepthImage){
     	ofLogError("ofxRGBDGPURenderer::update() -- no depth image");
        return;
    }

    if(!calibrationSetup && hasRGBImage){
     	ofLogError("ofxRGBDGPURenderer::update() -- no calibration for RGB Image");
        return;
    }
    
    if(simplify ==  ofVec2f(0,0)){
        setSimplification(ofVec2f(1.0));
    }
    
	depthTexture.loadData(*currentDepthImage);
}

//ofVboMesh& ofxRGBDGPURenderer::getMesh(){
//	return mesh;
//}

//void ofxRGBDGPURenderer::setXYShift(ofVec2f shift){
//    xshift = shift.x;
//    yshift = shift.y;
//}
//
//void ofxRGBDGPURenderer::setXYScale(ofVec2f scale){
//    xscale = scale.x;
//    yscale = scale.y;
//}

void ofxRGBDGPURenderer::setShaderPath(string path){
    shaderPath = path;
    reloadShader();
}

void ofxRGBDGPURenderer::reloadShader(){
    meshShader.load(shaderPath);
}

//void ofxRGBDGPURenderer::drawProjectionDebug(bool showDepth, bool showRGB, float rgbTexturePosition){
//    ofPushStyle();
//	glEnable(GL_DEPTH_TEST);
//	if(showRGB){
//		ofPushMatrix();
//		ofSetColor(255);
//		rgbMatrix = (depthToRGBView * rgbProjection);
//		ofScale(1,-1,1);
//		glMultMatrixf(rgbMatrix.getInverse().getPtr());
//		
//		ofNoFill();
//		ofSetColor(255,200,10);
//		ofBox(1.99f);
//		
//		//draw texture
//		if(rgbTexturePosition > 0){
//			ofSetColor(255);
//			ofTranslate(0, 0, 1.0 - powf(1-rgbTexturePosition, 2.0));
//			undistortedRGBImage.draw(1, 1, -2, -2);
//		}
//		ofPopMatrix();
//	}
//	
//	if(showDepth){
//		ofPushMatrix();
//		ofScale(-1,1,-1);
//		ofNoFill();
//		ofSetColor(10,200,255);
//		glMultMatrixf(depthProjection.getInverse().getPtr());
//		ofBox(1.99f);
//		ofPopMatrix();
//	}
//	
//	glDisable(GL_DEPTH_TEST);
//    ofPopStyle();
//}

bool ofxRGBDGPURenderer::bindRenderer(){

	if(!hasDepthImage){
     	ofLogError("ofxRGBDGPURenderer::update() -- no depth image");
        return false;
    }
    
    if(!calibrationSetup){
     	ofLogError("ofxRGBDGPURenderer::update() -- no calibration");
        return false;
    }
	
    ofPushMatrix();
    
    ofScale(1, -1, 1);
    if(!mirror){
	    ofScale(-1, 1, 1);    
    }
    
    ofRotate(worldRotation.x,1,0,0);
    ofRotate(worldRotation.y,0,1,0);
    ofRotate(worldRotation.z,0,0,1);

	if(hasRGBImage){
        meshShader.begin();
        glActiveTexture(GL_TEXTURE1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glActiveTexture(GL_TEXTURE0);

        setupProjectionUniforms();
    }
    
    rendererBound = true;
    return true;
}

void ofxRGBDGPURenderer::unbindRenderer(){
    
    if(!rendererBound){
        ofLogError("ofxRGBDGPURenderer::unbindRenderer -- called without renderer bound");
     	return;   
    }
    
    if(rendererBound && hasRGBImage){
        meshShader.end();
//        currentlyBoundShader->end();
//        currentlyBoundShader = NULL;
//        }
	}

	ofPopMatrix();
    rendererBound = false;
}

void ofxRGBDGPURenderer::setupProjectionUniforms(){
    ofVec2f dims = ofVec2f(currentRGBImage->getTextureReference().getWidth(),
                           currentRGBImage->getTextureReference().getHeight());

	meshShader.setUniformTexture("colorTex", currentRGBImage->getTextureReference(), 0);
	meshShader.setUniformTexture("depthTex", depthTexture, 1);
    meshShader.setUniform1i("useTexture", useTexture ? 1 : 0);
    meshShader.setUniform2f("shift", shift.x, shift.y);
    meshShader.setUniform2f("scale", scale.x, scale.y);
    meshShader.setUniform2f("dim", dims.x, dims.y);
    meshShader.setUniform2f("principalPoint", principalPoint.x, principalPoint.y);
    meshShader.setUniform2f("fov", fx, fy);
    meshShader.setUniform1f("farClip", farClip);
	meshShader.setUniform1f("edgeClip", edgeClip);
	if(flipTexture){
		ofMatrix4x4 flipMatrix;
		flipMatrix.makeScaleMatrix(-1,1,1);
        rgbMatrix = (depthToRGBView * (flipMatrix * rgbProjection));
        meshShader.setUniformMatrix4f("tTex", rgbMatrix );
	}
    else{
        rgbMatrix = (depthToRGBView * rgbProjection);
        meshShader.setUniformMatrix4f("tTex", rgbMatrix);
    }
    
    meshShader.setUniform1f("xsimplify", simplify.x);
    meshShader.setUniform1f("ysimplify", simplify.y);
}

//void ofxRGBDGPURenderer::drawMesh(){
//    drawMesh(meshShader);
//}
//
//void ofxRGBDGPURenderer::drawPointCloud(){
//    drawPointCloud(pointShader);
//}
//
//void ofxRGBDGPURenderer::drawWireFrame(){
//    drawWireFrame(meshShader);
//}

void ofxRGBDGPURenderer::draw(ofPolyRenderMode drawMode){
    if(bindRenderer()){
        
	    switch(drawMode){
            case OF_MESH_POINTS:
                mesh.drawVertices(); break;
            case OF_MESH_WIREFRAME:
                mesh.drawWireframe(); break;
            case OF_MESH_FILL:
                mesh.drawFaces(); break;
        }
        unbindRenderer();
    }
}

