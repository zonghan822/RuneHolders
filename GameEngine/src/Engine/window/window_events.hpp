/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         window_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   40%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   40%
* @co-author    Caven Koh (caven.koh@digipen.edu)   20%
* @date         03-10-2025
* @brief        Event structures for window and input events
*//*========================================================================================*/



#pragma once

namespace Window {

    /*============================================================================================
    * \struct   KeyEvent
    * \brief    Represents a keyboard input event
    *//*========================================================================================*/
    struct KeyEvent {
        int key;        ///< GLFW key code
        int scancode;   ///< System-specific scancode
        int action;     ///< GLFW action (PRESS, RELEASE, REPEAT)
        int mods;       ///< Modifier keys state
    };

    /*============================================================================================
    * \struct   MouseButtonEvent
    * \brief    Represents a mouse button input event
    *//*========================================================================================*/
    struct MouseButtonEvent {
        int button;     ///< GLFW mouse button code
        int action;     ///< GLFW action (PRESS, RELEASE)
        int mods;       ///< Modifier keys state
    };

    /*============================================================================================
    * \struct   MouseCursorEvent
    * \brief    Represents mouse cursor movement event
    *//*========================================================================================*/
    struct MouseCursorEvent {
        double xpos;    ///< Cursor x-coordinate
        double ypos;    ///< Cursor y-coordinate
    };

    /*============================================================================================
    * \struct   ScrollEvent
    * \brief    Represents mouse scroll wheel event
    *//*========================================================================================*/
    struct ScrollEvent {
        double xoffset; ///< Horizontal scroll offset
        double yoffset; ///< Vertical scroll offset
    };

    /*============================================================================================
    * \struct   FramebufferResizeEvent
    * \brief    Represents framebuffer resize event
    *//*========================================================================================*/
    struct FramebufferResizeEvent {
        int width;      ///< New framebuffer width
        int height;     ///< New framebuffer height
    };

    /*============================================================================================
    * \struct   WindowFocusEvent
    * \brief    Represents window focus change event (ALT-TAB, etc.)
    *//*========================================================================================*/
    struct WindowFocusEvent {
        bool focused;   ///< true if window gained focus, false if lost
    };

    /*============================================================================================
    * \struct   WindowIconifyEvent
    * \brief    Represents window minimize/restore event
    *//*========================================================================================*/
    struct WindowIconifyEvent {
        bool iconified; ///< true if window minimized, false if restored
    };

}