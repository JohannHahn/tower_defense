#include "raylib.h"
#include <iostream>
#include <cstring>
#include <time.h>
#include "game.hpp"
#include "draw.hpp"
#include "gui.hpp"

typedef uint64_t u64;
typedef uint32_t u32;


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
    tower.type = TOWER_BASIC;
    tower.turn_speed = .1f;
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
    event.position = {window.width / 2.f, window.height / 2.f};
    event.start = 0.f;
    event.delay = .5f;
    
    for (int i = 0; i < ENEMY_TYPE_MAX; i++) {
        event.enemies[i] = 100;
    }
    round.events.push_back(event);
    //event.start = 2;
    //round.events.push_back(event);
    //event.start = 20;
    //round.events.push_back(event);
    level.rounds.push_back(round);

    return level;
}

constexpr const u64 initial_width = 1200;
constexpr const u64 initial_height = 900;
Game game;
Window window(initial_width, initial_height);

// TODO
//
struct Controller {
    Game game;
    Gui gui;
};

void start_callback() {
    game.start();
};

void exit_callback() {
    game.quit = true;
};

enum MenuIndex {
    MENU_MAIN, MENU_MAX, 
};

Menu make_main_menu(const Window& window) {

    Menu menu;
    float width = window.width / 10.f;
    float height = window.height / 3.f;
    menu.boundary = {window.width - width, 0.f, width, height};

    Button start_button; 
    start_button.text = "Start";
    start_button.on_click = start_callback;

    Button exit_button;
    exit_button.text = "Exit";
    exit_button.on_click = exit_callback;

    menu.buttons.push_back(start_button);
    menu.buttons.push_back(exit_button);
    menu.layout_vertical();
    return menu;
}

Gui make_gui(const Window& window) {
    Gui gui;
    Menu main_menu = make_main_menu(window);
    gui.menues.resize(MENU_MAX);
    gui.menues[MENU_MAIN] = main_menu;
    return gui;
}


int main() {
    Log_Level global_log_lvl = FULL;


    const char* img_path = "perlin_noise.bmp";
    Image img = LoadImage(img_path);
    // Raylib window
    window.set_fps(100);
    window.open();
    SetRandomSeed(time(NULL));

    Level test_lvl = make_test_level(window, img);
    game.levels.push_back(test_lvl);
    //game.start();
    //game.get_current_level().load_from_file("level.blob");

    Gui gui = make_gui(window);

    while (!WindowShouldClose()) {
        window.resize_if_needed();

        GameController::update(game);

        if (!(game.active_level == -1) || !game.paused) {
            game.update();
        }
        gui.update();

        if (game.quit) break; 

        BeginDrawing();
        ClearBackground(BLACK);
         
        window.draw(game, gui);

        DrawFPS(0, initial_height / 2.f);

        EndDrawing();
    }

    window.close();

    return 0;
}
