
#include "ofxRGBDCombinedVideoRenderer.h"

//--------------------------------------------------------------- 
ofxRGBDCombinedVideoRenderer::ofxRGBDCombinedVideoRenderer(){
    
    setShaderPath("shaders/rgbdcombined");
    
    shift.set(1.0f,1.0f);
	scale.set(1.0f,1.0f);
    simplify.set(0.0f,0.0f);
	textureScale.set(1.0f, 1.0f);
    
	nearClip    = 1.0f;
	edgeClip    = 50.0f;
	farClip     = 6000.0f;
    
    bMirror     = false;
    bFlipTexture = false;
    bRendererBound = false;
}

ofxRGBDCombinedVideoRenderer::~ofxRGBDCombinedVideoRenderer(){
    
}

//--------------------------------------------------------------- SET
bool ofxRGBDCombinedVideoRenderer::setup(string videoPath){
	ofxXmlSettings XML;
    
    if ( XML.loadFile(videoPath +"/_calibration.xml" ) ){

        colorPrincipalPoint.x = XML.getValue("colorIntrinsics/ppx", 971.743835449);
        colorPrincipalPoint.y = XML.getValue("colorIntrinsics/ppy", 546.945983887);
        colorFOV.x = XML.getValue("colorIntrinsics/fovx", 923.500793457);
        colorFOV.y = XML.getValue("colorIntrinsics/fovy", 921.060791016);
        colorImageSize.x = 0.0f;
        colorImageSize.y = 0.0f;
        colorImageSize.width = XML.getValue("colorIntrinsics/width", 1920.000000000);
        colorImageSize.height = XML.getValue("colorIntrinsics/width", 1080.000000000);
        
        for (int i = 0; i < 9; i++) {
            depthToRGBRotation[i] = XML.getValue("extrinsics/rotation/r"+ofToString(i), 1.0f);
        }
        
        for (int i = 0; i < 3; i++) {
            depthToRGBTranslation[i] = XML.getValue("extrinsics/translation/t"+ofToString(i), 1.0f);
        }
        
        for (int i = 0; i < 3; i++) {
            distortionK[i] = XML.getValue("colorIntrinsics/dK/k"+ofToString(i), 1.0f);
        }
        
        for (int i = 0; i < 2; i++) {
            distortionP[i] = XML.getValue("colorIntrinsics/dP/p"+ofToString(i), 1.0f);
        }
        
        depthPrincipalPoint.x = XML.getValue("depthIntrinsics/ppx", 320.0);
        depthPrincipalPoint.y = XML.getValue("depthIntrinsics/ppy", 240.0);
        depthFOV.x = XML.getValue("depthIntrinsics/fovx", 570.34);
        depthFOV.y = XML.getValue("depthIntrinsics/fovy", 570.34);
        
        depthImageSize.x = 0.0;     //  TODO: do this atomatically
        depthImageSize.y = 720.0;   //  
        depthImageSize.width = XML.getValue("depthIntrinsics/width", 640.0);
        depthImageSize.height = XML.getValue("depthIntrinsics/height", 480.0);
        
        nearClip    = XML.getValue("minDepth", 1.0f);
        farClip     = XML.getValue("maxDepth",6000.0f);
        
        //  TODO
        //simplify
        //textureScale
        
        return true;
    }
    
    return false;
}

void ofxRGBDCombinedVideoRenderer::setShaderPath(string _shaderPath){
    shaderPath = _shaderPath;
	reloadShader();
}

void ofxRGBDCombinedVideoRenderer::setTextureScaleForImage(ofBaseHasTexture& _texture){
    textureScale.x = float(_texture.getTextureReference().getWidth()) / float(colorImageSize.width);
    textureScale.y = float(_texture.getTextureReference().getHeight()-depthImageSize.height) / float(colorImageSize.height);
}

void ofxRGBDCombinedVideoRenderer::setSimplification(ofVec2f _simplification){
	
	if(simplify == _simplification){
		return;
	}
	
	if(_simplification.x <= 0  || _simplification.y <= 0){
		return;
	}
	
	simplify = _simplification;
	
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
    
	bMeshGenerated = true;
}

void ofxRGBDCombinedVideoRenderer::setTexture(ofBaseHasTexture& _tex){
    tex = &_tex;
    setTextureScaleForImage(_tex);
}

void ofxRGBDCombinedVideoRenderer::reloadShader(){
	shader.load( shaderPath );
}

//--------------------------------------------------------------- BINDERS

bool ofxRGBDCombinedVideoRenderer::bindRenderer(){
    ofPushMatrix();
	
	ofScale(1, -1, 1);
	if(!bMirror){
		ofScale(-1, 1, 1);
	}
	
	ofRotate(worldRotation.x,1,0,0);
	ofRotate(worldRotation.y,0,1,0);
	ofRotate(worldRotation.z,0,0,1);
    
    
	shader.begin();
	glActiveTexture(GL_TEXTURE1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glActiveTexture(GL_TEXTURE0);
    
	setupProjectionUniforms();
	
	bRendererBound = true;
	return true;
}

void ofxRGBDCombinedVideoRenderer::setupProjectionUniforms(){
    
    ofVec2f dims = ofVec2f(tex->getTextureReference().getWidth(),
                           tex->getTextureReference().getHeight());
    
    shader.setUniformTexture("colorTex", tex->getTextureReference(), 0);
    shader.setUniform1i("useTexture", 1);
    shader.setUniform2f("dim", dims.x, dims.y);
    shader.setUniform2f("textureScale", textureScale.x, textureScale.y);
    shader.setUniform2f("shift", shift.x, shift.y);
    shader.setUniform2f("scale", scale.x, scale.y);
    shader.setUniform3f("dK", distortionK.x, distortionK.y, distortionK.z);
    shader.setUniform2f("dP", distortionP.x, distortionP.y);
    
    glUniformMatrix3fv( glGetUniformLocation(shader.getProgram(), "colorRotate"), 1, GL_FALSE,depthToRGBRotation);
    
    shader.setUniform3f("colorTranslate", depthToRGBTranslation.x,depthToRGBTranslation.y,depthToRGBTranslation.z);
    shader.setUniform2f("colorFOV", colorFOV.x, colorFOV.y );
    shader.setUniform2f("colorPP", colorPrincipalPoint.x, colorPrincipalPoint.y);

	shader.setUniform2f("principalPoint", depthPrincipalPoint.x, depthPrincipalPoint.y);
	shader.setUniform2f("fov", depthFOV.x, depthFOV.y);
	shader.setUniform1f("farClip", farClip);
	shader.setUniform1f("edgeClip", edgeClip);
	
	//TODO: vectorize in shader
	shader.setUniform1f("xsimplify", simplify.x);
	shader.setUniform1f("ysimplify", simplify.y);
}

void ofxRGBDCombinedVideoRenderer::unbindRenderer(){
    if(!bRendererBound){
		ofLogError("ofxRGBDGPURenderer::unbindRenderer -- called without renderer bound");
	 	return;
	}
	
	shader.end();
	bRendererBound = false;
    
	ofPopMatrix();
}


//--------------------------------------------------------------- ACTIONS
void ofxRGBDCombinedVideoRenderer::update(){
    if(simplify == ofVec2f(0,0)){
		setSimplification(ofVec2f(1.0, 1.0));
	}
    
    setSimplification(ofVec2f(1.0, 1.0));
}

void ofxRGBDCombinedVideoRenderer::drawMesh(){
	draw(OF_MESH_FILL);
}

void ofxRGBDCombinedVideoRenderer::drawPointCloud(){
	draw(OF_MESH_POINTS);
}
void ofxRGBDCombinedVideoRenderer::drawWireFrame(){
	draw(OF_MESH_WIREFRAME);
}

void ofxRGBDCombinedVideoRenderer::draw(ofPolyRenderMode drawMode){
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
