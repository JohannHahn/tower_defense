#include <iostream>
//#include "raylib/raylib/include/raylib.h"
#include "raylib.h"
#include <cassert>
#include <vector>

struct Window {
    int width = 900;
    int height = 900;
    const char* title = "Tower Defense";

    void init() {
        InitWindow(width, height, title);
    }

    void close() {
        CloseWindow();
    }
};

struct Map {
    // ground texture
    // road / waypoints
    // buildings_mask
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
};

int main() {
    Window window;

    window.init();

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLUE);

        EndDrawing();
    }

    window.close();

    return 0;
}
