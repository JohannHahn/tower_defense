#include <cassert>
#include <iostream>
#include <vector>
#include "includes/raylib.h"

typedef uint64_t u64;

constexpr u64 index(u64 x, u64 y, u64 width) {
    return x + y * width;
}

struct Window {
    Window(u64 width, u64 height, const char* title):
	width(width), height(height), title(title), resizable(false) {};
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
    //Creates Window!
    Context(Window window, u64 frame_counter, Tiles tiles):
	window(window), frame_counter(frame_counter), tiles(tiles) {
	game_boundary = {0.f, 0.f, (float)window.width, (float)window.height};
	InitWindow(window.width, window.height, window.title);
	SetTargetFPS(60);

	game_image = GenImageColor(game_boundary.width, game_boundary.height, bg_col);
	ImageFormat(&game_image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	Color c = *(Color*)(game_image.data);
	game_texture = LoadTextureFromImage(game_image);
    }
    Window window;
    u64 frame_counter = 0;
    Tiles tiles;
    Color bg_col = GRAY;
    Rectangle game_boundary;
    Image game_image;
    Texture game_texture;
    float tile_size = 100;

    void render_tiles() {
	for(u64 y  = 0; y < tiles.num_rows; ++y) {
	    for(u64 x = 0; x < tiles.num_cols; ++x) {
		u64 idx = index(x, y, tiles.num_cols);
		Tile tile = tiles.tiles[idx];
		assert(tile < TILE_MAX);
		Color tile_col = BLACK;
		if (tile == TILE_RED) tile_col = RED;
		if (tile == TILE_BROWN) tile_col = BROWN;
		ImageDrawRectangle(&game_image, x * tile_size, y * tile_size, tile_size, tile_size, tile_col);
	    }
	}
    }
    void render_texture() {
	UpdateTexture(game_texture, game_image.data);
	DrawTexture(game_texture, 0, 0, WHITE);
    }
};

void on_manual_resize(Context& ctx) {
    ctx.window.width = GetScreenWidth();
    ctx.window.height = GetScreenHeight();
}

int main() {
    //Creates window
    Context context = Context({1200, 900, "Tower Defense"}, 0, Tiles(10,10));
    context.window.resizable = true; 
    if (context.window.resizable) {
	SetWindowState(FLAG_WINDOW_RESIZABLE);
    }
    while (!WindowShouldClose()) {
	BeginDrawing();
	if (IsWindowResized() || IsWindowMaximized()) {
	    on_manual_resize(context);
	}
	ClearBackground(RED);
	context.render_tiles();
	context.render_texture();
	EndDrawing();
	context.frame_counter++;
    }
    CloseWindow();
    return 0;
}
