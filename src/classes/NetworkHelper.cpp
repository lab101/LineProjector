//
//  NetworkHelper.cpp
//  LineDancer
//
//  Created by Kris Meeusen on 21/08/2017.
//
//

#include "NetworkHelper.h"
#include "cinder/Utilities.h"
#include "GlobalSettings.h"

using namespace ci;
using namespace ci::osc;





NetworkHelper::NetworkHelper()
	: mIoService(new asio::io_service), mWork(new asio::io_service::work(*mIoService)),
	mReceiver(3000, asio::ip::udp::v4(), *mIoService)
{
}


bool NetworkHelper::setup(){

	mReceiver.setListener("/points",
		[&](const osc::Message &msg){
		//std::lock_guard<std::mutex> lock(mCirclePosMutex);
		//mCurrentCirclePos.x = msg[0].int32();
		//mCurrentCirclePos.y = msg[1].int32();


		int totals = msg.getNumArgs();
		int incomingGroupId = msg[0].int32();
		if (incomingGroupId != GS()->activeGroup.value()) return;

		bool isEraserOn = msg.getArgBool(1);
		std::string color = msg.getArgString(2);
		std::vector<ci::vec3> points;
		for (int i = 3; i < totals; i += 3){
			points.push_back(ci::vec3(msg[i].flt(), msg[i + 1].flt(), msg[i + 2].flt()));
		}
            PointsPackage newPackage;
            newPackage.setup(points, color);
            newPackage.setEraser(isEraserOn); ////// ERASER
            
		mPointsQueueLock.lock();
		pointsQueue.push(newPackage);
		mPointsQueueLock.unlock();

	});

	mReceiver.setListener("/shape",
		[&](const osc::Message &msg){
		int totals = msg.getNumArgs();

		int incomingGroupId = msg[0].int32();
		if (incomingGroupId != GS()->activeGroup.value()) return;

		std::string shape = msg.getArgString(1);
		std::string color = msg.getArgString(2);
		std::vector<ci::vec3> points;
		for (int i = 3; i < totals; i += 3){
			points.push_back(ci::vec3(msg[i].flt(), msg[i + 1].flt(), msg[i + 2].flt()));
		}
			
		
		PointsPackage newPackage;
        newPackage.setup(points, color);
        newPackage.setShape(shape);
		mShapesQueueLock.lock();
		shapesQueue.push(newPackage);
		mShapesQueueLock.unlock();

	});

	// For a description of the below setup, take a look at SimpleReceiver. The only difference
	// is the usage of the mutex around the connection map.
	try {
		mReceiver.bind();
	}
	catch (const osc::Exception &ex) {
		CI_LOG_E("Error binding: " << ex.what() << " val: " << ex.value());
	}


	mReceiver.listen(
		[](asio::error_code error, protocol::endpoint endpoint) -> bool {
		if (error) {
			CI_LOG_E("Error Listening: " << error.message() << " val: "
				<< error.value() << " endpoint: " << endpoint);
			return false;
		}
		else
			return true;
	});



	// Now that everything is setup, run the io_service on the other thread.
	mThread = std::thread(std::bind(
		[](std::shared_ptr<asio::io_service> &service){
		service->run();
	}, mIoService));


	return true;
}

void NetworkHelper::setNextGroup(){
	if (++groupId > GS()->maxGroups.value() - 1) groupId = 0;
}


void NetworkHelper::update(){

	if (ci::app::getElapsedSeconds() - lastBroadcast > 4){
		sendAlive();
		lastBroadcast = app::getElapsedSeconds();
	}


	mPointsQueueLock.lock();

	while (!pointsQueue.empty())
	{

		onReceivePoints.emit(pointsQueue.front());
		pointsQueue.pop();
	}

	mPointsQueueLock.unlock();

	mShapesQueueLock.lock();

	while (!shapesQueue.empty())
	{
		onReceiveShapes.emit(shapesQueue.front());
		shapesQueue.pop();
	}

	mShapesQueueLock.unlock();



}


std::string const NetworkHelper::getLastMyIpNr(){
	return mLastIpNr;
}

int const NetworkHelper::getGroupId(){
	// add one makes it nicer for non programmers
	return groupId + 1;
}


std::string NetworkHelper::extractLastIpNr(std::string& fullIp){

	std::vector<std::string> hostSplit = ci::split(fullIp, ".");
	return hostSplit.back();

}





void NetworkHelper::sendAlive(){

	/* osc::Message message;
	message.setAddress("alive");
	message.addIntArg(groupId);
	mSender.sendMessage(message);*/
}



void NetworkHelper::sendPoints(std::vector<ci::vec3>& points, bool isEraserOn){


}


