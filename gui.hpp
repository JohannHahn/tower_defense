#pragma once

#include <iostream>
#include <iterator>
#include <vector>
#include "raylib.h"

namespace GUI {

class MenuItem {
public:
    bool active = true;
    bool hovered = false;
    Rectangle boundary;

    void (*on_click)() = nullptr;
    void (*on_release)() = nullptr;
    virtual void update();
};

class Button: public MenuItem {
public:
    bool down = false;

    Texture* texture = nullptr;
    const char* text = nullptr;

    void update() {
        if (active == false ) return;

        hovered = CheckCollisionPointRec(GetMousePosition(), boundary);

        if (!down && hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            down = true;
            if (on_click)
                on_click();
        }

        if (down && IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
            down = false;
            if (on_release) on_release();
        }
    }

};

struct Menu {
    bool hovered = false;
    std::vector<MenuItem> items;
};

struct Gui {
    std::vector<Menu> menues;

    void update() {
        for (Menu& menu : menues) {
            for (MenuItem& item: menu.items) {
                item.update();
            }
        }
    }
};


} // namespace GUI
