#pragma once
#include <inttypes.h>
#include <iostream>
#include "raylib.h"
#include "raymath.h"
#include <cassert>
#include <vector>
#include <string>

typedef uint64_t u64;
typedef uint32_t u32;

template <class T>
static void remove_inactive_elements(std::vector<T>& array);

static Vector2 get_rec_center(Rectangle rec);

static Rectangle to_rec(const Vector2& v1, const Vector2& v2);


struct Map {
    Texture ground_tex;
    u64 width;
    u64 height;
    float road_width = 10.f;
    std::vector<Vector2> waypoints;
    std::vector<Rectangle> occupied_areas;

    Map(Rectangle bounds);

    void ground_tex_from_image(const Image& img);

    void add_rec(Rectangle rec);

    bool check_free(Rectangle rec) const ;
};

enum Enemy_Type {
    CHICKEN, ENEMY_TYPE_MAX
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

    bool hit = false;

    void set_position(Vector2 pos);

    Vector2 get_position() const;
    Vector2 get_center() const;

    void update(const std::vector<Vector2>& waypoints);

    void check_waypoint(const Vector2 wp, u64 waypoints_size);

    void find_nearest_waypoint(const std::vector<Vector2>& waypoints);

    void get_hit(float dmg);
};

enum Projectile_Type {
    STRAIGHT, SEEK
};

struct Projectile {
    bool active = true;
    float speed = 2000.f; 
    float radius = 2.f;
    float damage = 2.f;
    Vector2 position;
    Vector2 target;
    Vector2 direction;
    Projectile_Type type = STRAIGHT;

    void update(std::vector<Enemy>& enemies, Rectangle game_boundary);
};

enum Tower_Type {
    TOWER_BASIC, TOWER_SEEK  
};

struct Tower {
    float hp = 100.f;
    float damage = 10.f;
    float range = 100.f;
    float reload_time = 2.f;
    float time_since_shot = reload_time;
    Tower_Type type = TOWER_SEEK;
    std::vector<Projectile> active_bullets;

    Vector2 position = {0.f, 0.f};
    Vector2 size = {10.f, 10.f};
    Vector2 direction = {10.f, 10.f};
    u64 target_index;
    bool target_lock = false;

    void update(const std::vector<Enemy> enemies);

    bool shot_ready();

    void shoot();

    Vector2 get_center() const;

};

struct EnemySpawner {
    bool active = true;
    Enemy_Type type = CHICKEN;
    Vector2 position = {0.f, 0.f};
    float delay = 1.f;
    float time_since_spawn = 0.f;
    u64 max = 0;
    u64 spawned = 0;

    void spawn(std::vector<Enemy>& enemies);
    void update(std::vector<Enemy>& enemies);
};

struct SpawnEvent {
    float start;   
    float delay;
    u64 enemies[ENEMY_TYPE_MAX];
    Vector2 position;


    void spawn(std::vector<Enemy>& enemies, std::vector<EnemySpawner>& spawners);
    
};

struct Round {
    std::vector<EnemySpawner> spawners;
    std::vector<SpawnEvent> events; 
    float length;
    float time = 0.f;
    u64 next_event = 0;

    void update(std::vector<Enemy>& enemies, std::vector<EnemySpawner>& spawners);
};

struct Level {
    Map map;
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    std::vector<EnemySpawner> spawners;
    std::vector<Projectile> bullets;
    std::vector<Round> rounds;
    const char* name; 
    float time = 0.f;
    int active_round = -1;

    Level(const char* name, Rectangle bounds);

    void start();

    void update(Rectangle game_boundary);

    void update_enemies();

    void update_bullets(Rectangle game_boundary);

    void update_spawners();

    void update_towers();

    void update_round();

    void spawn_bullet(Tower& tower);

    void add_tower(Tower tower);

    std::string to_string(const char* prefix = "");

};
struct Game {
    std::vector<Level> levels;
    int active_level = -1;
    Tower* selected_building = nullptr;
    bool paused = true;


    Game();

    Game(const std::vector<Level>& levels);

    void start();

    void select_level(u64 index);

    void update(Rectangle game_boundary);

    Level& get_current_level();

    std::string to_string();
};
#define GAME_IMPLEMENTATION
#ifdef GAME_IMPLEMENTATION
template <class T>
static void remove_inactive_elements(std::vector<T>& array) {
    for (int i = 0; i < array.size(); ++i) {
        if (array.at(i).active == false) {
            array.at(i) = array.at(array.size() - 1); 
            array.pop_back();
            i--;
        }
    }
}

static Vector2 get_rec_center(Rectangle rec) {
    return {rec.x + rec.width / 2.f, rec.y + rec.height / 2.f};
}

static Rectangle to_rec(const Vector2& v1, const Vector2& v2) {
    return {v1.x, v1.y, v2.x, v2.y};
}
Game::Game() {
    levels.reserve(10);
}

Game::Game(const std::vector<Level>& levels) {
    this->levels.reserve(levels.size());
    for (int i = 0; i < levels.size(); ++i) {
        this->levels.push_back(levels[i]);
    }
}

void Game::start() {
    if (levels.size() == 0) {
        std::cerr << "No levels found!\n";
        return;
    }
    if (active_level < 0) active_level = 0;
    assert(active_level < levels.size());

    levels[active_level].start();
}

void Game::select_level(u64 index) {
    assert(index < levels.size());
    active_level = index;
}

void Game::update(Rectangle game_boundary) {
    assert(active_level < (int)levels.size());
    levels[active_level].update(game_boundary);
}

Level& Game::get_current_level() {
    assert(active_level < levels.size());
    return levels[active_level];
}

std::string Game::to_string() {
    std::string out = "Game: \n";
    out += "level count = "; out += std::to_string(levels.size()); out += "\n";
    for (int i = 0; i < levels.size(); ++i) {
        out += "level "; out += std::to_string(i); out += ": \n";
        out += levels[i].to_string("\t"); out += "\n";
    }
    return out;

}
Level::Level(const char* name, Rectangle bounds):name(name), map(bounds) {
    enemies.reserve(100); 
    towers.reserve(100); 
}

void Level::start() {
    time = 0.f;
    // TODO::choose
    active_round = 0;
}

void Level::update(Rectangle game_boundary) {
    time += GetFrameTime();
    update_round();
    update_spawners();
    update_towers();
    update_bullets(game_boundary);
    update_enemies();
}

void Level::add_tower(Tower tower) {
    towers.push_back(tower);
    map.add_rec(to_rec(tower.position, tower.size));
}

void Level::update_enemies() {
    for (Enemy& enemy : enemies) {
        enemy.update(map.waypoints);
    }
    remove_inactive_elements(enemies);
}

void Level::update_bullets(Rectangle game_boundary) {
    for (Projectile& bullet : bullets) {
        bullet.update(enemies, game_boundary);
    } 
    remove_inactive_elements(bullets);
}

void Level::update_spawners() {
    int i = 0;
    for (EnemySpawner& spawner: spawners) {
        if (spawner.active == false) continue;
        spawner.update(enemies);
        i++;
    }
}

void Level::update_towers() {
    for (Tower& tower : towers) {
        tower.update(enemies);
        if (tower.shot_ready()) {
            spawn_bullet(tower);
        }
    }     
    for (Tower& tower: towers) {
        remove_inactive_elements(tower.active_bullets);
    }
}

void Level::update_round() {
    assert(active_round < (int)rounds.size());

    if (active_round >= 0)
        rounds[active_round].update(enemies, spawners);
}

void Level::spawn_bullet(Tower& tower) {
    if (tower.target_lock == false) return;
    Projectile bullet;
    bullet.position = tower.get_center();
    bullet.direction = tower.direction;
    bullet.damage += tower.damage;
    bullets.push_back(bullet);
    tower.shoot();
}

std::string Level::to_string(const char* prefix) {
    if (prefix == nullptr) prefix = "";
    std::string out;
    out += prefix; out += "Level "; out += name; out += "\n";
    out += prefix; out += "active enemies: "; out += std::to_string(enemies.size()); out += "\n";  
    out += prefix; out += "active buildings: "; out += std::to_string(towers.size()); out += "\n";  
    return out;
}

void EnemySpawner::update(std::vector<Enemy>& enemies) {
    if (active == false) return;
    time_since_spawn += GetFrameTime();
    if (time_since_spawn < delay) return;
    spawn(enemies);
}

void EnemySpawner::spawn(std::vector<Enemy>& enemies) {
    Enemy enemy;
    enemy.type = type;
    enemy.set_position(position);
    enemies.push_back(enemy);
    time_since_spawn = 0.f;
    spawned++;
    // max = 0 => infinite spawn
    if (max == 0) return;

    if (spawned >= max) {
        active = false;
    }
}

void Tower::update(const std::vector<Enemy> enemies) {
    // find target
    time_since_shot += GetFrameTime() * 10;
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

bool Tower::shot_ready() {
    return time_since_shot >= reload_time;
} 

void Tower::shoot() {
    time_since_shot = 0.f;
}

Vector2 Tower::get_center() const {
    return {position.x + size.x / 2.f, position.y + size.y / 2.f};
}
void Projectile::update(std::vector<Enemy>& enemies, Rectangle game_boundary) {
    if (active == false) return;

    Vector2 dir;
    if (type == STRAIGHT) {
        dir = Vector2Scale(direction, speed * GetFrameTime());
    }
    else if (type == SEEK) {
        dir = Vector2Scale(Vector2Normalize(Vector2Subtract(target, position)), speed * GetFrameTime());
    }

    position = Vector2Add(position, dir);

    if (!CheckCollisionCircleRec(position, radius, game_boundary)) {
        active = false;
        return;
    }

    for (Enemy& enemy : enemies) {
        if (CheckCollisionCircleRec(position, radius, enemy.boundary)) {
            active = false;
            enemy.get_hit(damage);
            return;
        }
    }
}
void Enemy::set_position(Vector2 pos) {
    boundary.x = pos.x;
    boundary.y = pos.y;
}

Vector2 Enemy::get_position() const {
    return {boundary.x, boundary.y};
}

Vector2 Enemy::get_center() const {
    return {boundary.x + boundary.width / 2.f, boundary.y + boundary.height / 2.f};
}

void Enemy::update(const std::vector<Vector2>& waypoints) {
    if (hp <= 0.f) active = false;
    if (active == false) return;
    if (hit) hit = false;

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

void Enemy::check_waypoint(const Vector2 wp, u64 waypoints_size) {
    Vector2 pos = {boundary.x, boundary.y};
    if (Vector2Length(Vector2Subtract(pos, wp)) < 1.f) {
        next_waypoint++;
    }
    if (next_waypoint >= waypoints_size) {
        active = false;
        return;
    }
}

void Enemy::find_nearest_waypoint(const std::vector<Vector2>& waypoints) {
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

void Enemy::get_hit(float dmg) {
    hp -= dmg;
    hit = true;
}
Map::Map(Rectangle bounds): width(bounds.width), height(bounds.height) {
    Image img = GenImageColor(width, height, BROWN);
    ground_tex = LoadTextureFromImage(img);
    UnloadImage(img);

    waypoints.reserve(100);
}


void Map::ground_tex_from_image(const Image& img) {
    UnloadTexture(ground_tex);
    ground_tex = LoadTextureFromImage(img);
}

void Map::add_rec(Rectangle rec) {
    occupied_areas.push_back(rec);
}

bool Map::check_free(Rectangle rec) const {
    for (Rectangle occ : occupied_areas) {
        if (CheckCollisionRecs(rec, occ)) return false;
    }
    return true;
}

void Round::update(std::vector<Enemy>& enemies, std::vector<EnemySpawner>& spawners) {
    assert (next_event <= events.size());

    time += GetFrameTime();
    while (next_event < events.size() && time >= events[next_event].start) {
        events[next_event].spawn(enemies, spawners);
        next_event++;
    };
}

void SpawnEvent::spawn(std::vector<Enemy>& enemies, std::vector<EnemySpawner>& spawners) {
    for (int i = 0; i < ENEMY_TYPE_MAX; ++i) {
        EnemySpawner spawner;
        spawner.position = position;
        spawner.max = this->enemies[i];
        spawner.type = (Enemy_Type)i;
        spawner.delay = delay;
        spawners.push_back(spawner);
    }
}

#endif
