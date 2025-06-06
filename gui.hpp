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
    Color bg_color = DARKGRAY;
    Color text_color = RAYWHITE;
    Texture* texture = nullptr;
    const char* text = nullptr;

    void (*on_click)() = nullptr;
    void (*on_release)() = nullptr;
};

struct TextBox {
    bool active = true;
    bool hovered = false;
    Color bg_color = DARKGRAY;
    Color text_color = RAYWHITE;
    Rectangle boundary;
    std::string text;
};

struct Menu {
    bool hovered = false;
    Rectangle boundary;
    std::vector<Button> buttons;
    std::vector<TextBox> text_boxes;

    void layout_vertical(float padding = 1.f) {
        float offset = 0.f;
        Rectangle first_slot = { boundary.x, boundary.y, boundary.width, boundary.height / buttons.size() };
        for (Button& button : buttons) {
            button.boundary = first_slot;
            button.boundary.y += offset;
            offset += button.boundary.height + padding;
        } 
    }
};

struct GuiController {
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
    GuiController controller;
    std::vector<Menu> menues;

    void update() {
        for (Menu& menu : menues) {
            for (Button& button : menu.buttons) {
                update_button(button);
            }

            for (TextBox& text_box: menu.text_boxes) {
                update_textbox(text_box);
            }

        }
    }

    void update_button(Button& button) {
        controller.check_release(button);
        controller.check_hover(button);
        controller.check_click(button);
    }

    void update_textbox(TextBox& textbox) {

    }
};


} // namespace GUI
