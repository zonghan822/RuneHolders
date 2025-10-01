#pragma once

namespace Window {
    struct KeyEvent {
        int key;
        int scancode;
        int action;
        int mods;
    };

    struct MouseButtonEvent {
        int button;
        int action;
        int mods;
    };

    struct MouseCursorEvent {
        double xpos;
        double ypos;
    };

    struct ScrollEvent {
        double xoffset;
        double yoffset;
    };

    struct FramebufferResizeEvent {
        int width;
        int height;
    };
}