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

#include "CiSpoutOut.h"

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
    
	// wrapping stuff
    WarpList		mWarps;
    fs::path		mWrapSettings;


    int activeWindow;
    SettingController   mSettingController;
    ci::vec2		mMousePosition;

	// spout
	SpoutOut* mSpoutOut;

    
public:

	LineProjector2App();
    void setup() override;
	void setupNetwork();
	void drawScreenNumbers();
    
    void mouseMove(MouseEvent event) override;
    void mouseDown(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void mouseUp(MouseEvent event) override;
    
    void keyDown(KeyEvent event) override;
    void keyUp(KeyEvent event) override;
    
    void update() override;
    void draw() override;
};


LineProjector2App::LineProjector2App() {


}

void LineProjector2App::setup()
{
    

	activeWindow = 0;
	int nrOfScreens = GS()->nrOfScreens.value();
	int screenOrder[4] = { 1, 2, 3, 4 };
//	int screenOrder[4] = { 1, 4, 3, 2 };

    bool flipHorizontal = false;
    
    float offset = 1.0 / nrOfScreens;
	float scale = GS()->windowScale.value();

	ci::vec2 size(GS()->compositionWidth.value(), GS()->compositionHeight.value());

    
  	getWindow()->setUserData(new WindowData(ci::Rectf(flipHorizontal ? offset : 0, 1, flipHorizontal ? 0 : offset, 0.0), 0));
    setWindowPos(GS()->sceensLeftOffset.value() + ((screenOrder[0]-1) * size.x * scale), 120);
    setWindowSize(size * scale);
    getWindow()->setTitle("Window 1");

	// set first projector window.
	// debug window is initiated later.
	if (GS()->isFullScreen.value()) {
		setFullScreen(true);
	}
    
    CI_LOG_I("SETUP brush");
    GS()->brushColor = ci::ColorA(.0, 1.0, .0, 1.0);
    
    BrushManagerSingleton::Instance()->setup();
	mSettingController.setup();

    
    CI_LOG_I("SETUP composition with FBO");
    setupComposition(mActiveComposition,ivec2(size.x * nrOfScreens,size.y));
    
    
	setupNetwork();
	drawScreenNumbers();
	    

	vec2 position(20,120);
	app::WindowRef debugWindow = createWindow(Window::Format().size(800,800).pos(position));


	debugWindow->setUserData(new WindowData(ci::Rectf(0, 0, 1, 1),  -100));
	debugWindow->setTitle("DEBUG ");
    
    // creating the EXTRA WINDOWS
    for (int i = 0; i < nrOfScreens-1; i++){
		vec2 position(GS()->sceensLeftOffset.value() + (size.x * scale) * (screenOrder[i + 1] - 1), 0);
        app::WindowRef newWindow2 = createWindow(Window::Format().size(size * scale).pos(position).fullScreen(true));
        
        
        float offsetX1 = offset * (i + (flipHorizontal ? 2 : 1));
        float offsetX2 = offset * (i + (flipHorizontal ? 1 : 2));
        
        newWindow2->setUserData(new WindowData(ci::Rectf(offsetX1, 1, offsetX2, 0), i+1));
        newWindow2->setTitle("Window " + toString(i+2));
        
		newWindow2->setFullScreen(GS()->isFullScreen.value());
    }
    
    
    // setup WRAPS
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
			w->resize(size);
            mWarps.push_back(w);
        }
    }
    

	// SPOUT
	mSpoutOut = new ci::SpoutOut("lineproject", size);
	


}


void LineProjector2App::update()
{
    mNetworkHelper->update();
}


void LineProjector2App::setupNetwork(){

	mNetworkHelper = new NetworkHelper();
	mNetworkHelper->setup();

	// incoming points
	mNetworkHelper->onReceivePoints.connect([=](PointsPackage package){
        bool currentEraser = BrushManagerSingleton::Instance()->isEraserOn;
        BrushManagerSingleton::Instance()->isEraserOn = package.isEraserOn;

		for (auto&p : package.points){
			p.x *= mActiveComposition->getSize().x;
			p.y *= mActiveComposition->getSize().y;
		}
        if(BrushManagerSingleton::Instance()->isEraserOn)
        {
           GS()->brushColor = GS()->fboBackground;
        } else{
           GS()->brushColor = hexStringToColor(package.color);
        }
		
		mActiveComposition->drawInFbo(package.points, hexStringToColor(package.color));
        BrushManagerSingleton::Instance()->isEraserOn = currentEraser;
	});

	// incoming shapes
	mNetworkHelper->onReceiveShapes.connect([=](PointsPackage package){
		GS()->brushColor = hexStringToColor(package.color);

		for (auto&p : package.points){
			p.x *= mActiveComposition->getSize().x;
			p.y *= mActiveComposition->getSize().y;
		}

		if (package.shape == "RECT"){
			mActiveComposition->drawRectangle(package.points[0], package.points[1], hexStringToColor(package.color));
		}
		else if (package.shape == "CIRCLE"){

			mActiveComposition->drawCircle(package.points[0], package.points[1], hexStringToColor(package.color));
		}
		else if (package.shape == "LINE"){
			mActiveComposition->drawLine(package.points[0], package.points[1], hexStringToColor(package.color));
		}
	});

}



void LineProjector2App::drawScreenNumbers(){
	// DRAW screen numbers
	float posCounter = 1;
	vec2 size = mActiveComposition->getSize();
	size.x /= GS()->nrOfScreens.value();

	for (int i = 1; i <= GS()->nrOfScreens.value(); i++){
		float posX = posCounter - (0.25 * ((i - 1) / 2));
		for (int j = 1; j <= i; j++){

			float  xPosMid = (size.x * 0.5) * (posX);

			mActiveComposition->newLine(vec3(xPosMid, size.y / 4, 10));
			mActiveComposition->lineTo(vec3(xPosMid, size.y*0.7, 10), hexStringToColor("#00FFFF"));
			mActiveComposition->endLine();

			mActiveComposition->newLine(vec3(xPosMid - 70, size.y / 4, 10));
			mActiveComposition->lineTo(vec3(xPosMid + 70, size.y / 4, 10), hexStringToColor("#00FFFF"));
			mActiveComposition->endLine();

			mActiveComposition->newLine(vec3(xPosMid - 70, size.y*0.7, 10));
			mActiveComposition->lineTo(vec3(xPosMid + 70, size.y*0.7, 10), hexStringToColor("#0000FF"));
			mActiveComposition->endLine();

			posX += 0.25;
		}
		posCounter += 2;
	}

}

void LineProjector2App::setupComposition(std::shared_ptr<Composition>& composition,ci::ivec2 size , bool hasHistory){
    
    composition = make_shared<Composition>();
    composition->setup(size);
 }


void LineProjector2App::draw()
{
    
    WindowData *data = getWindow()->getUserData<WindowData>();
    
	if ( data->mId == -100){
		gl::clear();

		ci::gl::enableAlphaBlending();

		ci::gl::pushMatrices();
		ci::gl::scale(GS()->previewScale.value(), GS()->previewScale.value());


		mActiveComposition->draw(ci::Rectf(0,1,1,0));
		
	//	mSpoutOut->sendTexture(mActiveComposition->getTexture());
		
		ci::gl::popMatrices();

		//NotificationManagerSingleton::Instance()->draw();
		ci::gl::enableAlphaBlending();

		if (GS()->debugMode.value()) mSettingController.draw();
		return;
	}

    if (GS()->debugMode.value()){
        if (activeWindow > -1 && data->mId != activeWindow){
            gl::clear(ColorA(49.0f / 255.0f, 24.0f / 255.0f, 160.0f / 25.0f, 1.0f));
        }
        else{
			gl::clear(ColorA(0, 0, 0, 1.0f));
			//ci::gl::color(1, 1, 1);
            ///ci::gl::drawSolidCircle(mMousePosition, 10);
        }
    }
    else{
        gl::clear(ColorA(0, 0, 0, 1.0f));
    }
    
    
    
    mWarps[data->mId]->begin();
    mActiveComposition->draw(data->mDrawingArea);
	if (GS()->doFadeOut.value())  mActiveComposition->drawFadeOut();

	
	mWarps[data->mId]->end();
    
    
    
	
    
    
    
}



// WRAPPING

void LineProjector2App::mouseMove(MouseEvent event)
{


	mMousePosition = event.getPos();

	if (mWarps.size() == 0) return;

	if (activeWindow != -1){
		mWarps[activeWindow]->mouseMove(event);
	}

}



void LineProjector2App::mouseDown(MouseEvent event)
{
    mMousePosition = event.getPos();
    
    if(mWarps.size() == 0) return;
    
    if (activeWindow != -1){
        mWarps[activeWindow]->mouseDown(event);
    }
    
}

void LineProjector2App::mouseDrag(MouseEvent event)
{
    mMousePosition = event.getPos();
    
    if(mWarps.size() == 0) return;

	if (activeWindow != -1){
		mWarps[activeWindow]->mouseDrag(event);
	}
}

void LineProjector2App::mouseUp(MouseEvent event)
{
	mMousePosition = event.getPos();
	if (mWarps.size() == 0) return;

	if (activeWindow != -1){
		mWarps[activeWindow]->mouseUp(event);
	}
    mMousePosition = event.getPos();
    

}


void LineProjector2App::keyDown(KeyEvent event)
{
    
    if (event.getCode() == event.KEY_d){
        GS()->debugMode.value() = !GS()->debugMode.value();
    }
    
	if (event.getCode() == event.KEY_j){ // FADE
		if (GS()->fadeoutFactorDrawing.value() > 0){
			GS()->fadeoutFactorDrawing.decreaseStep(1);
		}
	};

	if (event.getCode() == event.KEY_l){ //FADE 
		GS()->fadeoutFactorDrawing.increaseStep(1);
	}
    
    if (mSettingController.checkKeyDown(event))
    {
    //    return;
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

			case KeyEvent::KEY_s:
				// toggle warp edit mode
				Warp::writeSettings(mWarps, writeFile(mWrapSettings));
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
