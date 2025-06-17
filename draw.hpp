#pragma once

#include "game.hpp"
#include "gui.hpp"
#include "raylib.h"

using namespace GUI;

float center_text(Font font, const char* text, Rectangle boundary, Vector2* position = nullptr) {
    float font_size = boundary.height ;
    Vector2 size = MeasureTextEx(font, text, font_size, 2.f);
    font_size *= (boundary.width / size.x);
    if (position) {
        *position = {boundary.x, boundary.y + (boundary.height - font_size) / 2.f};
    }
    return font_size;
}

Rectangle squish_rec(Rectangle rec, float dist) {
    rec.x += dist;
    rec.y += dist;
    rec.width  -= 2.f*dist;
    rec.height -= 2.f*dist;
    return rec;
}


struct Renderer {
    Rectangle bounds;
    bool draw_debug = true;

    void draw_map(const Map& map); 
    void draw_level(const Level& level);
    void draw_enemy(const Enemy& enemy, const Map& map);
    void draw_tower(const Tower& tower, const std::vector<EnemyRecord>& enemy_records);
    void draw_bullet(const Projectile& bullet);
    void draw_game(const Game& game);

    void draw_gui(const Game& game, const Gui& gui) {
        // not started game yet
        if (game.active_level == -1) {
            // TODO:: enum ins commons
            draw_menu(gui.menues[0]); 
        }
        else if (game.edit_mode) {
            // draw edit menu
        }
        if (game.paused){
            // draw pause menu
        }

    }

    void draw_menu(const Menu& menu) {
        for (const MenuItem& item : menu.items) {

            assert(false);
            //draw_button(item);
        }
    }

    void draw_button(const Button& button) {
        Color col = button.bg_color;
        float thicc = 1.f;
        Rectangle inside_rec = button.boundary;
        if (button.down) {
            thicc = 3.f;
            inside_rec = squish_rec(inside_rec, 3.f);
        }
        if (button.hovered) col = ColorBrightness(col, -0.5f);
        DrawRectangleRec(inside_rec, col);
        if (button.texture) {
            //DrawTextureRec(*button.texture, button.boundary, {0.f, 0.f}, WHITE);
        }
        if (button.text) {
            Rectangle rec = squish_rec(button.boundary, button.down ? 13.f : 10.f);
            Vector2 position;
            float font_size = center_text(GetFontDefault(), button.text, rec, &position);
            
            //DrawText(button.text, button.boundary.x, button.boundary.y, (font_size.y + font_size.x) / 2.f, BLACK);
            Color text_color = button.text_color;
            if (button.hovered) text_color = ColorBrightness(text_color, -0.5f);
            DrawTextEx(GetFontDefault(), button.text, position, font_size, 2.f, button.text_color);

        }
        DrawRectangleLinesEx(button.boundary, thicc, BLACK);
    }

    void draw_textbox(const TextBox& textbox) {
        Color bg = WHITE;
        DrawRectangleRec(textbox.boundary, WHITE);
        DrawRectangleLinesEx(textbox.boundary, 1.f, BLACK);
        Font font = GetFontDefault();
        Vector2 position;
        float font_size = center_text(font, textbox.text.c_str(), squish_rec(textbox.boundary, 5.f), &position);
        DrawTextEx(font, textbox.text.c_str(), position, font_size, 1.f, BLACK);
    }

};

struct Window {
    u64 width;
    u64 height;
    u64 fps = 0;
    const char* title = "Tower Defense";
    Renderer renderer;

    Window(u64 width, u64 height);

    void open();

    void close();

    void resize_if_needed();

    void set_fps(u64 fps);

    void draw(const Game& game, const Gui& gui);

    Rectangle get_game_boundary() const;
};

Window::Window(u64 width, u64 height): height(height), width(width){
    renderer.bounds = {0.f, 0.f, (float)width, (float)height};
};

void Window::open() {
    InitWindow(width, height, title);
}

void Window::close() {
    CloseWindow();
}

void Window::resize_if_needed() {
    if (!IsWindowResized()) return;
    this->width = GetScreenWidth();
    this->height = GetScreenHeight();
}
void Window::draw(const Game& game, const Gui& gui) {
    renderer.draw_game(game);
    renderer.draw_gui(game, gui);
}

void Window::set_fps(u64 fps) {
    //if (fps == 0) return;
    this->fps = fps;
    SetTargetFPS(fps);
}

Rectangle Window::get_game_boundary() const {
    return {0.f, 0.f, (float)width, (float)height};
}
void Renderer::draw_map(const Map& map) {
    // draw ground
    Rectangle source = {.x = 0, .y = 0, .width = (float)map.ground_tex.width, .height = (float)map.ground_tex.height};
    Rectangle dest = {.x = 0, .y = 0, .width = bounds.width, .height = bounds.height};
    DrawTexturePro(map.ground_tex, source, dest, {0.f, 0.f}, 0.f, WHITE);

    // draw waypoints
    for (int i = 0; i < map.waypoints.size(); ++i) {
        Vector2 current = map.waypoints[i]; 
        DrawCircleV(current, 1, RED);
        if (i == map.waypoints.size() - 1) continue;

        Vector2 next = map.waypoints[i + 1]; 
        //DrawLineV(current, next, BLUE);

        Vector2 dir = Vector2Subtract(next, current);
        Vector2 dir_90 = {dir.y, -dir.x};
        dir_90 = Vector2Normalize(dir_90);
        dir_90 = Vector2Scale(dir_90, map.road_width);
        Vector2 road_current = Vector2Add(current, dir_90); 
        Vector2 road_next = Vector2Add(next, dir_90); 

        // draw road segment on one side
        DrawLineV(road_current, road_next, BROWN);
        // the other side
        road_current = Vector2Add(current, Vector2Scale(dir_90, -1.f)); 
        road_next = Vector2Add(next, Vector2Scale(dir_90, -1.f)); 
        DrawLineV(road_current, road_next, BROWN);
    }
}
void Renderer::draw_level(const Level& level) {
    // draw map
    draw_map(level.map);
    // draw enemies
    for (const Enemy& enemy : level.enemies) {
        draw_enemy(enemy, level.map);
    }
    // draw buildings
    for (const Tower& tower: level.towers) {
        draw_tower(tower, level.enemy_records);
    }
    
    for (const Projectile& bullet: level.bullets) {
        draw_bullet(bullet);
    }

    for (const EnemySpawner& spawner: level.spawners) {
        //if (spawner.active == false) continue;
        Color color = WHITE;
        if (spawner.active == false) color = GRAY;
        DrawCircleV(spawner.position, 5.f, color);
    }
    DrawText(TextFormat("enemies.size = %d", level.enemies.size()), bounds.width / 2.f, 0, 20, WHITE);
    DrawText(TextFormat("enemy_records.size = %d", level.enemy_records.size()), bounds.width / 2.f, 100, 20, WHITE);
    DrawText(TextFormat("bullets.size = %d", level.bullets.size()), bounds.width / 1.3f, 0, 20, WHITE);
    DrawText(TextFormat("spawners.size = %d", level.spawners.size()), bounds.width / 1.3f, 200, 20, WHITE);
    DrawText(TextFormat("Time: %f", level.time), 10, 10, 20, WHITE);

}

void Renderer::draw_bullet(const Projectile& bullet) {
    if (bullet.active == false) return;
    DrawCircleV(bullet.position, bullet.radius, YELLOW);
}

void Renderer::draw_enemy(const Enemy& enemy, const Map& map) {
    if (enemy.active == false) return;
    // draw boundary
    if (draw_debug) {
        Color color = RED;
        if (enemy.hit) color = MAGENTA;
        DrawRectangleRec(enemy.boundary, color);

        if (enemy.next_waypoint >= map.waypoints.size()) {
        }
            
        assert(enemy.next_waypoint < map.waypoints.size());
        Vector2 pos = enemy.get_center();
        Vector2 direction = enemy.direction;
        direction = Vector2Scale(direction, 1.f / GetFrameTime());
        //DrawLineV(pos, Vector2Add(pos, direction), GREEN);
    }
    // draw "model"
}

void Renderer::draw_tower(const Tower& tower, const std::vector<EnemyRecord>& enemy_records) {
    if (draw_debug) {
        Color color = tower.target_lock ? GREEN : GRAY;
        Rectangle tower_rec = {tower.position.x, tower.position.y, tower.size.x, tower.size.y};
        DrawRectangleRec(tower_rec, color);
        DrawLineV(tower.get_center(), Vector2Add(tower.get_center(), Vector2Scale(tower.direction, 2.f)), GREEN);
        DrawCircleLinesV(tower.get_center(), tower.range, RED);
        if (tower.target_lock) {
            DrawLineV(tower.get_center(), enemy_records[tower.target_id].center, RED);
        }
    }
}

void Renderer::draw_game(const Game& game) {
    if (game.edit_mode) {
        draw_level(game.edit_level);
        return;
    }

    assert(game.active_level < (int)game.levels.size());

    if (game.active_level >= 0) {
        draw_level(game.levels[game.active_level]);
    } 
    // draw menu
    else {

    }
}

