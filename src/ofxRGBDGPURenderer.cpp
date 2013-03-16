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
	depthOnly = false;
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
	
    mesh.clearIndices();
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
	for (float y = 0; y < depthImageSize.height; y += simplify.y){
		for (float x = 0; x < depthImageSize.width; x += simplify.x){
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
    
    meshGenerated = true;
}


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
    
}


ofTexture& ofxRGBDGPURenderer::getDepthTexture(){
    return depthTexture;
}

void ofxRGBDGPURenderer::update(){
    
	if(!hasDepthImage){
     	ofLogError("ofxRGBDGPURenderer::update() -- no depth image");
        return;
    }

    if(!calibrationSetup && hasRGBImage && !depthOnly){
     	ofLogError("ofxRGBDGPURenderer::update() -- no calibration for RGB Image");
        return;
    }
    
    if(simplify == ofVec2f(0,0)){
        setSimplification(ofVec2f(1.0, 1.0));
    }
    
	depthTexture.loadData(*currentDepthImage);
}

void ofxRGBDGPURenderer::setShaderPath(string path){
    shaderPath = path;
    reloadShader();
}

void ofxRGBDGPURenderer::reloadShader(){
    shader.load(shaderPath);
}

bool ofxRGBDGPURenderer::bindRenderer(){

	if(!hasDepthImage){
     	ofLogError("ofxRGBDGPURenderer::update() -- no depth image");
        return false;
    }
    
    if(!calibrationSetup && !depthOnly){
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

//	if(hasRGBImage){
        shader.begin();
        glActiveTexture(GL_TEXTURE1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glActiveTexture(GL_TEXTURE0);

        setupProjectionUniforms();
//    }
    
    rendererBound = true;
    return true;
}

void ofxRGBDGPURenderer::unbindRenderer(){
    
    if(!rendererBound){
        ofLogError("ofxRGBDGPURenderer::unbindRenderer -- called without renderer bound");
     	return;   
    }
    
	shader.end();
	rendererBound = false;

	ofPopMatrix();
}

void ofxRGBDGPURenderer::setupProjectionUniforms(){

	if(!depthOnly && useTexture){
		ofVec2f dims = ofVec2f(currentRGBImage->getTextureReference().getWidth(),
							   currentRGBImage->getTextureReference().getHeight());
		shader.setUniformTexture("colorTex", currentRGBImage->getTextureReference(), 0);
		shader.setUniform1i("useTexture", 1);
		shader.setUniform2f("dim", dims.x, dims.y);
		shader.setUniform2f("shift", shift.x, shift.y);
		shader.setUniform2f("scale", scale.x, scale.y);
		shader.setUniform3f("dK", distortionK.x, distortionK.y, distortionK.z);
		shader.setUniform2f("dP", distortionP.x, distortionP.y);
		glUniformMatrix3fv(shader.getUniformLocation("colorRotate"), 1, GL_FALSE, depthToRGBRotation);
		shader.setUniform3f("colorTranslate", depthToRGBTranslation.x,depthToRGBTranslation.y,depthToRGBTranslation.z);
		shader.setUniform2f("colorFOV", colorFOV.x, colorFOV.y );
		shader.setUniform2f("colorPP", colorPrincipalPoint.x, colorPrincipalPoint.y);
	}
	else{
		shader.setUniform1i("useTexture", 0);
	}
	
	shader.setUniformTexture("depthTex", depthTexture, 1);
	
//	if(flipTexture){
//		ofMatrix4x4 flipMatrix;
//		flipMatrix.makeScaleMatrix(-1,1,1);
//		rgbMatrix = (depthToRGBView * (flipMatrix * rgbProjection));
//		shader.setUniformMatrix4f("tTex", rgbMatrix );
//	}
//	else{
//		rgbMatrix = (depthToRGBView * rgbProjection);
//		shader.setUniformMatrix4f("tTex", rgbMatrix);
		
		//float* translatefv = translationDepthToRGB.ptr<float>();
//		ofVec3f Rtranslate(translationDepthToRGB.at<double>(0,0),
//						   translationDepthToRGB.at<double>(1,0),
//						   translationDepthToRGB.at<double>(2,0));
		
	
//		cout << "translate " << Rtranslate << endl;
	
//		Mat rx3;
//		cv::Rodrigues(rotationDepthToRGB, rx3);
//		float rotation3fv[9] = {
//			float(rx3.at<double>(0,0)),float(rx3.at<double>(1,0)),float(rx3.at<double>(2,0)),
//			float(rx3.at<double>(0,1)),float(rx3.at<double>(1,1)),float(rx3.at<double>(2,1)),
//			float(rx3.at<double>(0,2)),float(rx3.at<double>(1,2)),float(rx3.at<double>(2,2))
//		};
//
//		Mat dis = rgbCalibration.getDistCoeffs();
//		ofVec3f disortionK = ofVec3f(dis.at<double>(0,0),
//									 dis.at<double>(0,1),
//									 dis.size().height == 5 ? dis.at<double>(0,4) : 0);
//		ofVec2f disortionP = ofVec2f(dis.at<double>(0,2),dis.at<double>(0,3));
		
//		cout << (dis.size().height) << " disort K " << disortionK << " Disort P " << disortionP << endl;
//		cout << " " << rx3.size().width << " " << rx3.size().height << endl;
//		cout << rx3.at<double>(0,0) << " " << rx3.at<double>(0,1) << " " << rx3.at<double>(0,2) << endl;
//		cout << rx3.at<double>(1,0) << " " << rx3.at<double>(1,1) << " " << rx3.at<double>(1,2) << endl;
//		cout << rx3.at<double>(2,0) << " " << rx3.at<double>(2,1) << " " << rx3.at<double>(2,2) << endl;
	
		//double* rotationdv = rx3.ptr<double>();
//		cout << "rotate " << rotationdv[0] << " " << rotationdv[1] << " " << rotationdv[2] << endl;
//		cout << "translate" << translatefv[0] << " " << translatefv[1] << " " << translatefv[2] << endl;
		
//	}

    shader.setUniform2f("principalPoint", depthPrincipalPoint.x, depthPrincipalPoint.y);
    shader.setUniform2f("fov", depthFOV.x, depthFOV.y);
    shader.setUniform1f("farClip", farClip);
	shader.setUniform1f("edgeClip", edgeClip);
	
    //TODO: vectorize in shader
    shader.setUniform1f("xsimplify", simplify.x);
    shader.setUniform1f("ysimplify", simplify.y);
}

ofShader& ofxRGBDGPURenderer::getShader(){
    return shader;
}

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

