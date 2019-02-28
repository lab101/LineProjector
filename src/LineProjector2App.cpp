#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "../blocks/Base/src/Settings/GlobalSettings.h"

#include "Composition.hpp"
#include "BrushManager.h"
#include "NetworkHelper.h"
#include "WindowData.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class LineProjector2App : public App {

	std::shared_ptr<Composition>    mActiveComposition;
	std::queue<std::vector<ci::vec3> > pointsQueue;

	void setupComposition(std::shared_ptr<Composition>& composition, bool hasHistory = false);
	NetworkHelper*       mNetworkHelper;

  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void LineProjector2App::setup()
{


	int nrOfScreens = 4;
	float offset = 1.0 / nrOfScreens;
	float scale = 0.5;
	ci::vec2 size(1280, 720);

	getWindow()->setUserData(new WindowData(ci::Rectf(0, 1, offset, 0.0), 0));
	setWindowPos(0, 120);
	setWindowSize(size * scale);
	getWindow()->setTitle("MAIN window");


	CI_LOG_I("SETUP brush");
	GS()->brushColor = ci::ColorA(1.0, 0.0, 0.0, 1.0);

	BrushManagerSingleton::Instance()->setup();



	CI_LOG_I("SETUP composition with FBO");
	setupComposition(mActiveComposition);


	mNetworkHelper = new NetworkHelper();
	mNetworkHelper->setup();

	mNetworkHelper->onReceivePoints.connect([=](std::vector<ci::vec3>& points, bool isEraserOn){
		BrushManagerSingleton::Instance()->isEraserOn = false;

		for (auto&p : points){
			p.x *= mActiveComposition->mSize.x;
			p.y *= mActiveComposition->mSize.y;
		}

		GS()->brushColor = ci::ColorA(1.0, 0.0, 0.0, 1.0);
		mActiveComposition->drawInFbo(points);

		//pointsQueue.push(points);
	});


	mActiveComposition->newLine(vec3(10, 10, 40));
	mActiveComposition->lineTo(vec3(size.x * nrOfScreens -20, size.y -20, 10));
	mActiveComposition->endLine();

	mActiveComposition->newLine(vec3(size.x * nrOfScreens - 20, 10, 10));
	mActiveComposition->lineTo(vec3(10, size.y, 40));
	mActiveComposition->endLine();


	for (int i = 0; i < nrOfScreens-1; i++){

		app::WindowRef newWindow2 = createWindow(Window::Format().size(size * scale).pos(size.x * scale * (i+1), 120));
		newWindow2->setUserData(new WindowData(ci::Rectf(offset * (i+1), 1, offset * (i+2), 0), 1));
		newWindow2->setTitle("Window " + toString(i+2));
	}




}

void LineProjector2App::mouseDown( MouseEvent event )
{
}

void LineProjector2App::update()
{
	mNetworkHelper->update();
}



void LineProjector2App::setupComposition(std::shared_ptr<Composition>& composition, bool hasHistory){

	composition = make_shared<Composition>();
	//composition->setup(GS()->compositionSize);
	composition->setup(vec2(1280 *4.0,720));

	// when the new points with correct spacing are calculated we send them to the other
	// clients we don't send rawpoints.
	composition->onNewPoints.connect([=](pointVec p){
		//mNetworkHelper.sendPoints(p, BrushManagerSingleton::Instance()->isEraserOn);
	});
}


void LineProjector2App::draw()
{
	gl::clear(ColorA(249.0f / 255.0f, 242.0f / 255.0f, 160.0f / 255.0f, 1.0f));


	WindowData *data = getWindow()->getUserData<WindowData>();
	mActiveComposition->draw(data->mDrawingArea);
}

CINDER_APP( LineProjector2App, RendererGl )
