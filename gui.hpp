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

    virtual void update() {};
    //virtual ~MenuItem() = default; 
};

class Button: public MenuItem {
public:
    bool down = false;

    Texture* texture = nullptr;
    const char* text = nullptr;
    Color bg_color = DARKGRAY;
    Color text_color = RAYWHITE;

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

struct TextBox: public MenuItem {
    Color bg_color = DARKGRAY;
    Color text_color = RAYWHITE;
    std::string text;

    void update() {

    }
};

struct Menu {
    bool hovered = false;
    std::vector<MenuItem> items;
    Rectangle boundary;

    void layout_vertical(float padding = 1.f) {
        float offset = 0.f;
        Rectangle first_slot = { boundary.x, boundary.y, boundary.width, boundary.height / items.size() };
        for (MenuItem& item: items) {
            item.boundary = first_slot;
            item.boundary.y += offset;
            offset += item.boundary.height + padding;
        } 
    }
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
