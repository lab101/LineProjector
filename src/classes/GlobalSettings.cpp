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

   
    doFadeOut = Setting<bool>("doFadeout",false);
	zoomLevel = Setting<float>("zoomlevel", 1.0).setSteps(0.05);
	windowScale = Setting<float>("windowScale", 1.0).setSteps(0.05);
	debugMode = Setting<bool>("debugMode", false);
	maxGroups = Setting<int>("maxGroups", 2);
	sceensLeftOffset = Setting<int>("sceensLeftOffset", 20);
	activeGroup = Setting<int>("activeGroup", 0);
	isFullScreen = Setting<bool>("isFullScreen", false);
	isSpoutActive = Setting<bool>("isSpoutActive", false);
	isReplayActive = Setting<bool>("isReplayActive", false);

    compositionWidth = Setting<int>("compositionWidth",1920).setSteps(10);
    compositionHeight = Setting<int>("compositionHeight",1080).setSteps(10);

	nrOfScreens = Setting<int>("nrOfScreens",1);
	fadeoutFactorDrawing = Setting<float>("fadeoutFactorDrawing", 1).setMin(0).setMax(1000).setSteps(1);
	fadeoutFactorReplay = Setting<float>("fadeoutFactorReplay", 4).setMin(0).setMax(1000).setSteps(1);
	replayStartTime = Setting<int>("replayStartTime", 10).setMin(0).setMax(60 * 10).setSteps(1);
	replayInterval = Setting<int>("replayInterval", 100).setMin(0).setSteps(1);


	previewScale = Setting<float>("previewScale", 1).setMin(0).setMax(4).setSteps(0.01);

	

   
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
	//addSetting(&fadeoutFactorReplay);
	//addSetting(&replayStartTime);
	addSetting(&previewScale);
	addSetting(&isSpoutActive);
	addSetting(&isReplayActive);
	addSetting(&replayInterval);


    compositionSize = ci::ivec2(compositionWidth.value(),compositionHeight.value());
    
    
    fboBackground = ci::ColorA(0.0,0.0,0.0,1.0);
    brushColor = ci::ColorA(0.0, 1.0,1.0,0.0);

}








