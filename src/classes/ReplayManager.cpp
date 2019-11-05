
#include "ReplayManager.h"
#include "cinder/app/App.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;


void ReplayManager::readData() {

}

void ReplayManager::writeData(PointsPackage& p) {

	mDrawingCommands.push_back(p);

	// write interpolated points to a data file in the output folder
	std::string mOutputFolder = ci::app::getAssetPath("/").string();
	std::string dataFilePath = mOutputFolder + "/replayData.txt";
	try {
		std::ofstream dataFile;

		dataFile.open(dataFilePath);

		/*for (pointVec s : strokes) {
			for (vec3& p : s) {
				dataFile << p.x << "," << p.y << "," << p.z << ";";
			}
			dataFile << std::endl;
		}*/

		dataFile.close();
	}
	catch (...) {
		CI_LOG_E("couldn't write to path: " + dataFilePath);
	}

}


PointsPackage& ReplayManager::getNextCommand() {

	if (++mCurrentDrawingIndex >= mDrawingCommands.size()) mCurrentDrawingIndex = 0;
	return mDrawingCommands[mCurrentDrawingIndex];

}



bool ReplayManager::hasData() {

	return mDrawingCommands.size() > 0;

}



