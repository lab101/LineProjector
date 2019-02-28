
#include "cinder/cinder.h"
#include "cinder/rect.h"

class WindowData {
public:

	ci::Rectf mDrawingArea;
	int mId;

	WindowData(ci::Rectf drawingArea, int id)
	{
		mDrawingArea = drawingArea;
		mId = id;
	
	}
};