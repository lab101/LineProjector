//
//  BrushManager.cpp
//  LiveDraw
//
//  Created by lab101 on 06/09/16.
//
//

#include "BrushManager.h"
#include "cinder/app/App.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"

#include "GlobalSettings.h"


#include "cinder/gl/Batch.h"
#include "cinder/gl/Scoped.h"

using namespace ci;
using namespace ci::app;

void BrushManager::setup(){
    
    
    isEraserOn = false;
    
    
    // setup shader
    try {
		mShader = gl::GlslProg::create(loadAsset("shaders/shader_es2.vert"), loadAsset("shaders/shader_es2.frag"));

    }
    catch( gl::GlslProgCompileExc ex ) {
        CI_LOG_E("error loading brush shader");
        CI_LOG_E(ex.what());
		exit(0);
    }


    CI_LOG_I("set GL_PROGRAM_POINT_SIZE");
    gl::enable(GL_PROGRAM_POINT_SIZE);
    CI_LOG_I("set GL_VERTEX_PROGRAM_POINT_SIZE");
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
}


void BrushManager::drawBrush(std::vector<vec3>& points,float softness){
	//mShader = gl::GlslProg::create(loadAsset("shaders/shader_es2.vert"), loadAsset("shaders/shader_es2.frag"));
	ci::Color(1, 1, 1);
	gl::ScopedGlslProg glslProg(mShader);
    ci::gl::VertBatchRef mBatch = gl::VertBatch::create();
    
    for(vec3& p : points){
        mBatch->vertex( p );
		mBatch->color(GS()->brushColor);
	}
    
    
	gl::enableAlphaBlendingPremult();
    mBatch->draw();
	

}

