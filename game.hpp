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

// tools
enum Log_Level {
    NO_LOG, LOW, FULL, 
};


template <class T>
void log_var(T var, const char* name = nullptr, Log_Level log_lvl = FULL) {
    if (log_lvl == NO_LOG) return;

    if (name && strlen(name) > 0) {
        std::cout << name << ":\n";
    }
    std::cout << var << "\n";
}

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


void string_to_blob(byte* blob, size_t& offset, const std::string& string) {
    write_to_blob(blob, offset, string.size());
    write_to_blob(blob, offset, &string[0], string.size());
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
    read_from_blob(blob, offset, &array[0], size * sizeof(T));
}

template<class T>
void struct_array_to_blob(byte* blob, size_t& offset, const std::vector<T>& array) {
    write_to_blob(blob, offset, array.size());
    for(T t : array) {
        t.save_to_blob(blob, offset);
    }
}

template<class T>
void struct_array_from_blob(byte* blob, size_t& offset, std::vector<T>& array) {
    size_t size = 0;
    read_from_blob(blob, offset, size);
    array.resize(size);
    for (T& t : array) {
        t.load_from_blob(blob, offset);
    }
}

void string_from_blob(byte* blob, size_t& offset, std::string& out) {
    size_t size;
    read_from_blob(blob, offset, size);
    out.resize(size);
    read_from_blob(blob, offset, &out[0], size);
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

    size_t get_byte_size() const {
        size_t num_bytes = sizeof(width) + sizeof(height) + sizeof(road_width);
        //waypoints.size
        num_bytes += sizeof(size_t);
        //occupied_areas.size
        num_bytes += sizeof(size_t);
        for (Vector2 vec : waypoints) num_bytes += sizeof(Vector2);
        for (Rectangle rec : occupied_areas) num_bytes += sizeof(Rectangle);
        return num_bytes; 
    }

    void save_to_blob(byte* blob, size_t& offset) const {
        size_t local_offset = offset;
        write_to_blob(blob, offset, width) ;
        write_to_blob(blob, offset, height);
        write_to_blob(blob, offset, road_width);

        array_to_blob(blob, offset, waypoints);
        array_to_blob(blob, offset, occupied_areas);

        local_offset = offset - local_offset;
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

    size_t get_byte_size() const {
        size_t size = 0;
        size += sizeof(active);
        size += sizeof(hp);
        size += sizeof(speed);
        size += sizeof(damage);
        size += sizeof(type);
        size += sizeof(boundary);
        size += sizeof(direction);
        size += sizeof(next_waypoint);
        size += sizeof(id);
        size += sizeof(hit);
        return size;
    }

    void save_to_blob(byte* blob, size_t& offset) const {
        size_t local_offset = offset;

        write_to_blob(blob, offset, active);
        write_to_blob(blob, offset, hp);
        write_to_blob(blob, offset, speed);
        write_to_blob(blob, offset, damage);
        write_to_blob(blob, offset, type);
        write_to_blob(blob, offset, boundary);
        write_to_blob(blob, offset, direction);
        write_to_blob(blob, offset, next_waypoint);
        write_to_blob(blob, offset, id);
        write_to_blob(blob, offset, hit);

        local_offset = offset - local_offset;
        assert(local_offset == get_byte_size());
    }

    void load_from_blob(byte* blob, size_t& offset) {
        read_from_blob(blob, offset, active);
        read_from_blob(blob, offset, hp);
        read_from_blob(blob, offset, speed);
        read_from_blob(blob, offset, damage);
        read_from_blob(blob, offset, type);
        read_from_blob(blob, offset, boundary);
        read_from_blob(blob, offset, direction);
        read_from_blob(blob, offset, next_waypoint);
        read_from_blob(blob, offset, id);
        read_from_blob(blob, offset, hit);
    }
};

struct EnemyRecord {
    bool active;
    Vector2 center;

    size_t get_byte_size() const {
        return sizeof(active) + sizeof(center);
    }

    void save_to_blob(byte* blob, size_t& offset) const {
        size_t local_offset = offset;

        write_to_blob(blob, offset, active);
        write_to_blob(blob, offset, center);

        local_offset = offset - local_offset;
        assert(local_offset == get_byte_size());
    }

    void load_from_blob(byte* blob, size_t& offset) {
        read_from_blob(blob, offset, active);
        read_from_blob(blob, offset, center);
    }
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
    
    size_t get_byte_size() const {
        size_t size = sizeof(active);
        size += sizeof(speed);
        size += sizeof(radius);
        size += sizeof(damage);
        size += sizeof(target_id);
        size += sizeof(target_lost);
        size += sizeof(position);
        size += sizeof(direction);
        size += sizeof(type);
        return size;
    }

    void save_to_blob(byte* blob, size_t& offset) const {
        size_t local_offset = offset;

        write_to_blob(blob, offset, active);
        write_to_blob(blob, offset, speed);
        write_to_blob(blob, offset, radius);
        write_to_blob(blob, offset, damage);
        write_to_blob(blob, offset, target_id);
        write_to_blob(blob, offset, target_lost);
        write_to_blob(blob, offset, position);
        write_to_blob(blob, offset, direction);
        write_to_blob(blob, offset, type);

        local_offset = offset - local_offset;
        assert(local_offset == get_byte_size());
    }

    void load_from_blob(byte* blob, size_t& offset) {
        read_from_blob(blob, offset, active);
        read_from_blob(blob, offset, speed);
        read_from_blob(blob, offset, radius);
        read_from_blob(blob, offset, damage);
        read_from_blob(blob, offset, target_id);
        read_from_blob(blob, offset, target_lost);
        read_from_blob(blob, offset, position);
        read_from_blob(blob, offset, direction);
        read_from_blob(blob, offset, type);
    }

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

    size_t get_byte_size() const {
        size_t size = 0;
        size += sizeof(hp);
        size += sizeof(damage);
        size += sizeof(range);
        size += sizeof(reload_time);
        size += sizeof(time_since_shot);
        size += sizeof(turn_speed);
        size += sizeof(type);
        size += sizeof(position);
        size += sizeof(this->size);
        size += sizeof(direction);
        size += sizeof(target_id);
        size += sizeof(target_lock);
        return size;
    } 

    void save_to_blob(byte* blob, size_t& offset) const {
        size_t local_offset = offset;        

        write_to_blob(blob, offset, hp);
        write_to_blob(blob, offset, damage);
        write_to_blob(blob, offset, range);
        write_to_blob(blob, offset, reload_time);
        write_to_blob(blob, offset, time_since_shot);
        write_to_blob(blob, offset, turn_speed);
        write_to_blob(blob, offset, type);
        write_to_blob(blob, offset, position);
        write_to_blob(blob, offset, size);
        write_to_blob(blob, offset, direction);
        write_to_blob(blob, offset, target_id);
        write_to_blob(blob, offset, target_lock);

        local_offset = offset - local_offset;
        assert(local_offset == get_byte_size());
    }

    void load_from_blob(byte* blob, size_t& offset) {
        read_from_blob(blob, offset, hp);
        read_from_blob(blob, offset, damage);
        read_from_blob(blob, offset, range);
        read_from_blob(blob, offset, reload_time);
        read_from_blob(blob, offset, time_since_shot);
        read_from_blob(blob, offset, turn_speed);
        read_from_blob(blob, offset, type);
        read_from_blob(blob, offset, position);
        read_from_blob(blob, offset, size);
        read_from_blob(blob, offset, direction);
        read_from_blob(blob, offset, target_id);
        read_from_blob(blob, offset, target_lock);
    }

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

    size_t get_byte_size() const {
        size_t size = 0;
        size += sizeof(active);
        size += sizeof(type);
        size += sizeof(position);
        size += sizeof(delay);
        size += sizeof(time_since_spawn);
        size += sizeof(max);
        size += sizeof(spawned);
        return size;
    }

    void save_to_blob(byte* blob, size_t& offset) const {
        size_t local_offset = offset;

        write_to_blob(blob, offset, active);
        write_to_blob(blob, offset, type);
        write_to_blob(blob, offset, position);
        write_to_blob(blob, offset, delay);
        write_to_blob(blob, offset, time_since_spawn);
        write_to_blob(blob, offset, max);
        write_to_blob(blob, offset, spawned);

        local_offset = offset - local_offset;
        assert(local_offset == get_byte_size());
    }

    void load_from_blob(byte* blob, size_t& offset) {
        read_from_blob(blob, offset, active);
        read_from_blob(blob, offset, type);
        read_from_blob(blob, offset, position);
        read_from_blob(blob, offset, delay);
        read_from_blob(blob, offset, time_since_spawn);
        read_from_blob(blob, offset, max);
        read_from_blob(blob, offset, spawned);
    }
};

struct SpawnEvent {
    float start;   
    float delay;
    u64 enemies[ENEMY_TYPE_MAX];
    Vector2 position;


    void spawn(std::vector<Enemy>& enemies, std::vector<EnemySpawner>& spawners);

    size_t get_byte_size() const {
        size_t size = 0;
        size += sizeof(start);
        size += sizeof(delay);
        size += ENEMY_TYPE_MAX * sizeof(u64);
        size += sizeof(position);
        return size;
    } 

    void save_to_blob(byte* blob, size_t& offset) const {
        size_t local_offset = offset;

        write_to_blob(blob, offset, start);
        write_to_blob(blob, offset, delay);
        write_to_blob(blob, offset, &enemies[0], sizeof(u64) * ENEMY_TYPE_MAX);
        write_to_blob(blob, offset, position);

        local_offset = offset - local_offset;
        assert(local_offset == get_byte_size());
    }

    void load_from_blob(byte* blob, size_t& offset) {
        read_from_blob(blob, offset, start);
        read_from_blob(blob, offset, delay);
        read_from_blob(blob, offset, &enemies[0], sizeof(u64) * ENEMY_TYPE_MAX);
        read_from_blob(blob, offset, position);
    }
    
};

struct Round {
    std::vector<SpawnEvent> events; 
    float length;
    float time = 0.f;
    u64 next_event = 0;

    void update(std::vector<Enemy>& enemies, std::vector<EnemySpawner>& spawners);

    size_t get_byte_size() const {
        size_t size = 0;

        size += sizeof(size_t);
        for (const SpawnEvent e : events) {
            size += e.get_byte_size(); 
        }
        size += sizeof(length);
        size += sizeof(time);
        size += sizeof(next_event);
        return size;
    } 

    void save_to_blob(byte* blob, size_t& offset) const {
        size_t local_offset = offset;

        struct_array_to_blob(blob, offset, events);
        write_to_blob(blob, offset, length);
        write_to_blob(blob, offset, time);
        write_to_blob(blob, offset, next_event);

        local_offset = offset - local_offset;
        assert(local_offset == get_byte_size());
    }

    void load_from_blob(byte* blob, size_t& offset) {
        struct_array_from_blob(blob, offset, events);
        read_from_blob(blob, offset, length);
        read_from_blob(blob, offset, time);
        read_from_blob(blob, offset, next_event);
    }
};

struct Level {
    Map map;

    std::vector<Enemy> enemies;
    std::vector<EnemyRecord> enemy_records;
    std::vector<Tower> towers;
    std::vector<EnemySpawner> spawners;
    std::vector<Projectile> bullets;
    std::vector<Round> rounds;

    std::string name; 
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

    size_t get_byte_size() const {
        size_t size = 0;
        size += map.get_byte_size();

        size += sizeof(size_t);
        for (const Enemy& e : enemies) size += e.get_byte_size();

        size += sizeof(size_t);
        for (const EnemyRecord& er : enemy_records) size += er.get_byte_size();

        size += sizeof(size_t);
        for (const Tower& t : towers) size += t.get_byte_size();

        size += sizeof(size_t);
        for (const EnemySpawner& es : spawners) size += es.get_byte_size();

        size += sizeof(size_t);
        for (const Projectile& p : bullets) size += p.get_byte_size();

        size += sizeof(size_t);
        for (const Round& r : rounds) size += r.get_byte_size();

        size += sizeof(size_t);
        size += name.size();

        size += sizeof(time);
        size += sizeof(active_round);
        size += sizeof(object_id_counter);

        return size;
    }

    void save_to_file(const char* file_name) const {
        size_t total_size = get_byte_size();
        byte* blob = new byte[total_size];
        size_t offset = 0;

        map.save_to_blob(blob, offset);

        struct_array_to_blob(blob, offset, enemies);
        struct_array_to_blob(blob, offset, enemy_records);
        struct_array_to_blob(blob, offset, towers);
        struct_array_to_blob(blob, offset, spawners);
        struct_array_to_blob(blob, offset, bullets);
        struct_array_to_blob(blob, offset, rounds);

        string_to_blob(blob, offset, name);
        write_to_blob(blob, offset, time);
        write_to_blob(blob, offset, active_round);
        write_to_blob(blob, offset, object_id_counter);

        assert(offset == total_size);

        SaveFileData(file_name, blob, total_size);

        delete[] blob;
    }

    void load_from_file(const char* file_name) {
        int total_size = 0;
        byte* blob = (byte*)LoadFileData(file_name, &total_size);
        size_t offset = 0;
        map.load_from_blob(blob, offset);

        struct_array_from_blob(blob, offset, enemies);
        struct_array_from_blob(blob, offset, enemy_records);
        struct_array_from_blob(blob, offset, towers);
        struct_array_from_blob(blob, offset, spawners);
        struct_array_from_blob(blob, offset, bullets);
        struct_array_from_blob(blob, offset, rounds);

        string_from_blob(blob, offset, name);
        read_from_blob(blob, offset, time);
        write_to_blob(blob, offset, active_round);
        read_from_blob(blob, offset, object_id_counter);

        UnloadFileData(blob);
    }

};

struct LevelEditor {
    void place_tower(const Tower& tower, Level& level) {
        level.add_tower(tower);
    }    

    void place_spawner(const EnemySpawner& spawner, Level& level) {
        level.spawners.push_back(spawner);
    }
};

struct Game {
    std::vector<Level> levels;
    int active_level = -1;
    Tower* selected_building = nullptr;
    bool paused = true;
    bool edit_mode = false;
    Level edit_level;
    Rectangle boundary;

    Game();

    Game(Rectangle boundary, const std::vector<Level>& levels);

    void start();

    void select_level(u64 index);

    void update();

    void start_edit() {
        edit_level = Level("New Level", boundary);
        edit_mode = true;
    }

    void stop_edit() {
        edit_mode = false;
        paused = true;
    }

    Level& get_current_level();

    std::string to_string();
};

struct GameController {

    static void update(Game& game) {
        if (IsKeyPressed(KEY_SPACE)) {
            game.paused = !game.paused;
        }
        Vector2 position = {(float)GetMouseX(), (float)GetMouseY()};
        Tower tower;
        tower.position = position;
        Rectangle rec = {to_rec(tower.position, tower.size)};

        Level* level = nullptr;
        if (game.edit_mode) level = &game.edit_level;

        else level = &game.get_current_level();
        if (level->map.check_free(rec)) {
            DrawRectangleRec(rec, GREEN);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                level->add_tower(tower);
            }
                
        } else {
            DrawRectangleRec(rec, RED);
        }
    }
};

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
// edit level gets reinitialized later
Game::Game(): boundary({0, 0, 1200, 900}), edit_level(Level("New Level", boundary)) {
    levels.reserve(10);
}

Game::Game(Rectangle boundary, const std::vector<Level>& levels)
    : edit_level(Level("New Level", boundary)) {

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
    // already active
    if (active_level >= 0) {
        assert(active_level < levels.size());
        return;
    }

    select_level(0);
    levels[active_level].start();
    paused = false;
}

void Game::select_level(u64 index) {
    assert(index < levels.size());
    active_level = index;
}

void Game::update() {
    if (edit_mode) {
        return;
    }
    assert(active_level < (int)levels.size());
    levels[active_level].update(boundary);
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

