//
//  GlobalSettings.cpp
//  InnovationWall
//
//  Created by Kris Meeusen on 01/02/16.
//
//

#include <stdio.h>
#include "GlobalSettings.h"

#include "cinder/Utilities.h"

using namespace std;
using namespace ci;



GlobalSettings::GlobalSettings(){

    hasGifOutput = Setting<bool>("hasGifOutput", true);
    hasClearButton = Setting<bool>("hasClearButton",true);
    hasLayerButton = Setting<bool>("hasLayerButton",true);
    doFadeOut = Setting<bool>("doFadeout",false);
	zoomLevel = Setting<float>("zoomlevel", 0.9).setSteps(0.05);
	windowScale = Setting<float>("windowScale", 0.5).setSteps(0.05);
	debugMode = Setting<bool>("debugMode", false);
	maxGroups = Setting<int>("maxGroups", 2);
	sceensLeftOffset = Setting<int>("sceensLeftOffset", 0);
	activeGroup = Setting<int>("activeGroup", 0);
	isFullScreen = Setting<bool>("isFullScreen", false);

    compositionWidth = Setting<int>("compositionWidth",1920).setSteps(10);
    compositionHeight = Setting<int>("compositionHeight",1080).setSteps(10);

	zoomLevel = Setting<float>("zoomlevel", 0.9).setSteps(0.05);
	nrOfScreens = Setting<int>("nrOfScreens", 3);
	fadeoutFactorDrawing = Setting<float>("fadeoutFactorDrawing", 1).setMin(0).setMax(1000).setSteps(1);
	fadeoutFactorReplay = Setting<float>("fadeoutFactorReplay", 4).setMin(0).setMax(1000).setSteps(1);
	replayStartTime = Setting<int>("replayStartTime", 10).setMin(0).setMax(60 * 10).setSteps(1);



    addSetting(&hasGifOutput);
    addSetting(&hasClearButton);
    addSetting(&hasLayerButton);
    addSetting(&doFadeOut);
    addSetting(&zoomLevel);
    addSetting(&compositionWidth);
    addSetting(&compositionHeight);
    addSetting(&debugMode);
    addSetting(&maxGroups);
	addSetting(&nrOfScreens);
	addSetting(&sceensLeftOffset);
	addSetting(&activeGroup);
	addSetting(&windowScale);
	addSetting(&isFullScreen);
	addSetting(&fadeoutFactorDrawing);
	addSetting(&fadeoutFactorReplay);
	addSetting(&replayStartTime);



    compositionSize = ci::ivec2(compositionWidth.value(),compositionHeight.value());
    
    
    fboBackground = ci::ColorA(0.0,0.0,0.0,0.0);
    brushColor = ci::ColorA(0.0, 0.0,.0,1.0);

}








