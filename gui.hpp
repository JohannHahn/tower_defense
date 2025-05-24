#pragma once

#include <iostream>
#include <iterator>
#include <vector>
#include "raylib.h"

namespace GUI {

struct Button {
    bool active = true;
    bool hovered = false;
    bool down = false;

    Rectangle boundary; 
    Texture* texture = nullptr;
    const char* text = nullptr;

    void (*on_click)() = nullptr;
    void (*on_release)() = nullptr;
};

struct Menu {
    bool hovered = false;
    std::vector<Button> buttons;
};

struct Controller {
    void check_click(Button& button) {
        if (button.active == false ) return;

        if (!button.down 
            && CheckCollisionPointRec(GetMousePosition(), button.boundary) 
            && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

            button.down = true;
            //button.hovered = false;
            if (button.on_click)
                button.on_click();
        }
    }

    void check_release(Button& button) {
        if (button.active == false) return;

        if (button.down && IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
            button.down = false;
            if (button.on_release) button.on_release();
        }
    }

    void check_hover(Button& button) {
        if (button.active == false) return;

        button.hovered = CheckCollisionPointRec(GetMousePosition(), button.boundary);
    }
};

struct Gui {
    Controller controller;
    std::vector<Menu> menues;

    void update() {
        for (Menu& menu : menues) {
            for (Button& button : menu.buttons) {
                controller.check_release(button);
                controller.check_hover(button);
                controller.check_click(button);
            }
        }
    }
};


} // namespace GUI
