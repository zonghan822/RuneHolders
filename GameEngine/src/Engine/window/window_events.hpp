/*============================================================================================
* @file         window_events.hpp
* @author       Law Zong Han (zonghan.law@digipen.edu)
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
}