//
// Created by romaonishuk on 24.02.20.
//

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <algorithm>
#include "event_manager.hpp"

#include "lexi_linux/inc/x_window_impl.hpp"
#include "window.hpp"

#include <iostream>

EventManager::EventManager(Gui::Window* w): m_mainWindow(w), m_currentWindow(m_mainWindow)
{}

bool EventManager::ChangeCurrentWindow(unsigned long window)
{
    if(window == m_currentWindow->m_window_impl->GetWindow()) {
        return true;
    }

    auto windowsIt = std::find_if(childWindows.begin(), childWindows.end(),
        [window](const auto* w) { return w->m_window_impl->GetWindow() == window; });
    if(windowsIt == childWindows.end()) {
        std::cout << "Didn't find appropriate window" << std::endl;
        return false;
    }

    m_currentWindow = *windowsIt;
    return true;
}

void EventManager::RunLoop()
{
    // TODO(rmn): redesign this
    auto* display = reinterpret_cast<::Display*>(m_mainWindow->m_window_impl->GetDisplay());
    while(!stopLoop) {
        XEvent event;
        XNextEvent(display, &event);

        Point p{static_cast<uint32_t>(event.xbutton.x), static_cast<uint32_t>(event.xbutton.y)};
        switch(event.type) {
            case Expose:
                if(ChangeCurrentWindow(event.xexpose.window)) {
                    m_currentWindow->Draw(m_currentWindow);
                }
                break;
            case MapNotify:
            case UnmapNotify:
                break;
            case DestroyNotify:
                std::cout << "Close received!" << std::endl;
                return;
            case ButtonPress:
                if(ChangeCurrentWindow(event.xbutton.window)) {
                    m_currentWindow->ProcessEvent(m_currentWindow, {p, EventType::ButtonPressed});
                }
                break;
            case ButtonRelease:
                if(m_currentWindow->m_window_impl->GetWindow() == event.xbutton.window) {
                    m_currentWindow->ProcessEvent(m_currentWindow, {p, EventType::ButtonReleased});
                }
                break;
            case ClientMessage:
                // TODO(rmn): learn about client messages!
                std::cout << "Close received!" << std::endl;
                return;
            case CreateNotify:
                break;
            case MotionNotify:
                m_currentWindow->ProcessEvent(m_currentWindow, {p, EventType::FocusedIn});
                break;
            case FocusIn:
                std::cout << "Focus in" << std::endl;
                break;
            case FocusOut:
                std::cout << "Focus out" << std::endl;
                break;
            case EnterNotify:
            case LeaveNotify:
                break;
            default:
                std::cout << "RMN unprocessed event:" << event.type << std::endl;
                break;
        }
    }
}

void EventManager::Stop()
{
    std::cout << "Event loop was stopped!";
    stopLoop = true;
}
