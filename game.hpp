#pragma once
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <iostream>
#include "raylib.h"
#include "raymath.h"
#include <cassert>
#include <vector>
#include <string>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t  byte;

// TODO:: REREFACTOR FUNCTIONS
template <class T>
static void remove_inactive_elements(std::vector<T>& array);

static Vector2 get_rec_center(Rectangle rec);

static Rectangle to_rec(const Vector2& v1, const Vector2& v2);

// could go boom boom
template<class T>
void write_to_blob(byte* blob, size_t& offset, T val) {
    size_t size = sizeof(T);
    memcpy(blob + offset, &val, size);
    offset += size;
}

template<class T>
void write_to_blob(byte* blob, size_t& offset, T* val, size_t size) {
    memcpy(blob + offset, val, size);
    offset += size;
}

template<class T>
void read_from_blob(byte* blob, size_t& offset, T& out) {
    size_t size = sizeof(T);
    memcpy(&out, blob + offset, size); 
    offset += size;
}

template<class T>
void read_from_blob(byte* blob, size_t& offset, T* out, size_t size) {
    memcpy(out, blob + offset, size); 
    offset += size;
}

template<class T>
void array_to_blob(byte* blob, size_t& offset, const std::vector<T>& array) {
    write_to_blob(blob, offset, array.size());
    write_to_blob(blob, offset, &array[0], array.size() * sizeof(T));
}

template<class T>
void array_from_blob(byte* blob, size_t& offset, std::vector<T>& array) {
    size_t size = 0; 
    read_from_blob(blob, offset, size);
    array.resize(size);
    read_from_blob(blob, offset, &array[0], size);
}

struct Level;

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

    size_t get_byte_size() {
        size_t num_bytes = sizeof(width) + sizeof(height) + sizeof(road_width);
        //waypoints.size
        num_bytes += sizeof(size_t);
        //occupied_areas.size
        num_bytes += sizeof(size_t);
        for (Vector2 vec : waypoints) num_bytes += sizeof(Vector2);
        for (Rectangle rec : occupied_areas) num_bytes += sizeof(Rectangle);
        return num_bytes; 
    }

    void save_to_blob(byte* blob, size_t& offset) {
        size_t local_offset = offset;
        write_to_blob(blob, offset, width) ;
        write_to_blob(blob, offset, height);
        write_to_blob(blob, offset, road_width);

        array_to_blob(blob, offset, waypoints);
        array_to_blob(blob, offset, occupied_areas);

        local_offset = offset - local_offset;
        std::printf("local offset = %llu ", local_offset);
        std::printf("offset = %llu ", offset);
        std::printf("byte_size = %llu ", get_byte_size());
        assert(local_offset == get_byte_size());
    }

    void load_from_blob(byte* blob, size_t& offset) {
        read_from_blob(blob, offset, width);
        read_from_blob(blob, offset, height);
        read_from_blob(blob, offset, road_width);

        array_from_blob(blob, offset, waypoints);
        array_from_blob(blob, offset, occupied_areas);
    }

};

enum Enemy_Type {
    CHICKEN, ENEMY_TYPE_MAX
};

struct Enemy {
    bool active = true;
    float hp = 100.f;
    float speed = 100.f;
    float damage = 1.f;
    Enemy_Type type = CHICKEN;
    Rectangle boundary = {0.f, 0.f, 10.f, 10.f};
    Vector2 direction = {1.f, 0.f};
    u64 next_waypoint = 0;
    u64 id;

    bool hit = false;

    void set_position(Vector2 pos);

    Vector2 get_position() const;
    Vector2 get_center() const;

    void update(const std::vector<Vector2>& waypoints);

    void find_nearest_waypoint(const std::vector<Vector2>& waypoints);

    void get_hit(float dmg);
};

struct EnemyRecord {
    bool active;
    Vector2 center;
};

enum Projectile_Type {
    STRAIGHT, SEEK
};

struct Projectile {
    bool active = true;
    float speed = 500.f; 
    float radius = 2.f;
    float damage = 2.f;
    u64 target_id;
    bool target_lost = false;
    Vector2 position;
    Vector2 direction;
    Projectile_Type type = STRAIGHT;

    void update(std::vector<Enemy>& enemies, std::vector<EnemyRecord>& enemy_records, Rectangle game_boundary);
};

enum Tower_Type {
    TOWER_BASIC, TOWER_SEEK  
};

struct Tower {
    float hp = 100.f;
    float damage = 10.f;
    float range = 100.f;
    float reload_time = 0.2f;
    float time_since_shot = reload_time;
    float turn_speed = 10.f;
    Tower_Type type = TOWER_SEEK;

    Vector2 position = {0.f, 0.f};
    Vector2 size = {10.f, 10.f};
    Vector2 direction = {10.f, 10.f};
    u64 target_id;
    bool target_lock = false;

    void update(const std::vector<Enemy>& enemies, const std::vector<EnemyRecord>& enemy_records);

    bool shot_ready();

    void shoot();

    Vector2 get_center() const;

    void turn_to_target(Vector2 target_dir);

};


struct EnemySpawner {
    bool active = true;
    Enemy_Type type = CHICKEN;
    Vector2 position = {0.f, 0.f};
    float delay = 1.f;
    float time_since_spawn = 0.f;
    u64 max = 0;
    u64 spawned = 0;

    void spawn(Level& level);
    void update(Level& level);
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
    std::vector<EnemyRecord> enemy_records;
    std::vector<Tower> towers;
    std::vector<EnemySpawner> spawners;
    std::vector<Projectile> bullets;
    std::vector<Round> rounds;
    const char* name; 
    float time = 0.f;
    int active_round = -1;
    u64 object_id_counter = 0;

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

    void add_enemy(Enemy& enemy);

    std::string to_string(const char* prefix = "");

    void save_to_file(const char* file_name) {
        size_t total_size = map.get_byte_size()+ 99999;
        byte* blob = new byte[total_size];
        size_t offset = 0;

        map.save_to_blob(blob, offset);

        array_to_blob(blob, offset, enemies);
        array_to_blob(blob, offset, enemy_records);
        array_to_blob(blob, offset, towers);
        array_to_blob(blob, offset, spawners);
        array_to_blob(blob, offset, bullets);
        array_to_blob(blob, offset, rounds);

        write_to_blob(blob, offset, name);
        write_to_blob(blob, offset, time);
        write_to_blob(blob, offset, object_id_counter);

        assert(offset < total_size);

        SaveFileData(file_name, blob, total_size);

        delete[] blob;
    }

    void load_from_file(const char* file_name) {
        int total_size = map.get_byte_size() + 99999 ;
        byte* blob = (byte*)LoadFileData(file_name, &total_size);
        size_t offset = 0;
        map.load_from_blob(blob, offset);

        array_from_blob(blob, offset, enemies);
        printf("hi");
        array_from_blob(blob, offset, enemy_records);
        array_from_blob(blob, offset, towers);
        array_from_blob(blob, offset, spawners);
        array_from_blob(blob, offset, bullets);
        array_from_blob(blob, offset, rounds);
        read_from_blob(blob, offset, name);
        read_from_blob(blob, offset, time);
        read_from_blob(blob, offset, object_id_counter);
        printf("hi");
        
    }

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
    paused = false;
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

void Level::add_enemy(Enemy& enemy) {
    enemy.id = object_id_counter++;
    enemies.push_back(enemy);
    EnemyRecord record = {.active = enemy.active, .center = enemy.get_center()};
    enemy_records.push_back(record);
}

void Level::add_tower(Tower tower) {
    towers.push_back(tower);
    map.add_rec(to_rec(tower.position, tower.size));
}

void Level::update_enemies() {
    for (Enemy& enemy : enemies) {
        enemy.update(map.waypoints);
        enemy_records[enemy.id].active = enemy.active;
        enemy_records[enemy.id].center = enemy.get_center();
    }
    remove_inactive_elements(enemies);
}

void Level::update_bullets(Rectangle game_boundary) {
    for (Projectile& bullet : bullets) {
        bullet.update(enemies, enemy_records, game_boundary);
    } 
    remove_inactive_elements(bullets);
}

void Level::update_spawners() {
    int i = 0;
    for (EnemySpawner& spawner: spawners) {
        if (spawner.active == false) continue;
        spawner.update(*this);
        i++;
    }
}

void Level::update_towers() {
    for (Tower& tower : towers) {
        tower.update(enemies, enemy_records);
        if (tower.shot_ready()) {
            spawn_bullet(tower);
        }
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
    // TODO convert method 
    bullet.type = (Projectile_Type)tower.type;
    bullet.target_id = tower.target_id;
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

void EnemySpawner::update(Level& level) {
    if (active == false) return;
    time_since_spawn += GetFrameTime();
    if (time_since_spawn < delay) return;
    spawn(level);
}

void EnemySpawner::spawn(Level& level) {
    Enemy enemy;
    enemy.type = type;
    enemy.set_position(position);
    level.add_enemy(enemy);
    time_since_spawn = 0.f;
    spawned++;
    // max = 0 => infinite spawn
    if (max == 0) return;

    if (spawned >= max) {
        active = false;
    }
}

void Tower::update(const std::vector<Enemy>& enemies, const std::vector<EnemyRecord>& enemy_records) {
    time_since_shot += GetFrameTime();
    if (target_lock == false) {
        u64 i = 0;
        for (const Enemy& enemy: enemies) {
            if (enemy.active == false) continue;
            Vector2 line_to_target = Vector2Subtract(enemy.get_center(), position);
            if (Vector2Length(line_to_target) <= range) {
                target_id = enemy.id;
                target_lock = true;
                break;
            }
            i++;
        }
    }

    if (target_lock) {
        EnemyRecord target = enemy_records[target_id];
        Vector2 line_to_target = Vector2Subtract(target.center, position);
        if (target.active == false || Vector2Length(line_to_target) > range) {
            target_lock = false;
            return;
        }
        
        //direction = Vector2Normalize(line_to_target);
        turn_to_target(line_to_target);
    }
}

void Tower::turn_to_target(Vector2 target_dir) {
    float angle_dif = Vector2Angle(target_dir, direction);
    if (angle_dif <= turn_speed) {
        direction = target_dir;
    }
    else {
        float angle = angle_dif < 0 ? -turn_speed : turn_speed;
        Vector2Rotate(direction, angle);
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
void Projectile::update(std::vector<Enemy>& enemies, std::vector<EnemyRecord>& enemy_records, Rectangle game_boundary) {
    if (active == false) return;


    Vector2 dir;
    if (type == STRAIGHT) {
        dir = Vector2Scale(direction, speed * GetFrameTime());
    }
    else if (type == SEEK) {
        // TODO:: find target -> array move event? listneres?
        EnemyRecord target = enemy_records.at(target_id);
        if (target.active == false) { 
            if (!target_lost) {
                target_lost = true;
                dir = Vector2Scale(Vector2Normalize(Vector2Subtract(target.center, position)), speed * GetFrameTime());
                direction = dir;
            }
        }
        else {
            dir = Vector2Scale(Vector2Normalize(Vector2Subtract(target.center, position)), speed * GetFrameTime());
        }
        if (target_lost) { 
            dir = direction;
        }
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

    assert(next_waypoint < waypoints.size());

    Vector2 wp = waypoints[next_waypoint]; 
    Vector2 pos = get_center();
    float distance = Vector2Length(Vector2Subtract(wp, pos));

    direction = Vector2Scale(Vector2Normalize(Vector2Subtract(wp, get_center())), speed * GetFrameTime());
    pos = Vector2Add(pos, direction);
    boundary.x = pos.x - boundary.width / 2.f;
    boundary.y = pos.y - boundary.height / 2.f;

    if (distance < 10.f) {
        //find_nearest_waypoint(waypoints);
        next_waypoint++;
        if (next_waypoint >= waypoints.size()) {
            active = false;
        }
    }
}


void Enemy::find_nearest_waypoint(const std::vector<Vector2>& waypoints) {
    if (waypoints.size() == 0) return;
    float min_distance = 999999.f;
    u64 nearest = 0;
    int i = 0;
    for (const Vector2& wp : waypoints) {
        float wp_dist = Vector2Distance(wp, get_center());
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
    if (hp <= 0.f) active = false;
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
