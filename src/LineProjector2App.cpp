#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "GlobalSettings.h"

#include "Composition.hpp"
#include "BrushManager.h"
#include "NetworkHelper.h"
#include "WindowData.h"
#include "Warp.h"
#include "Lab101Utils.h"

#include "../blocks/Base/src/Settings/SettingController.h"



using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ph::warping;

class LineProjector2App : public App {
    
    std::shared_ptr<Composition>    mActiveComposition;
    std::queue<std::vector<ci::vec3> > pointsQueue;
    
    void setupComposition(std::shared_ptr<Composition>& composition,ci::ivec2 size, bool hasHistory = false);
    NetworkHelper*       mNetworkHelper;
    
    WarpList		mWarps;
    fs::path		mWrapSettings;
    int activeWindow;
	SettingController   mSettingController;
	ci::vec2		mMousePosition;
    
public:
    void setup() override;
    
    void mouseMove(MouseEvent event) override;
    void mouseDown(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void mouseUp(MouseEvent event) override;
    
    void keyDown(KeyEvent event) override;
    void keyUp(KeyEvent event) override;
    
    void update() override;
    void draw() override;
};


void LineProjector2App::setup()
{
    

	activeWindow = -1;
	int nrOfScreens = 3;
	int screenOrder[3] = {1,2,3};


    bool flipHorizontal = true;
    
    float offset = 1.0 / nrOfScreens;

    float scale = 0.5;
    float offsetLeft = 0;
	ci::vec2 size(GS()->compositionWidth.value(), GS()->compositionHeight.value());

    
    
    getWindow()->setUserData(new WindowData(ci::Rectf(flipHorizontal ? 0 : offset, 1, flipHorizontal ? offset : 0, 0.0), 0 ));
    setWindowPos(offsetLeft + ((screenOrder[0]-1) * size.x * scale), 120);
    setWindowSize(size * scale);
    getWindow()->setTitle("Window 1");
    
    
    CI_LOG_I("SETUP brush");
    GS()->brushColor = ci::ColorA(.0, 1.0, .0, 1.0);
    
    BrushManagerSingleton::Instance()->setup();
    
    
    
    CI_LOG_I("SETUP composition with FBO");
    setupComposition(mActiveComposition,ivec2(size.x * nrOfScreens,size.y));
    
    
    mNetworkHelper = new NetworkHelper();
    mNetworkHelper->setup();
    
	// incoming points
    mNetworkHelper->onReceivePoints.connect([=](std::vector<ci::vec3>& points, bool isEraserOn, std::string color){
        BrushManagerSingleton::Instance()->isEraserOn = isEraserOn;

        for(auto&p : points){
            p.x *= mActiveComposition->mSize.x;
            p.y *= mActiveComposition->mSize.y;
        }
        GS()->brushColor =hexStringToColor(color);
        mActiveComposition->drawInFbo(points, hexStringToColor(color));
    });

	// incoming shapes
    mNetworkHelper->onReceiveShapes.connect([=] (std::vector<ci::vec3>& points, std::string shape, std::string color){
        GS()->brushColor =hexStringToColor(color);
        
        for (auto&p : points){
            p.x *= mActiveComposition->mSize.x;
            p.y *= mActiveComposition->mSize.y;
        }
        
        if(shape == "RECT"){
            mActiveComposition->drawRectangle(points[0],points[1], hexStringToColor(color));
        }
        else if(shape == "CIRCLE"){
            
            mActiveComposition->drawCircle(points[0],points[1], hexStringToColor(color));
        }
        else if(shape == "LINE"){
            mActiveComposition->drawLine(points[0],points[1], hexStringToColor(color));
        }
    });
    
    
    // DRAW screen numbers
	float posCounter = 1;

	for (int i = 1; i <= nrOfScreens; i++){
		float posX = posCounter - (0.25 * ((i - 1) / 2));
		for (int j = 1; j <= i; j++){

			float  xPosMid = offsetLeft + (size.x * 0.5) * (posX);

			mActiveComposition->newLine(vec3(xPosMid, size.y / 4, 10));
			mActiveComposition->lineTo(vec3(xPosMid, size.y*0.7, 10), hexStringToColor("#FF00FF"));
			mActiveComposition->endLine();

			mActiveComposition->newLine(vec3(xPosMid - 70, size.y / 4, 10));
			mActiveComposition->lineTo(vec3(xPosMid + 70, size.y / 4, 10), hexStringToColor("#FF00FF"));
			mActiveComposition->endLine();

			mActiveComposition->newLine(vec3(xPosMid - 70, size.y*0.7, 10));
			mActiveComposition->lineTo(vec3(xPosMid + 70, size.y*0.7, 10), hexStringToColor("#FFFFFF"));
			mActiveComposition->endLine();

			posX += 0.25;
		}
		posCounter += 2;
	}
    
    
    for (int i = 0; i < nrOfScreens-1; i++){
        vec2 position(offsetLeft + (size.x * scale) * (screenOrder[i+1]-1) , 120);
        app::WindowRef newWindow2 = createWindow(Window::Format().size(size * scale).pos(position));
        
        
        float offsetX1 = offset * (i + (flipHorizontal ? 1 : 2));
        float offsetX2 = offset * (i + (flipHorizontal ? 2 : 1));
        
        newWindow2->setUserData(new WindowData(ci::Rectf(offsetX1, 1, offsetX2, 0), i+1));
        newWindow2->setTitle("Window " + toString(i+2));
        
        
    }
    
    
    // setup wraps
    // initialize warps
    mWrapSettings = getAssetPath("") / "warps.xml";
    if (fs::exists(mWrapSettings)) {
        // load warp settings from file if one exists
        mWarps = Warp::readSettings(loadFile(mWrapSettings));
    }
    else {
        // otherwise create a warp from scratch
        for (int i = 0; i < nrOfScreens; i++){
            auto w = WarpPerspective::create();
            w->setSize(size);
            mWarps.push_back(w);
        }
    }
    
    // adjust the content size of the warps
    Warp::setSize(mWarps, size);
	mSettingController.setup();

}


void LineProjector2App::update()
{
    mNetworkHelper->update();
}



void LineProjector2App::setupComposition(std::shared_ptr<Composition>& composition,ci::ivec2 size , bool hasHistory){
    
    composition = make_shared<Composition>();
    //composition->setup(GS()->compositionSize);
    composition->setup(size);
    
    // when the new points with correct spacing are calculated we send them to the other
    // clients we don't send rawpoints.
    composition->onNewPoints.connect([=](pointVec p){
        //mNetworkHelper.sendPoints(p, BrushManagerSingleton::Instance()->isEraserOn);
    });
}


void LineProjector2App::draw()
{
    
    ci::gl::lineWidth(4);
    
    WindowData *data = getWindow()->getUserData<WindowData>();
   
	if (GS()->debugMode.value()){
		if (activeWindow > -1 && data->mId != activeWindow){
			gl::clear(ColorA(49.0f / 255.0f, 24.0f / 255.0f, 160.0f / 25.0f, 1.0f));
		}
		else{
			gl::clear(ColorA(40 / 255.0f, 40 / 255.0f, 40 / 255.0f, 1.0f));
			ci::gl::color(1, 1, 1);
			ci::gl::drawSolidCircle(mMousePosition, 10);
		}
	}
	else{
		gl::clear(ColorA(0, 0, 0, 1.0f));
	}
    


    
    
    mWarps[data->mId]->begin();
    mActiveComposition->draw(data->mDrawingArea);
    mWarps[data->mId]->end();
	
	if (GS()->doFadeOut.value())  mActiveComposition->drawFadeOut();
    

	if (GS()->debugMode.value()){
		ci::gl::enableAlphaBlending();
		//NotificationManagerSingleton::Instance()->draw();
		mSettingController.draw();
	}

   
    
}



// WRAPPING

void LineProjector2App::mouseMove(MouseEvent event)
{

	mMousePosition = event.getPos();

    if(mWarps.size() == 0) return;
    
    WarpList		mWarpsSelected;
    if (activeWindow == -1){
        mWarpsSelected = mWarps;
    }
    else{
        mWarps[activeWindow]->mouseMove(event);
    }
    return;
    
    
}

void LineProjector2App::mouseDown(MouseEvent event)
{
	mMousePosition = event.getPos();

    if(mWarps.size() == 0) return;
    WarpList		mWarpsSelected;
    
    if (activeWindow == -1){
        mWarpsSelected = mWarps;
    }
    else{
        mWarps[activeWindow]->mouseDown(event);
    }
    return;
    
    // pass this mouse event to the warp editor first
    
    if (!Warp::handleMouseDown(mWarpsSelected, event)) {
        // let your application perform its mouseDown handling here
    }
}

void LineProjector2App::mouseDrag(MouseEvent event)
{
	mMousePosition = event.getPos();

    if(mWarps.size() == 0) return;
    WarpList		mWarpsSelected;
    
    if (activeWindow == -1){
        mWarpsSelected = mWarps;
    }
    else{
        mWarps[activeWindow]->mouseDrag(event);
    }
    return;
    
    // pass this mouse event to the warp editor first
    if (!Warp::handleMouseDrag(mWarps, event)) {
        // let your application perform its mouseDrag handling here
    }
}

void LineProjector2App::mouseUp(MouseEvent event)
{
	mMousePosition = event.getPos();

    if(mWarps.size() == 0) return;
    WarpList		mWarpsSelected;
    
    if (activeWindow == -1){
        mWarpsSelected = mWarps;
    }
    else{
        mWarps[activeWindow]->mouseUp(event);
    }
    
    return;
    // pass this mouse event to the warp editor first
    if (!Warp::handleMouseUp(mWarps, event)) {
        // let your application perform its mouseUp handling here
    }
}

void LineProjector2App::keyDown(KeyEvent event)
{

	if (event.getCode() == event.KEY_d){
		GS()->debugMode.value() = !GS()->debugMode.value();
	}
    

	if (mSettingController.checkKeyDown(event))
	{
		return;
	}

    // pass this key event to the warp editor first
    if (!Warp::handleKeyDown(mWarps, event)) {
        // warp editor did not handle the key, so handle it here
        switch (event.getCode()) {
            case KeyEvent::KEY_ESCAPE:
                // quit the application
                quit();
                break;
            case KeyEvent::KEY_f:
                // toggle full screen
                setFullScreen(!isFullScreen());
                break;
            case KeyEvent::KEY_v:
                // toggle vertical sync
                gl::enableVerticalSync(!gl::isVerticalSyncEnabled());
                break;
			case KeyEvent::KEY_w:
				// toggle warp edit mode
				Warp::enableEditMode(!Warp::isEditModeEnabled());
				break;

			case KeyEvent::KEY_c:
				// toggle warp edit mode
				mActiveComposition->clearScene();
				break;

            case KeyEvent::KEY_0:
                activeWindow = -1;
                break;
                
            case KeyEvent::KEY_1:
                activeWindow = 0;
                break;
            case KeyEvent::KEY_2:
				if (GS()->nrOfScreens.value() > 1) activeWindow = 1;
                break;
            case KeyEvent::KEY_3:
				if (GS()->nrOfScreens.value() > 2) activeWindow = 2;
                break;
            case KeyEvent::KEY_4:
				if (GS()->nrOfScreens.value() > 3) activeWindow = 3;
                break;
            case KeyEvent::KEY_r:
              
                break;
                
        }
    }
}

void LineProjector2App::keyUp(KeyEvent event)
{
    if(mWarps.size() == 0) return;
    // pass this key event to the warp editor first
    if (!Warp::handleKeyUp(mWarps, event)) {
        // let your application perform its keyUp handling here
    }
}

CINDER_APP( LineProjector2App, RendererGl )
