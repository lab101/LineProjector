//
//  Composition.cpp
//  LiveDraw
//
//  Created by lab101 on 06/09/16.
//
//

#include "Composition.hpp"
#include "BrushManager.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "Lab101Utils.h"
#include "cinder/Log.h"
#include "GlobalSettings.h"

#include <iostream>
#include <fstream>

using namespace ci;
using namespace gl;


void Composition::setup(ivec2 size){
    
    //setup onion shader
    try{
		mOnionShader = gl::GlslProg::create(ci::app::loadAsset("shaders/onionLayer.vert"), ci::app::loadAsset("shaders/onionLayer.frag"));
    }
    catch( gl::GlslProgCompileExc ex ) {
        CI_LOG_E("error loading mask shader");
        CI_LOG_E(ex.what());
        
        exit(-1);
    }

    
    
    mSize        = size;
    setFbo(mActiveFbo,size, 1);
    
    newComposition();
}


ci::vec3 Composition::getNormalisedPositionAtIndex(int index){
    return getNormalisedPositionAtIndex(mPath,mDepths,index);
}


ci::vec3 Composition::getNormalisedPositionAtIndex(ci::Path2d& points, ci::Path2d depths,int index){
    ci::vec2 p = points.getPoints()[index] / mSize;
    ci::vec2 d = depths.getPoints()[index];

    return ci::vec3(p.x,p.y,d.y);
    
}


void Composition::clearScene(bool clearOnionLayer){
    
    if(!clearOnionLayer && mActiveFbo){
        auto source = mActiveFbo->getColorTexture()->createSource();
        mLastDrawingTexture = ci::gl::Texture::create(source);
    }
    
    mPath.clear();
    mDepths.clear();
    strokes.clear();
    clearFbo();
    
    if(clearOnionLayer && mActiveFbo){
        auto source = mActiveFbo->getColorTexture()->createSource();
        mLastDrawingTexture = ci::gl::Texture::create(source);
    }

}

void Composition::newComposition(){

    clearScene();
    
    if(mActiveFbo){
        auto source = mActiveFbo->getColorTexture()->createSource();
        mLastDrawingTexture = ci::gl::Texture::create(source);
    }
    
    mStepId =0;
    
    mGifInputFiles.clear();
    mCompositionId = getDateString();
    mOutputFolder = getDocumentsDirectory().string() + "lineDancer/" + mCompositionId;
}


void Composition::newLine(ci::vec3 pressurePoint){
    
    mPath.clear();
    mDepths.clear();
    
    mPath.moveTo(vec2(pressurePoint.x   ,pressurePoint.y));
    mDepths.moveTo(vec2(pressurePoint.x ,pressurePoint.z));
    
    lastDrawDistance = 0;
    minDistance = 0;
    
    // adding new vector of strokes for data capture
    pointVec newStrokes;
    strokes.push_back(newStrokes);
}


void Composition::endLine(){

 
    mStepId++;
}


void Composition::lineTo(ci::vec3 pressurePoint){
    mPath.lineTo(vec2(pressurePoint.x,pressurePoint.y));
    mDepths.lineTo(vec2(pressurePoint.x,pressurePoint.z));
    
    calculatePath(mPath,mDepths,true);
}

void Composition::drawCircle(ci::vec3 point1,ci::vec3 point2){
    gl::ScopedFramebuffer fbScp( mActiveFbo );
    gl::ScopedViewport fbVP (mActiveFbo->getSize());
    gl::setMatricesWindow( mActiveFbo->getSize() );
    
    gl::ScopedBlendPremult scpBlend;
    
    
    gl::color(GS()->brushColor);
    
    
    ci::gl::drawSolidCircle(vec2(point1.x,point1.y), glm::distance(point1, point2));
    
    
    gl::setMatricesWindow( ci::app::getWindowSize() );
    
   //DRAW CIRCLE STROKE
    
}

void Composition::drawRectangle(ci::vec3 point1,ci::vec3 point2){
   
    gl::ScopedFramebuffer fbScp( mActiveFbo );
    gl::ScopedViewport fbVP (mActiveFbo->getSize());
    gl::setMatricesWindow( mActiveFbo->getSize() );

    gl::ScopedBlendPremult scpBlend;


    gl::color(GS()->brushColor);
    
        Rectf rect( point1.x, point1.y, point2.x , point2.y);
      ci::gl::drawSolidRect(rect);
    //BrushManagerSingleton::Instance()->drawBrush(points, 0.98);
    
    gl::setMatricesWindow( ci::app::getWindowSize() );
    
    const int brushSize = 10;
    point1.z = brushSize;
     newLine(point1);
    mPath.lineTo(vec2(point2.x,point1.y));
    mDepths.lineTo(vec2(point2.x,brushSize));
   calculatePath(mPath,mDepths,false);
    mPath.lineTo(vec2(point2.x,point2.y));
     mDepths.lineTo(vec2(point2.x,brushSize));
    calculatePath(mPath,mDepths,false);
    mPath.lineTo(vec2(point1.x,point2.y));
    mDepths.lineTo(vec2(point1.x,brushSize));
    calculatePath(mPath,mDepths,false);
    mPath.lineTo(vec2(point1.x,point1.y));
    mDepths.lineTo(vec2(point1.x,brushSize));
    calculatePath(mPath,mDepths,false);
    endLine();
}




void Composition::setFbo(ci::gl::FboRef& fbo,ci::ivec2 size,float windowScale){
    
    gl::Fbo::Format format;
    format.setColorTextureFormat( gl::Texture2d::Format().internalFormat( GL_RGBA32F ) );
    
    gl::enableAlphaBlending();
   // format.setSamples( 4 );
    fbo = gl::Fbo::create(size.x, size.y ,format );
    
    clearFbo();

}


void Composition::drawInFbo(std::vector<ci::vec3>& points){
    
    
    if(points.size() > 0){

        gl::ScopedFramebuffer fbScp( mActiveFbo );
        gl::ScopedViewport fbVP (mActiveFbo->getSize());
        gl::setMatricesWindow( mActiveFbo->getSize() );

        gl::ScopedBlendPremult scpBlend;
   
        
        gl::color(1, 1, 1);

        BrushManagerSingleton::Instance()->drawBrush(points, 0.98);
        
        gl::setMatricesWindow( ci::app::getWindowSize() );
    }

}

void Composition::drawFadeOut(){
    
    

        gl::ScopedFramebuffer fbScp( mActiveFbo );
        gl::ScopedViewport fbVP (mActiveFbo->getSize());
        gl::setMatricesWindow( mActiveFbo->getSize() );
        
    
    // Enable pre-multiplied alpha blending.
  //  gl::ScopedBlendPremult scpBlend;

        ci::ColorA fade = GS()->fboBackground;
    fade.a = GS()->fadeoutFactor;
        gl::color(fade);
        ci::gl::drawSolidRect(Rectf(0,0, mActiveFbo->getSize().x, mActiveFbo->getSize().y));
        
        gl::setMatricesWindow( ci::app::getWindowSize() );
    
    
}




void Composition::calculatePath(ci::Path2d& path,ci::Path2d& depths, bool emmitTrueOrFalse){
    
    
    float length = path.calcLength();
    if(length <= minDistance) return;
    
    
    float newDrawPosition = lastDrawDistance + minDistance;
    
    pointVec pointsToDraw;
    pointVec pointsToDrawNormalised;

    while(newDrawPosition + minDistance < length){
        
        float newTime = path.calcTimeForDistance(newDrawPosition);
        if(newDrawPosition == 0) newTime = 0;
        
        vec3 newPoint(path.getPosition(newTime),depths.getPosition(newTime).y);
        
        pointsToDraw.push_back(newPoint);

        minDistance = fmax(.8f,(newPoint.z * .17));
        
        lastDrawDistance = newDrawPosition;
        newDrawPosition = (lastDrawDistance + minDistance);
        
        // save them normalised later to a file.
        newPoint.x /= mSize.x;
        newPoint.y /= mSize.y;
        
        pointsToDrawNormalised.push_back(newPoint);
        strokes.back().push_back(newPoint);

    }
    

    
    if(pointsToDraw.size() > 0  && emmitTrueOrFalse){
        // emmit to other listner in this case network
        onNewPoints.emit(pointsToDrawNormalised);
        // draw the new points into the fbo.
        drawInFbo(pointsToDraw);
    }
}



void Composition::draw(ci::Rectf drawingArea){

   
    gl::color(1, 1, 1, 1);
   
    ci::gl::enableAlphaBlending();

    ci::gl::pushMatrices();
    
        gl::ScopedGlslProg glslProg( mOnionShader );
        mOnionShader->uniform( "uTex0", 0 );
        mOnionShader->uniform( "uTex1", 1 );

        mActiveFbo->getColorTexture()->bind(0);
        mLastDrawingTexture->bind(1);

        //gl::drawSolidRect(ci::Rectf(0,0,mActiveFbo->getWidth(),mActiveFbo->getHeight()));



		const Rectf r(0, 0, app::getWindowSize().x, app::getWindowSize().y);
		vec2 upperLeftTexCoord = drawingArea.getUpperLeft();
		vec2 lowerRightTexCoord = drawingArea.getLowerRight();


		GLfloat data[8 + 8]; // both verts and texCoords
		GLfloat *verts = data, *texs = data + 8;

		verts[0 * 2 + 0] = r.getX2(); texs[0 * 2 + 0] = lowerRightTexCoord.x;
		verts[0 * 2 + 1] = r.getY1(); texs[0 * 2 + 1] = upperLeftTexCoord.y;
		verts[1 * 2 + 0] = r.getX1(); texs[1 * 2 + 0] = upperLeftTexCoord.x;
		verts[1 * 2 + 1] = r.getY1(); texs[1 * 2 + 1] = upperLeftTexCoord.y;
		verts[2 * 2 + 0] = r.getX2(); texs[2 * 2 + 0] = lowerRightTexCoord.x;
		verts[2 * 2 + 1] = r.getY2(); texs[2 * 2 + 1] = lowerRightTexCoord.y;
		verts[3 * 2 + 0] = r.getX1(); texs[3 * 2 + 0] = upperLeftTexCoord.x;
		verts[3 * 2 + 1] = r.getY2(); texs[3 * 2 + 1] = lowerRightTexCoord.y;

		auto ctx = context();

		ctx->pushVao();
		ctx->getDefaultVao()->replacementBindBegin();
		VboRef defaultVbo = ctx->getDefaultArrayVbo(sizeof(float) * 16);
		ScopedBuffer bufferBindScp(defaultVbo);
		defaultVbo->bufferSubData(0, sizeof(float) * 16, data);

		int posLoc = mOnionShader->getAttribSemanticLocation(geom::Attrib::POSITION);
		if (posLoc >= 0) {
			enableVertexAttribArray(posLoc);
			vertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		}
		int texLoc = mOnionShader->getAttribSemanticLocation(geom::Attrib::TEX_COORD_0);
		if (texLoc >= 0) {
			enableVertexAttribArray(texLoc);
			vertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float) * 8));
		}
		ctx->getDefaultVao()->replacementBindEnd();
		ctx->setDefaultShaderVars();
		ctx->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
		ctx->popVao();





  
    // for debugging only
//    ci::gl::draw(mActiveFbo->getColorTexture());
    ci::gl::popMatrices();


}


void Composition::writeDataFile(){
    
    // write interpolated points to a data file in the output folder
    std::string dataFilePath = mOutputFolder + "/data.txt";
    try{
        std::ofstream dataFile;
        
        dataFile.open(dataFilePath);
        
        for(pointVec s :  strokes){
            for(vec3& p :  s){
                dataFile << p.x << "," << p.y << "," << p.z << ";";
            }
            dataFile <<  std::endl;
        }
        
        dataFile.close();
    }catch(...){
        CI_LOG_E( "couldn't write to path: " + dataFilePath);
    }

}


void Composition::framesToGif(std::vector<std::string>& inputFiles, std::string gifPath){
    
    ////writing out the gif file.
    //std::vector<Magick::Image> frames;
    //
    //for(auto &i: inputFiles){
    //    Magick::Image img;
    //    img.read(i);
    //    img.animationDelay(19);  // increase the delay if you want a slower gif.
    //    img.animationIterations(-1);
    //    frames.push_back(img);
    //}
    //
    //try{
    //    CI_LOG_I("writing gif:" + gifPath);
    //    Magick::writeImages(frames.begin(), frames.end(), gifPath);
    //}
    //catch ( Magick::WarningConfigure & error)
    //{
    //    CI_LOG_E( error.what());
    //}
}



void Composition::finished(){
    
    writeDataFile();
    
    std::string path = mOutputFolder + "/__" +  mCompositionId + "composition.gif";

    std::vector<std::string> layerImages;
    
    if(ci::fs::exists(mOutputFolder)){
        for( fs::directory_iterator it( mOutputFolder); it != fs::directory_iterator(); ++it ){
            {
                if( is_directory( *it )){
                    std::string layerName  = it->path().filename().string();
                    std::vector<std::string> stepImages;

                    // found a layer folder now read the files;
                    for( fs::directory_iterator it2( it->path()); it2 != fs::directory_iterator(); ++it2 ){
                        {
                            if(it2->path().extension() == ".gif"){
                                std::cout << "--- step: " << it2->path() << std::endl;
                                stepImages.push_back(it2->path().string());
                                
                            }
                            
                        }
                    }
                    std::cout << "-------" << std::endl;
                    std::cout << "---- writing ---" << std::endl;
                    std::sort (stepImages.begin(), stepImages.end());
                    framesToGif(stepImages, mOutputFolder + "/" + layerName + "_final.gif");
                    for(auto s : stepImages){
                        std::cout << s << std::endl;
                    }
                    layerImages.push_back(stepImages.back());
                    std::cout << "---- end ---" << std::endl;

                }
            }
        }
    }
    // only write a layered final if user has been using layers
    if(layerImages.size() > 1){
        std::sort (layerImages.begin(), layerImages.end());

        framesToGif(layerImages, mOutputFolder + "/_" + mCompositionId + "_final.gif");
    }
    
}


void Composition::clearFbo(){
    // clear the screen;
    if(!mActiveFbo) return;
    
    gl::ScopedFramebuffer fbScp( mActiveFbo );
    gl::clear( GS()->fboBackground );
}


void Composition::saveLineSegmentForGif(){
    // check if output folder exists
    if(!fs::exists(mOutputFolder + "/layers/")){
        fs::create_directories(mOutputFolder);
    }
    
    // write the current drawing to a png image
    std::string path = mOutputFolder + "/layer_" + getStringWithLeadingZero(mImageLayerId, 5)+ "/" + getStringWithLeadingZero(mStepId, 5) + ".gif";
    
    writeGifStep(path);

}

void Composition::saveLayer(){
    mImageLayerId++;
    mStepId = 0;
}



void Composition::writeGifStep(std::string fileName){
    
    auto source = mActiveFbo->getColorTexture()->createSource();

    std::thread threadObj([=]{
     
            try{
                writeImage(fileName, source);
                mGifInputFiles.push_back(fileName);
            }catch(...){
                CI_LOG_E("error writing GIF image file: " + fileName);
            }
       
    });

    threadObj.detach();
}


ci::gl::TextureRef Composition::getTexture(){
    return mActiveFbo->getColorTexture();
}



