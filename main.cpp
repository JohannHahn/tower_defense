#include <iostream>
//#include "raylib/raylib/include/raylib.h"
#include "raylib.h"
#include "raymath.h"
#include <cassert>
#include <vector>
#include <string>
#include <cstring>
#include <time.h>
#include <inttypes.h>

typedef uint64_t u64;
typedef uint32_t u32;

// tools
enum Log_Level {
    NO_LOG, LOW, FULL, 
};
Log_Level global_log_lvl = FULL;
constexpr const u64 initial_width = 900;
constexpr const u64 initial_height = 600;

template <class T>
void log_var(T var, const char* name = nullptr, Log_Level log_lvl = FULL) {
    if (global_log_lvl == NO_LOG) return;
    if (log_lvl < global_log_lvl) return;

    if (name && strlen(name) > 0) {
        std::cout << name << ":\n";
    }

    std::cout << var << "\n";
}

struct Window {
    u64 width = initial_width;
    u64 height = initial_height;
    u64 fps = 0;
    const char* title = "Tower Defense";

    void open() {
        InitWindow(width, height, title);
    }

    void close() {
        CloseWindow();
    }

    void resize_if_needed() {
        if (!IsWindowResized()) return;
        this->width = GetScreenWidth();
        this->height = GetScreenHeight();
    }

    void set_fps(u64 fps) {
        if (fps == 0) return;
        this->fps = fps;
        SetTargetFPS(fps);
    }
};

struct Map {
    Texture ground_tex;
    u64 width = initial_width;
    u64 height = initial_height;
    float road_width = 10.f;
    std::vector<Vector2> waypoints;

    Map() {
        //Image img = GenImageChecked(width, height, width / 10, height / 10, BLACK, WHITE);
        Image img = GenImageColor(width, height, BROWN);
        ground_tex = LoadTextureFromImage(img);
        UnloadImage(img);

        waypoints.reserve(100);
    }

    void draw(const Window& window) {
        // draw ground
        Rectangle source = {.x = 0, .y = 0, .width = (float)ground_tex.width, .height = (float)ground_tex.height};
        Rectangle dest = {.x = 0, .y = 0, .width = (float)window.width, .height = (float)window.height};
        DrawTexturePro(ground_tex, source, dest, {0.f, 0.f}, 0.f, WHITE);

        // draw waypoints
        for (int i = 0; i < waypoints.size(); ++i) {
            Vector2 current = waypoints[i]; 
            DrawCircleV(current, 1, RED);
            if (i == waypoints.size() - 1) continue;

            Vector2 next = waypoints[i + 1]; 
            //DrawLineV(current, next, BLUE);

            Vector2 dir = Vector2Subtract(next, current);
            Vector2 dir_90 = {dir.y, -dir.x};
            dir_90 = Vector2Normalize(dir_90);
            dir_90 = Vector2Scale(dir_90, road_width);
            Vector2 road_current = Vector2Add(current, dir_90); 
            Vector2 road_next = Vector2Add(next, dir_90); 

            // draw road segment on one side
            DrawLineV(road_current, road_next, GREEN);
            // the other side
            road_current = Vector2Add(current, Vector2Scale(dir_90, -1.f)); 
            road_next = Vector2Add(next, Vector2Scale(dir_90, -1.f)); 
            DrawLineV(road_current, road_next, GREEN);
        }
    }

    void ground_tex_from_image(const Image& img) {
        UnloadTexture(ground_tex);
        ground_tex = LoadTextureFromImage(img);
    }
};

enum Enemy_Type {
    CHICKEN,
};

struct Enemy {
    float hp;
    float speed;
    float damage;
    float max_speed;
    Enemy_Type type;
};

enum Building_Type {
    HOLE,
};

struct Building {
    float hp;
    float damage;
    float range;
    Building_Type type;
};

struct Level {
    Map map;
    std::vector<Enemy> enemies;
    std::vector<Building> buildings;
    const char* name; 
    float time = 0.f;

    Level(const char* name):name(name) {
        enemies.reserve(100); 
        buildings.reserve(100); 
    }

    void start() {
        time = 0.f;
    }

    void draw(const Window& window) {
        // draw map
        map.draw(window);
        // draw enemies
        // draw buildings
    }

    std::string to_string(const char* prefix = "") {
        if (prefix == nullptr) prefix = "";
        std::string out;
        out += prefix; out += "Level "; out += name; out += "\n";
        out += prefix; out += "active enemies: "; out += std::to_string(enemies.size()); out += "\n";  
        out += prefix; out += "active buildings: "; out += std::to_string(buildings.size()); out += "\n";  
        return out;
    }
};

Level make_test_level(const Window& window, Image img) {
    Level level = Level("test");
    int point_count = 50;
    for (int i = 0; i < point_count; ++i) {
        level.map.waypoints.push_back({(float)i * window.width / (float)point_count, (float)i * window.height / (float)point_count});
        if (GetRandomValue(0, 2) == 0) {
            if (GetRandomValue(0, 1) == 1)
                level.map.waypoints[i].x += GetRandomValue(1, 10);
            else if (GetRandomValue(0, 1) == 1)
                level.map.waypoints[i].y += GetRandomValue(1, 10);
        }
    }
    level.map.ground_tex_from_image(img);
    return level;
}

struct Game {
    std::vector<Level> levels;
    int active_level = -1;
    Building* selected_building = nullptr;

    Game() {
        levels.reserve(10);
    }

    Game(const std::vector<Level>& levels) {
        this->levels.reserve(levels.size());
        for (int i = 0; i < levels.size(); ++i) {
            this->levels.push_back(levels[i]);
        }
    }

    void start() {
        if (levels.size() == 0) {
            std::cerr << "No levels found!\n";
            return;
        }
        if (active_level < 0) active_level = 0;
        assert(active_level < levels.size());

        levels[active_level].start();
    }

    void select_level(u64 index) {
        assert(index < levels.size());
        active_level = index;
    }

    void draw(const Window& window) {
        assert(active_level < levels.size());

        if (active_level >= 0) {
            levels[active_level].draw(window);
        } 
        // draw menu
        else {

        }
    }

    std::string to_string() {
        std::string out = "Game: \n";
        out += "level count = "; out += std::to_string(levels.size()); out += "\n";
        for (int i = 0; i < levels.size(); ++i) {
            out += "level "; out += std::to_string(i); out += ": \n";
            out += levels[i].to_string("\t"); out += "\n";
        }
        return out;
    }
};


int main() {
    const char* img_path = "perlin_noise.bmp";
    Image img = LoadImage(img_path);
    // Raylib window
    Window window;
    window.set_fps(100);
    window.open();
    SetRandomSeed(time(NULL));

    Game game;
    game.levels.push_back(make_test_level(window, img));
    game.start();

    while (!WindowShouldClose()) {
        window.resize_if_needed();

        BeginDrawing();
        ClearBackground(BLACK);
         
        game.draw(window);

        DrawFPS(0, 0);

        EndDrawing();
    }

    window.close();

    return 0;
}
