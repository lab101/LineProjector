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
    
<<<<<<< HEAD
	mReceiver.setListener("points",
		[&](const osc::Message &msg){
		//std::lock_guard<std::mutex> lock(mCirclePosMutex);
		//mCurrentCirclePos.x = msg[0].int32();
		//mCurrentCirclePos.y = msg[1].int32();

		int totals = msg.getNumArgs();
		bool isEraserOn = msg[1].int32();


		std::vector<ci::vec3> points;
		for (int i = 2; i < totals; i += 3){
			points.push_back(ci::vec3(msg[i].flt(), msg[i + 1].flt(), msg[i+2].flt()* 2.0));
		}


		mPointsQueueLock.lock();
		pointsQueue.push(points);
		mPointsQueueLock.unlock();

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
=======
    mReceiver.setListener("points",
                          [&](const osc::Message &msg){
                              //std::lock_guard<std::mutex> lock(mCirclePosMutex);
                              //mCurrentCirclePos.x = msg[0].int32();
                              //mCurrentCirclePos.y = msg[1].int32();
                              
                              int totals = msg.getNumArgs();
                              // bool isEraserOn = msg.getArgInt32(1);
                              std::string color = msg.getArgString(2);
                              std::vector<ci::vec3> points;
                              for(int i=3;i < totals;i+=3){
                                  points.push_back(ci::vec3(msg[i].flt(), msg[i + 1].flt(), msg[i+2].flt()));
                              }
                              mPointsQueueLock.lock();
                              pointsQueue.push(points);
                              hexColor = color;
                              mPointsQueueLock.unlock();
                              
                          });
    
    mReceiver.setListener("shape",
                          [&](const osc::Message &msg){
                              int totals = msg.getNumArgs();
                              std::string shape = msg.getArgString(1);
                              std::string color = msg.getArgString(2);
                              std::vector<ci::vec3> points;
                              for(int i=3;i < totals;i+=3){
                                  points.push_back(ci::vec3(msg[i].flt(), msg[i + 1].flt(), msg[i+2].flt()));
                              }
                              mShapesQueueLock.lock();
                              shapesQueue.push(points);
                              receivedShape = shape;
                              hexColor = color;
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
    
    
    //try{
    //    setupOSCSender();
    //   // mListener.setup(3000);
    //
    //}catch(...){
    //    return false;
    //}
    //
    //return true;
    
    
    return true;
>>>>>>> e9ec6ae50c20d5c16c1882855467933483d85288
}

void NetworkHelper::setNextGroup(){
    if(++groupId > GS()->maxGroups.value()-1) groupId = 0;
}


void NetworkHelper::update(){
    
    if(ci::app::getElapsedSeconds() - lastBroadcast > 4){
        sendAlive();
        lastBroadcast = app::getElapsedSeconds();
    }
<<<<<<< HEAD

	
	mPointsQueueLock.lock();

	while (!pointsQueue.empty())
	{
		onReceivePoints.emit(pointsQueue.front(), false);
		pointsQueue.pop();
	}

	mPointsQueueLock.unlock();



=======
    
    
    mPointsQueueLock.lock();
    
    while (!pointsQueue.empty())
    {
        
        onReceivePoints.emit(pointsQueue.front(), false,hexColor);
        pointsQueue.pop();
    }
    
    mPointsQueueLock.unlock();
    
    mShapesQueueLock.lock();
    
    while (!shapesQueue.empty())
    {
        onReceiveShapes.emit(shapesQueue.front(), receivedShape, hexColor);
        shapesQueue.pop();
    }
    
    mShapesQueueLock.unlock();
    
    
    
    //while( mListener.hasWaitingMessages() ) {
    //    osc::Message message;
    //    mListener.getNextMessage( &message );
    //
    //    // return from our own broadcast
    //    std::string remoteIp = message.getRemoteIp();
    //    if( remoteIp ==  mOwnIpAdress)
    //        continue;
    //
    //    std::string remoteLastNr = extractLastIpNr(remoteIp);
    //    std::string const adress = message.getAddress();
    //
    //    int incomingGroupId =  message.getArgAsInt32(0);
    //
    //    // discard packages from other groups
    //    if(incomingGroupId == groupId){
    
    //
    //        if(mAliveIps.find(remoteLastNr) == mAliveIps.end()){
    //            onNewConnection.emit(remoteLastNr);
    //        }
    //
    //        mAliveIps[remoteLastNr] = ci::app::getElapsedSeconds();
    //        onAlivePing.emit(remoteLastNr);
    
    
    //        if(adress == "points"){
    //            int totals = message.getNumArgs() ;
    //            bool isEraserOn = message.getArgAsInt32(1);
    
    //
    //            std::vector<ci::vec3> points;
    //            for(int i=2;i < totals;i+=3){
    //                points.push_back(ci::vec3(message.getArgAsFloat(i),message.getArgAsFloat(i+1),message.getArgAsFloat(i+2)));
    //            }
    //
    //            onReceivePoints.emit(points,isEraserOn);
    //        }
    //    }
    //
    //
    //}
    
>>>>>>> e9ec6ae50c20d5c16c1882855467933483d85288
}


std::string const NetworkHelper::getLastMyIpNr(){
    return mLastIpNr;
}

int const NetworkHelper::getGroupId(){
    // add one makes it nicer for non programmers
    return groupId +1;
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
    /*  osc::Message message;
     message.setAddress("points");
     message.addIntArg(groupId);
     message.addIntArg(isEraserOn);
     
     for(vec3& p : points){
     message.addFloatArg(p.x);
     message.addFloatArg(p.y);
     message.addFloatArg(p.z);
     }
     
     mSender.sendMessage(message);
     lastBroadcast = app::getElapsedSeconds();
     */
    
}



