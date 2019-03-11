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
    bool flipHorizontal = true;
    
    float offset = 1.0 / nrOfScreens;
    float scale = 0.75;
    float offsetLeft = 0;
    ci::vec2 size(1920, 1080);
    
    
    getWindow()->setUserData(new WindowData(ci::Rectf(flipHorizontal ? 0 : offset, 1, flipHorizontal ? offset : 0, 0.0), 0));
    setWindowPos(offsetLeft, 120);
    setWindowSize(size * scale);
    getWindow()->setTitle("MAIN window");
    
    
    CI_LOG_I("SETUP brush");
    GS()->brushColor = ci::ColorA(0.0, 0.0, 0.0, 1.0);
    
    BrushManagerSingleton::Instance()->setup();
    
    
    
    CI_LOG_I("SETUP composition with FBO");
    setupComposition(mActiveComposition,ivec2(size.x * nrOfScreens,size.y));
    
    
    mNetworkHelper = new NetworkHelper();
    mNetworkHelper->setup();
    
    mNetworkHelper->onReceivePoints.connect([=](std::vector<ci::vec3>& points, bool isEraserOn, std::string color){
        BrushManagerSingleton::Instance()->isEraserOn = isEraserOn;
        
		//points[0].x *= mActiveComposition->mSize.x;
		//points[0].y *= mActiveComposition->mSize.y;

		//mActiveComposition->newLine(points[0]);
		//for (int i = 1; i < points.size(); i++){
		//	points[i].x *= mActiveComposition->mSize.x;
		//	points[i].y *= mActiveComposition->mSize.y;


		//	mActiveComposition->lineTo(points[i] , hexStringToColor(color));

		//}

		//mActiveComposition->endLine();
	


        for(auto&p : points){
            p.x *= mActiveComposition->mSize.x;
            p.y *= mActiveComposition->mSize.y;
        }
        GS()->brushColor =hexStringToColor(color);
        mActiveComposition->drawInFbo(points, hexStringToColor(color));
    });
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
    
    
    
    //CROSS
    //        mActiveComposition->newLine(vec3(10, 10, 20));
    //     mActiveComposition->lineTo(vec3(size.x * nrOfScreens - 10, size.y - 20, 20),ci::ColorA(1.0, 1.0, 1.0, 1.0));
    //     mActiveComposition->endLine();
    //     GS()->brushColor = ci::ColorA(1.0, 1.0, 0.0, 1.0);
    //
    //     mActiveComposition->newLine(vec3(size.x * nrOfScreens - 10, 10, 20));
    //     mActiveComposition->lineTo(vec3(10, size.y, 20),ci::ColorA(1.0, 1.0, 1.0, 1.0));
    //     mActiveComposition->endLine();
    float posCounter =2;
    
    for(int i = 1; i <= nrOfScreens; i++){
        float posX = posCounter - (0.25 * (i-1) );
        for(int j = 1; j <= i; j++){
            
            float  xPosMid = offsetLeft + (size.x * scale) * (posX);
            
            mActiveComposition->newLine(vec3(xPosMid,size.y/4,10));
            mActiveComposition->lineTo(vec3(xPosMid,size.y*0.7,10),hexStringToColor("#000000"));
            mActiveComposition->endLine();
            
            mActiveComposition->newLine(vec3(xPosMid-70,size.y/4,10));
            mActiveComposition->lineTo(vec3(xPosMid+70,size.y/4,10),hexStringToColor("#000000"));
            mActiveComposition->endLine();
            
            mActiveComposition->newLine(vec3(xPosMid-70,size.y*0.7,10));
            mActiveComposition->lineTo(vec3(xPosMid+70,size.y*0.7,10),hexStringToColor("#000000"));
            mActiveComposition->endLine();
            
            posX +=0.5 ;
        }
        posCounter += 4;
    }
    
    
    
    
    //   mActiveComposition->newLine(vec3(offsetLeft + (size.x * scale) * (6),size.y/4,10));
    //   mActiveComposition->lineTo(vec3(offsetLeft + (size.x * scale) * (6),size.y*0.7,10),ci::ColorA(1.0, 1.0, 1.0, 1.0));
    //                mActiveComposition->endLine();
    //
    //    mActiveComposition->newLine(vec3(offsetLeft + (size.x * scale) * (10),size.y/4,10));
    //    mActiveComposition->lineTo(vec3(offsetLeft + (size.x * scale) * (10),size.y*0.7,10),ci::ColorA(1.0, 1.0, 1.0, 1.0));
    //    mActiveComposition->endLine();
    
    
    for (int i = 0; i < nrOfScreens-1; i++){
        
        
        
        vec2 position(offsetLeft + (size.x * scale) * (i+1) , 120);
        //______________________________________________________________________________ARRAY OF POS {3,1,2}
        app::WindowRef newWindow2 = createWindow(Window::Format().size(size * scale).pos(position));
        
        
        float offsetX1 = offset * (i + (flipHorizontal ? 1 : 2));
        float offsetX2 = offset * (i + (flipHorizontal ? 2 : 1));
        
        newWindow2->setUserData(new WindowData(ci::Rectf(offsetX1, 1, offsetX2, 0), i + 1));
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
    
    if (activeWindow > -1 && data->mId != activeWindow){
        gl::clear(ColorA(49.0f / 255.0f, 24.0f / 255.0f, 160.0f / 25.0f, 1.0f));
    } else{
        gl::clear(ColorA(249.0f / 255.0f, 242.0f / 255.0f, 160.0f / 255.0f, 1.0f));
    }
    
    
    
    mWarps[data->mId]->begin();
    mActiveComposition->draw(data->mDrawingArea);

    mWarps[data->mId]->end();
        // mActiveComposition->drawFadeOut();
    
   
    
}



// WRAPPING

void LineProjector2App::mouseMove(MouseEvent event)
{
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
    if(mWarps.size() == 0) return;
    ci::vec2 size(1280, 720);
    
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
                
            case KeyEvent::KEY_0:
                activeWindow = -1;
                break;
                
            case KeyEvent::KEY_1:
                activeWindow = 0;
                break;
            case KeyEvent::KEY_2:
                activeWindow = 1;
                break;
            case KeyEvent::KEY_3:
                activeWindow = 2;
                break;
            case KeyEvent::KEY_4:
                activeWindow = 3;
                break;
            case KeyEvent::KEY_r:
                // adjust the content size of the warps
                for (auto w : mWarps){
                    w->setSize(size);
                    
                }
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
