/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2020  WerWolv

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <functional>
#include <string>

#ifdef __SDL2__
// Button constants matching GLFW layout for borealis compatibility
// These must match the values in application.hpp's BRLS_GAMEPAD_BUTTON_* defines
enum {
    _BRLS_KEY_A             = 0,
    _BRLS_KEY_B             = 1,
    _BRLS_KEY_X             = 2,
    _BRLS_KEY_Y             = 3,
    _BRLS_KEY_LEFT_BUMPER   = 4,
    _BRLS_KEY_RIGHT_BUMPER  = 5,
    _BRLS_KEY_BACK          = 6,
    _BRLS_KEY_START         = 7,
    _BRLS_KEY_GUIDE         = 8,
    _BRLS_KEY_LEFT_THUMB    = 9,
    _BRLS_KEY_RIGHT_THUMB   = 10,
    _BRLS_KEY_DPAD_UP       = 11,
    _BRLS_KEY_DPAD_RIGHT    = 12,
    _BRLS_KEY_DPAD_DOWN     = 13,
    _BRLS_KEY_DPAD_LEFT     = 14,
};
#else
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

namespace brls
{

class View;

typedef std::function<bool(void)> ActionListener;

// ZL and ZR do not exist here because GLFW doesn't know them
enum class Key
{
#ifdef __SDL2__
    A      = _BRLS_KEY_A,
    B      = _BRLS_KEY_B,
    X      = _BRLS_KEY_X,
    Y      = _BRLS_KEY_Y,
    LSTICK = _BRLS_KEY_LEFT_THUMB,
    RSTICK = _BRLS_KEY_RIGHT_THUMB,
    L      = _BRLS_KEY_LEFT_BUMPER,
    R      = _BRLS_KEY_RIGHT_BUMPER,
    PLUS   = _BRLS_KEY_START,
    MINUS  = _BRLS_KEY_BACK,
    DLEFT  = _BRLS_KEY_DPAD_LEFT,
    DUP    = _BRLS_KEY_DPAD_UP,
    DRIGHT = _BRLS_KEY_DPAD_RIGHT,
    DDOWN  = _BRLS_KEY_DPAD_DOWN,
#else
    A      = GLFW_GAMEPAD_BUTTON_A,
    B      = GLFW_GAMEPAD_BUTTON_B,
    X      = GLFW_GAMEPAD_BUTTON_X,
    Y      = GLFW_GAMEPAD_BUTTON_Y,
    LSTICK = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
    RSTICK = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,
    L      = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
    R      = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
    PLUS   = GLFW_GAMEPAD_BUTTON_START,
    MINUS  = GLFW_GAMEPAD_BUTTON_BACK,
    DLEFT  = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
    DUP    = GLFW_GAMEPAD_BUTTON_DPAD_UP,
    DRIGHT = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
    DDOWN  = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
#endif
};

struct Action
{
    Key key;

    std::string hintText;
    bool available;
    bool hidden;
    ActionListener actionListener;

    bool operator==(const Key other)
    {
        return this->key == other;
    }
};

} // namespace brls
