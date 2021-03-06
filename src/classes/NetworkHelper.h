//
//  NetworkHelper.hpp
//  LineDancer
//
//  Created by Kris Meeusen on 21/08/2017.
//
//

#pragma once

//osc
#include "cinder/osc/Osc.h"
#include "cinder/System.h"
#include "cinder/Signals.h"
#include "PointsPackage.hpp"


#include <map>
#include <queue>

using Receiver = ci::osc::ReceiverUdp;
using protocol = asio::ip::udp;




class NetworkHelper{
    
	std::string mOwnIpAdress;
	std::string mLastIpNr;
	int groupId = 0;
	float lastBroadcast;

	std::queue<PointsPackage> pointsQueue;
	std::mutex mPointsQueueLock;
    
	std::queue<PointsPackage> shapesQueue;
    std::mutex mShapesQueueLock;
    

public:

	NetworkHelper();

    // threaded OSC server
	std::shared_ptr<asio::io_service>		mIoService;
	std::shared_ptr<asio::io_service::work>	mWork;
	std::thread								mThread;

	ci::osc::ReceiverUdp mReceiver;
	std::map<uint64_t, protocol::endpoint> mConnections;


	ci::signals::Signal<void(PointsPackage& pointPackage)>   onReceivePoints;
	ci::signals::Signal<void(PointsPackage& pointPackage)> onReceiveShapes;
    ci::signals::Signal<void(std::string&)>   onNewConnection;
    ci::signals::Signal<void(std::string&)>   onAlivePing;

    
    std::map<std::string,float> mAliveIps;

    bool setup();
    void update();
    void setNextGroup();
	void sendAlive();
	std::string extractLastIpNr(std::string& fullIp);
	std::string const getLastMyIpNr();


    int const getGroupId();

    void sendOscMessage(std::string command,ci::vec3 point);
    void sendPoints(std::vector<ci::vec3>& points,bool isEraserOn);



    
};
