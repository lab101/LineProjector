//
//  ReplayManager.h
//  LiveDraw
//
//  Created by lab101 on 05/11/19.
//
//

#pragma once
#include "PointsPackage.hpp"

class ReplayManager {

	std::vector<PointsPackage> mDrawingCommands;
	int			mCurrentDrawingIndex = 0;


public:

	void writeData(PointsPackage& pp);
	void readData();
	PointsPackage& getNextCommand();
	bool hasData();
};