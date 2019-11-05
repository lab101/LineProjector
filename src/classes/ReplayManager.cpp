
#include "ReplayManager.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;


void ReplayManager::readData() {
	std::string mOutputFolder = ci::app::getAssetPath("/").string();
	std::string dataFilePath = mOutputFolder + "/replayData.txt";
	try {
		std::ifstream dataFile(dataFilePath);
		std::string line;

		if (dataFile.is_open()) {

			while (getline(dataFile, line)) {
				std::vector<std::string> splitData = split(line, ";");
				if (splitData.size() > 3) {
					PointsPackage pointsPackage;
					pointsPackage.shape = splitData[0];
					pointsPackage.color = splitData[1];

					for (int i = 2; i < splitData.size()-1; i+=3) {
						vec3 point;
						point.x = stof(splitData[i]);
						point.y = stof(splitData[i+1]);
						point.z = stof(splitData[i+2]);
						pointsPackage.points.push_back(point);

					}
					mDrawingCommands.push_back(pointsPackage);

				}
			}

		}
		dataFile.close();
	}
	catch (...) {
			CI_LOG_E("couldn't read from path: " + dataFilePath);
	}
}

void ReplayManager::writeData(PointsPackage& p) {

	mDrawingCommands.push_back(p);

	// write interpolated points to a data file in the output folder
	std::string mOutputFolder = ci::app::getAssetPath("/").string();
	std::string dataFilePath = mOutputFolder + "/replayData.txt";
	try {
		std::ofstream dataFile;

		dataFile.open(dataFilePath, std::ios_base::app);
		dataFile << p.shape << ";" << p.color << ";";

		for (auto& point : p.points) {
			dataFile << point.x << ";" << point.y << ";" << point.z << ";";
		}

		dataFile << std::endl;
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



