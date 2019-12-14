//
// Created by romaonishuk on 27.10.19.
//

#include "x_window_impl.hpp"
#include "logger.hpp"

namespace Gui
{
XWindowImpl::XWindowImpl()
{
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        throw std::runtime_error("XWindowImpl display initialisation failed!");
    }

    // TODO(rmn): initial parameters!
    CreateWindow(0, 0, 1028, 480);
    CreateGraphicContext();

//    SetForeground(0xA9A9A9);

    Logger::Get().Log("XWindowImpl created!");
}

XWindowImpl::~XWindowImpl()
{
    XCloseDisplay(m_display);

    Logger::Get().Log("Window closed!");
}

void XWindowImpl::CreateWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
    auto screen = DefaultScreen(m_display);
    auto visual = DefaultVisual(m_display, screen);
    auto depth = DefaultDepth(m_display, screen);
    XSetWindowAttributes attributes;
    attributes.background_pixel = 0xC0C0C0; //XWhitePixel(m_display, screen);

    // TODO(rmn): check errors
    m_window = XCreateWindow(m_display, XRootWindow(m_display, screen), x, y, width, height, 5, depth,
                             InputOutput, visual, CWBackPixel, &attributes);

    XSelectInput(m_display, m_window, ExposureMask | ButtonPress | ButtonRelease | MotionNotify | FocusIn | FocusOut | Expose | GraphicsExpose);

    /*
    TODO(rmn): mb this should be called after all glyphs are drawn?!
    When windows are first created, they are not visible (not mapped) on the screen. 
    Any output to a window that is not visible on the screen and that does not have backing store will be discarded. 
    An application may wish to create a window long before it is mapped to the screen. 
    When a window is eventually mapped to the screen (using XMapWindow()), the X server generates an 
    Expose event for the window if backing store has not been maintained.
    */
    /* make the window actually appear on the screen. */
    XMapWindow(m_display, m_window);

    /* flush all pending requests to the X server. */
    XFlush(m_display);
}

void XWindowImpl::CreateGraphicContext()
{
    unsigned int line_width = 2;    /* line width for the GC.       */
    int line_style = LineSolid;     /* style for lines drawing and  */
    int cap_style = CapButt;        /* style of the line's edje and */
    int join_style = JoinBevel;     /*  joined lines.		*/

    m_gc = XCreateGC(m_display, m_window, 0, nullptr);
    if (m_gc < 0) {
        throw std::string("XCreateGC filed!");
    }

    /* define the style of lines that will be drawn using this GC. */
    XSetLineAttributes(m_display, m_gc, line_width, line_style, cap_style, join_style);

    /* define the fill style for the GC. to be 'solid filling'. */
    XSetFillStyle(m_display, m_gc, FillSolid);
}

void XWindowImpl::DrawRectangle(const Point& point, const width_t width, const height_t height)
{
    XDrawRectangle(m_display, m_window, m_gc, point.x, point.y, width, height);
    //XFlush (m_display);
}

void XWindowImpl::DrawText(const Point& text_position, std::string text)
{
    XDrawString(m_display, m_window, m_gc, text_position.x, text_position.y, text.c_str(), text.length());
}

void XWindowImpl::DrawLine(const Point& start_point, const Point& end_point)
{
    XDrawLine(m_display, m_window, m_gc, start_point.x, start_point.y, end_point.x, end_point.y);
}

void XWindowImpl::SetForeground(const int color)
{
    XSetForeground(m_display, m_gc, color);
}

void XWindowImpl::FillRectangle(const Point& point, const width_t width, const height_t height, const Color color)
{
    SetForeground(color);
    XFillRectangle(m_display, m_window, m_gc, point.x, point.y, width, height);
}

int XWindowImpl::GetEvent()
{
    XEvent event;
    XNextEvent(m_display, &event);

    return event.type;

//    return std::make_unique<
}

} // namespace Gui::Window