#include "raylib.h"
#include <iostream>
#include <cstring>
#include <time.h>
#define GAME_IMPLEMENTATION
#include "game.hpp"
#define DRAW_IMPLEMENTATION
#include "draw.hpp"

typedef uint64_t u64;
typedef uint32_t u32;

// tools
enum Log_Level {
    NO_LOG, LOW, FULL, 
};

//template <class T>
//void log_var(T var, const char* name = nullptr, Log_Level log_lvl = FULL) {
//    if (global_log_lvl == NO_LOG) return;
//    if (log_lvl < global_log_lvl) return;
//
//    if (name && strlen(name) > 0) {
//        std::cout << name << ":\n";
//    }
//    std::cout << var << "\n";
//}

Level make_test_level(const Window& window, Image img) {
    Level level = Level("test", window.get_game_boundary());
    int point_count = 50;
    for (int i = 0; i < point_count; ++i) {
        level.map.waypoints.push_back({(float)i * window.width / (float)point_count, (float)i * window.height / (float)point_count});
        if (GetRandomValue(0, 2) == 0) {
            if (GetRandomValue(0, 1) == 1)
                level.map.waypoints[i].x += GetRandomValue(1, 20);
            else if (GetRandomValue(0, 1) == 1)
                level.map.waypoints[i].y += GetRandomValue(1, 20);
        }
    }
    level.map.ground_tex_from_image(img);

    //EnemySpawner sp;
    //sp.position = {window.width / 2.f, 0};
    //sp.active = true;
    //level.spawners.push_back(sp);
    //EnemySpawner sp2;
    //sp2.active = true;
    //level.spawners.push_back(sp2);
    //sp.position = {100, 0};
    //level.spawners.push_back(sp);
    //sp.position = {100, 100};
    //level.spawners.push_back(sp);

    Tower tower; tower.position = {window.width / 2.f, window.height / 1.7f};
    level.add_tower(tower);
    tower.position = {window.width - 100.f, window.height / 1.1f};
    level.add_tower(tower);
    tower.position = {window.width / 2.f + 50, window.height / 1.6f};
    level.add_tower(tower);
    tower.position = {window.width - 50.f, window.height / 1.1f};
    level.add_tower(tower);
    tower.position = {window.width / 2.f + 100, window.height / 1.6f};
    level.add_tower(tower);

    Round round;
    round.length = 100; 
    SpawnEvent event;
    event.position = {0, 0};
    event.start = 0.5f;
    event.delay = 2.f;
    
    for (int i = 0; i < ENEMY_TYPE_MAX; i++) {
        event.enemies[i] = 10;
    }
    round.events.push_back(event);
    event.start = 5;
    round.events.push_back(event);
    event.start = 50;
    round.events.push_back(event);
    level.rounds.push_back(round);
    return level;
}

int main() {
    Log_Level global_log_lvl = FULL;

    constexpr const u64 initial_width = 900;
    constexpr const u64 initial_height = 600;

    const char* img_path = "perlin_noise.bmp";
    Image img = LoadImage(img_path);
    // Raylib window
    Window window(initial_width, initial_height);
    window.set_fps(100);
    window.open();
    SetRandomSeed(time(NULL));

    Game game;
    game.levels.push_back(make_test_level(window, img));
    game.start();

    while (!WindowShouldClose()) {
        window.resize_if_needed();
        if (!game.paused) {
            game.update(window.get_game_boundary());
        }

        BeginDrawing();
        ClearBackground(BLACK);
         
        window.draw(game);

        if (IsKeyPressed(KEY_SPACE)) {
            game.paused = !game.paused;
        }
        Vector2 position = {(float)GetMouseX(), (float)GetMouseY()};
        Tower tower;
        tower.position = position;
        Rectangle rec = {to_rec(tower.position, tower.size)};
        if (game.get_current_level().map.check_free(rec)) {
            DrawRectangleRec(rec, GREEN);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                game.get_current_level().add_tower(tower);
            }
                
        } else {
            DrawRectangleRec(rec, RED);
        }

        DrawFPS(0, initial_height / 2.f);

        EndDrawing();
    }

    window.close();

    return 0;
}
