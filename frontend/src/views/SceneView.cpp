#include "frontend/views/SceneView.hpp"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"

#include <QWidget>
#include <QResizeEvent>
#include <QTimer>

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#endif

namespace frontend {

// ----------------------------------------------------------------------------
SceneView::SceneView(QScreen* screen) :
    QWindow(screen),
    resizeTimer_(nullptr)
{
    initGraphics();
    resizeTimer_ = new QTimer(this);
    connect(resizeTimer_, SIGNAL(timeout()), this, SLOT(onResizeTimerTimeout()));
}

// ----------------------------------------------------------------------------
SceneView::SceneView(QWindow* parent) :
    QWindow(parent)
{
    initGraphics();
}

// ----------------------------------------------------------------------------
SceneView::~SceneView()
{
    bgfx::shutdown();
#ifdef Q_OS_LINUX
    XCloseDisplay(reinterpret_cast<::Display*>(X11Display_));
#endif
}

// ----------------------------------------------------------------------------
void SceneView::initGraphics()
{
    setSurfaceType(QSurface::OpenGLSurface);

    bgfx::Init init;
    init.resolution.width = 1;  // we get resized anyway
    init.resolution.height = 1;
    init.resolution.reset = BGFX_RESET_VSYNC;
    init.platformData.nwh = reinterpret_cast<void*>(winId());
#ifdef Q_OS_LINUX
    X11Display_ = XOpenDisplay(NULL);
    init.platformData.ndt = reinterpret_cast<void*>(X11Display_);
#endif
    bgfx::init(init);

    // Set up screen clears
    bgfx::setViewClear(0
        , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
        , 0x007700ff
        , 1.0f
        , 0
        );
}

// ----------------------------------------------------------------------------
void SceneView::resizeEvent(QResizeEvent* e)
{
    resizeTimer_->start(40);
}

// ----------------------------------------------------------------------------
void SceneView::onResizeTimerTimeout()
{
    if (width() == 0 || height() == 0)
        return;
    bgfx::reset(width(), height(), BGFX_RESET_VSYNC);
    draw();
}

// ----------------------------------------------------------------------------
void SceneView::draw()
{
    bgfx::setViewRect(0, 0, 0, width(), height());
    bgfx::touch(0);
    bgfx::frame();
}

}
