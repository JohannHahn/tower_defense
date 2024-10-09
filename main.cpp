#include <cassert>
#include <iostream>
#include <vector>
#include "includes/raylib.h"

typedef uint64_t u64;

constexpr u64 index(u64 x, u64 y, u64 width) {
    return x + y * width;
}

struct Window {
    Window(u64 width, u64 height, const char* title, bool resizable = false):
	width(width), height(height), title(title), resizable(resizable) {
    };
    u64 width;
    u64 height;
    const char* title;
    bool resizable = false;
};

enum Tile {
    EMPTY, TILE_RED, TILE_BROWN, TILE_MAX
};

Tile random_tile() {
    return (Tile)GetRandomValue(EMPTY, TILE_MAX - 1);
}

struct Tiles {
    u64 num_cols;
    u64 num_rows;
    u64 cursor = 0;
    std::vector<Tile> tiles;
    Tiles(u64 num_rows, u64 num_cols): 
	num_rows(num_rows), num_cols(num_cols) {
	tiles.reserve(num_cols * num_rows);
	for(u64 i = 0; i < num_cols * num_rows; ++i) {
	    tiles.push_back(random_tile());
	}
    }
};

struct Context {
    Context(Window window, u64 frame_counter, Tiles tiles, 
	    Color bg_col = GRAY, Rectangle game_boundary = {0.f, 0.f, 1200.f, 900.f}):
	window(window), frame_counter(frame_counter), tiles(tiles), bg_col(bg_col), game_boundary(game_boundary) {
	if (game_boundary.width > window.width) game_boundary.width = window.width;
	if (game_boundary.height > window.height) game_boundary.height = window.height;
    }
    Window window;
    u64 frame_counter = 0;
    Tiles tiles;
    Color bg_col = GRAY;
    float tile_size = 100;
    Rectangle game_boundary;
    Image game_image;
    Texture game_texture;
    Image tile_textures[TILE_MAX];
    Color colors[TILE_MAX] = {BLACK, RED, BROWN};
    u64 cursor = 0;

    void render_tiles() {
	for(u64 y  = 0; y < tiles.num_rows; ++y) {
	    for(u64 x = 0; x < tiles.num_cols; ++x) {
		u64 idx = index(x, y, tiles.num_cols);
		Tile tile = tiles.tiles[idx];
		assert(tile < TILE_MAX);
		Color tile_col =  colors[tile];
		ImageDrawRectangle(&game_image, x * tile_size, y * tile_size, tile_size, tile_size, tile_col);
	    }
	}
	// render selected tile
	u64 x = cursor % tiles.num_cols;
	u64 y = cursor / tiles.num_cols;
	ImageDrawRectangleLines(&game_image, {x * tile_size, y * tile_size, tile_size, tile_size}, 3, WHITE);
    }
    void render_texture() {
	UpdateTexture(game_texture, game_image.data);
	DrawTexture(game_texture, 0, 0, WHITE);
    }
};

void init_window(Window& window) {
    InitWindow(window.width, window.height, window.title);
    if (window.resizable) {
	SetWindowState(FLAG_WINDOW_RESIZABLE);
    }
    SetTargetFPS(60);
}
void on_manual_resize(Context& ctx) {
    ctx.window.width = GetScreenWidth();
    ctx.window.height = GetScreenHeight();
}

void setup_textures(Context& ctx) {
    ctx.game_image = GenImageColor(ctx.game_boundary.width, ctx.game_boundary.height, ctx.bg_col);
    ctx.game_texture = LoadTextureFromImage(ctx.game_image);
    for (int i = 0; i < TILE_MAX; ++i) {
	ctx.tile_textures[i] = GenImageColor(ctx.tile_size, ctx.tile_size, ctx.colors[i]);
    }
}

void controls(Context& ctx) {
    if (IsKeyPressed(KEY_RIGHT)) ctx.cursor++;
    else if (IsKeyPressed(KEY_LEFT)) ctx.cursor--;
    else if (IsKeyPressed(KEY_DOWN)) ctx.cursor += ctx.tiles.num_cols;
    else if (IsKeyPressed(KEY_UP)) ctx.cursor -= ctx.tiles.num_cols;
    ctx.cursor %= ctx.tiles.tiles.size();
}

int main() {
    Window window = Window(1300, 900, "Tower Defense", true);
    Context ctx = Context(window, 0, Tiles(10,20));
    // calls InitWindow from raylib
    init_window(window);
    setup_textures(ctx);
    while (!WindowShouldClose()) {
	controls(ctx);
	BeginDrawing();
	if (IsWindowResized() || IsWindowMaximized()) {
	    on_manual_resize(ctx);
	}
	ClearBackground(RED);
	ctx.render_tiles();
	ctx.render_texture();
	EndDrawing();
	ctx.frame_counter++;
    }
    CloseWindow();
    return 0;
}
