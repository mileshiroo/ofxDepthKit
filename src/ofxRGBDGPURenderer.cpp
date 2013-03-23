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

