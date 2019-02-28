//
//  BrushManager.hpp
//  LiveDraw
//
//  Created by lab101 on 06/09/16.
//
//

#pragma once

#include <stdio.h>
#include "Singleton.h"
#include "cinder/Surface.h"
#include "cinder/gl/GlslProg.h"

class BrushManager{
    
    
public:
    
    ci::gl::GlslProgRef     mShader;

    bool isEraserOn;
    float brushScale;
    void setup();
    
    void drawBrush(std::vector<ci::vec3>& points,float softness);
};

typedef Singleton<BrushManager> BrushManagerSingleton;
