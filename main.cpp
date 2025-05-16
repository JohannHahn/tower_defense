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

template <class T>
void remove_inactive_elements(std::vector<T>& array) {
    for (int i = 0; i < array.size(); ++i) {
        if (array.at(i).active == false) {
            array.at(i) = array.at(array.size() - 1); 
            array.pop_back();
            i--;
        }
    }
}

Vector2 get_rec_center(Rectangle rec) {
    return {rec.x + rec.width / 2.f, rec.y + rec.height / 2.f};
}

Rectangle to_rec(const Vector2& v1, const Vector2& v2) {
    return {v1.x, v1.y, v2.x, v2.y};
}

// tools
enum Log_Level {
    NO_LOG, LOW, FULL, 
};
Log_Level global_log_lvl = FULL;

bool global_draw_debug = true;

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
        //if (fps == 0) return;
        this->fps = fps;
        SetTargetFPS(fps);
    }

    Rectangle get_game_boundary() const {
        return {0.f, 0.f, (float)width, (float)height};
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

struct Toggleable {
    bool active;
};


struct Enemy {
    bool active = true;
    float hp = 100.f;
    float speed = 100.f;
    float max_speed = 100.f;
    float damage = 1.f;
    Enemy_Type type = CHICKEN;
    Rectangle boundary = {0.f, 0.f, 10.f, 10.f};
    Vector2 direction = {1.f, 0.f};
    u64 next_waypoint = 0;

    void set_position(Vector2 pos) {
        boundary.x = pos.x;
        boundary.y = pos.y;
    }

    Vector2 get_position() const {
        return {boundary.x, boundary.y};
    }

    Vector2 get_center() const {
        return {boundary.x + boundary.width / 2.f, boundary.y + boundary.height / 2.f};
    }

    void update(const std::vector<Vector2>& waypoints) {
        if (active == false) return;

        Vector2 wp = waypoints[next_waypoint]; 
        Vector2 pos = get_position();
        float distance = Vector2Length(Vector2Subtract(wp, pos));

        if (distance > 100.f) {
            find_nearest_waypoint(waypoints);
        }

        direction = Vector2Scale(Vector2Normalize(Vector2Subtract(wp, pos)), speed * GetFrameTime());
        pos = Vector2Add(pos, direction);
        boundary.x = pos.x;
        boundary.y = pos.y;

        check_waypoint(wp, waypoints.size());
    }

    void check_waypoint(const Vector2 wp, u64 waypoints_size) {
        Vector2 pos = {boundary.x, boundary.y};
        if (Vector2Length(Vector2Subtract(pos, wp)) < 1.f) {
            next_waypoint++;
        }
        if (next_waypoint >= waypoints_size) {
            active = false;
            return;
        }
    }

    void find_nearest_waypoint(const std::vector<Vector2>& waypoints) {
        float min_distance = 99999999.f; 
        u64 nearest = 0;
        int i = 0;
        for (const Vector2& wp : waypoints) {
            float wp_dist = Vector2Distance(get_center(), wp);
            if (wp_dist < min_distance) {
                nearest = i;
                min_distance = wp_dist;
            }
            i++;
        }
        next_waypoint = nearest;
    }
};


struct Projectile {
    bool active = true;
    float speed = 70.f; 
    float radius = 1.f;
    Vector2 position;
    // TODO seeking
    //Vector2 target;
    Vector2 direction;

    void update(const std::vector<Enemy>& enemies, Rectangle game_boundary) {
        if (active == false) return;
        Vector2 dir = Vector2Scale(direction, speed / GetFrameTime());
        position = Vector2Add(position, dir);
        for (const Enemy& enemy : enemies) {
            if (CheckCollisionCircleRec(position, radius, enemy.boundary)) {
                active = false;
                // TODO:: enemy.hit();
                return;
            }
            else if (!CheckCollisionCircleRec(position, radius, game_boundary)) {
                active = false;
                return;
            }
        }
    }
};

enum Tower_Type {
    BASIC,
};

struct Tower {
    float hp = 100.f;
    float damage = 10.f;
    float range = 100.f;
    float reload_time = 50.f;
    float time_since_shot = reload_time;
    std::vector<Projectile> active_bullets;
    Tower_Type type = BASIC;

    Vector2 position = {0.f, 0.f};
    Vector2 size = {10.f, 10.f};
    Vector2 direction = {10.f, 10.f};
    u64 target_index;
    bool target_lock = false;

    void update(const std::vector<Enemy> enemies) {
        // find target
        if (target_lock == false) {
            u64 i = 0;
            for (const Enemy& enemy : enemies) {
                if (enemy.active == false) continue;
                Vector2 line_to_target = Vector2Subtract(enemy.get_center(), position);
                if (Vector2Length(line_to_target) <= range) {
                    target_index = i;
                    target_lock = true;
                    break;
                }
                i++;
            }
        }

        if (target_lock) {
            if (target_index >= enemies.size() ) {
                target_lock = false;

                return;
            }
            Vector2 line_to_target = Vector2Subtract(enemies[target_index].get_center(), get_center());
            direction = Vector2Normalize(line_to_target);
            if (Vector2Length(line_to_target) > range) target_lock = false;
        }
    }

    Vector2 get_center() const {
        return {position.x + size.x / 2.f, position.y + size.y / 2.f};
    }
};

struct EnemySpawner {
    bool active = true;
    Enemy_Type type = CHICKEN;
    Vector2 position = {0.f, 0.f};
    float delay = 1.f;
    float time_since_spawn = delay;
    u64 max = 0;
    u64 spawned = 0;

    void spawn() {
        if (active == false) return;
        spawned++;
        if (max > 0 && spawned >= max) {
            active = false;
        }
    }
};

struct Level {
    Map map;
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    std::vector<EnemySpawner> spawners;
    std::vector<Projectile> bullets;
    const char* name; 
    float time = 0.f;

    Level(const char* name):name(name) {
        enemies.reserve(100); 
        towers.reserve(100); 
    }

    void start() {
        time = 0.f;
    }

    void update(Rectangle game_boundary) {
        update_spawners();
        update_towers();
        update_bullets(game_boundary);
        update_enemies();
    }

    void update_enemies() {
        for (Enemy& enemy : enemies) {
            enemy.update(map.waypoints);
        }
        remove_inactive_elements(enemies);
    }

    void update_bullets(Rectangle game_boundary) {
        for (Projectile& bullet : bullets) {
            bullet.update(enemies, game_boundary);
        } 
    }

    void update_spawners() {
        for (EnemySpawner& spawner: spawners) {
            spawner.time_since_spawn += GetFrameTime();
            if (!spawner.active || spawner.time_since_spawn < spawner.delay) return;
            
            Enemy enemy;
            enemy.type = spawner.type;
            enemy.set_position(spawner.position);
            spawner.spawn();
            enemies.push_back(enemy);
            spawner.time_since_spawn = 0.f;
        }
    }

    void update_towers() {
        for (Tower& tower : towers) {
            tower.update(enemies);

            tower.time_since_shot += GetFrameTime();
            if (tower.time_since_shot < tower.reload_time) return;
            spawn_bullet(tower);
        }     
        for (Tower& tower: towers) {
            remove_inactive_elements(tower.active_bullets);
        }
    }

    void spawn_bullet(const Tower& tower) {
        if (tower.target_lock == false) return;
        Projectile bullet;
        bullet.position = tower.get_center();
        bullet.direction = tower.direction;
        bullets.push_back(bullet);
    }

    void draw(const Window& window) {
        // draw map
        map.draw(window);
        // draw enemies
        for (const Enemy& enemy : enemies) {
            draw_enemy(enemy);
        }
        // draw buildings
        for (const Tower& tower: towers) {
            draw_tower(tower);
        }
        
        for (const Projectile& bullet: bullets) {
            if (bullet.active == false) continue;
            DrawCircleV(bullet.position, 1.f, YELLOW);
        }
        DrawText(TextFormat("enemies.size = %d", enemies.size()), window.width / 2.f, 0, 20, WHITE);
        DrawText(TextFormat("bullets.size = %d", bullets.size()), window.width / 1.3f, 0, 20, WHITE);
    }

    void draw_enemy(const Enemy& enemy, bool draw_debug = false) {
        if (enemy.active == false) return;
        // draw boundary
        if (draw_debug || global_draw_debug) {
            DrawRectangleRec(enemy.boundary, RED);

            if (enemy.next_waypoint >= map.waypoints.size()) {
            }
                
            assert(enemy.next_waypoint < map.waypoints.size());
            Vector2 pos = enemy.get_center();
            Vector2 direction = enemy.direction;
            direction = Vector2Scale(direction, 1.f / GetFrameTime());
            DrawLineV(pos, Vector2Add(pos, direction), GREEN);
        }
        // draw "model"
    }

    void draw_tower(const Tower& tower, bool draw_debug = false) {
        if (draw_debug || global_draw_debug) {
            Color color = tower.target_lock ? GREEN : GRAY;
            Rectangle tower_rec = {tower.position.x, tower.position.y, tower.size.x, tower.size.y};
            DrawRectangleRec(tower_rec, color);
            DrawLineV(tower.get_center(), Vector2Add(tower.get_center(), Vector2Scale(tower.direction, 40.f)), GREEN);
            DrawCircleLinesV(tower.get_center(), tower.range, RED);
            if (tower.target_lock) {
                DrawLineV(tower.get_center(), enemies[tower.target_index].get_center(), GRAY);
                // draw_target
                DrawRectangleRec(enemies[tower.target_index].boundary, GREEN);
            }
        }
    }

    std::string to_string(const char* prefix = "") {
        if (prefix == nullptr) prefix = "";
        std::string out;
        out += prefix; out += "Level "; out += name; out += "\n";
        out += prefix; out += "active enemies: "; out += std::to_string(enemies.size()); out += "\n";  
        out += prefix; out += "active buildings: "; out += std::to_string(towers.size()); out += "\n";  
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
                level.map.waypoints[i].x += GetRandomValue(1, 20);
            else if (GetRandomValue(0, 1) == 1)
                level.map.waypoints[i].y += GetRandomValue(1, 20);
        }
    }
    level.map.ground_tex_from_image(img);

    EnemySpawner sp;
    sp.position = {window.width / 2.f, 0};
    sp.max = 20;
    level.spawners.push_back(sp);

    Tower tower; tower.position = {window.width / 2.f, window.height / 1.5f};
    level.towers.push_back(tower);
    return level;
}

struct Game {
    std::vector<Level> levels;
    int active_level = -1;
    Tower* selected_building = nullptr;

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

    void update(Rectangle game_boundary) {
        assert(active_level < levels.size());
        levels[active_level].update(game_boundary);
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
        game.update(window.get_game_boundary());

        BeginDrawing();
        ClearBackground(BLACK);
         
        game.draw(window);

        DrawFPS(0, 0);

        EndDrawing();
    }

    window.close();

    return 0;
}
