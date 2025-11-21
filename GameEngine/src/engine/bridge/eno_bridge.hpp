#pragma once

// Input
#include "../input/eno_input.hpp"


namespace Bridge
{
    // Instead of a raw pointer, expose a setter/getter
    void setBridge(class EnoBridge* bridge);
    class EnoBridge* getBridge();

    class EnoBridge
    {
    public:
        EnoBridge(Input::EnoInputManager& input)
            : input_{ input }
        {
            setBridge(this);
        }

        Input::EnoInputManager& input() const { return input_; }

    private:
        Input::EnoInputManager& input_;
    };
}

