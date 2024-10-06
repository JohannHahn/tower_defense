#include <iostream>
#include "includes/raylib.h"

typedef uint64_t u64;

struct Window {
    Window(u64 width, u64 height, const char* title):
	width(width), height(height), title(title), resizable(false) {};
    u64 width;
    u64 height;
    const char* title;
    bool resizable = false;
};

struct Context {
    Window window;
    u64 frame_counter = 0;
};

Context context = {.window = {1200, 900, "Tower Defense"}};

void on_manual_resize(Context& ctx) {
    ctx.window.width = GetScreenWidth();
    ctx.window.height = GetScreenHeight();
    SetWindowSize(ctx.window.width, ctx.window.height);
}

void on_maximize(Context& ctx) {
    
}

int main() {
    context.window.resizable = true; 
    InitWindow(context.window.width, context.window.height, context.window.title);

    SetWindowState(FLAG_WINDOW_HIGHDPI);
    SetTargetFPS(60);
    if (context.window.resizable) {
	SetWindowState(FLAG_WINDOW_RESIZABLE);
    }

    while (!WindowShouldClose()) {
	BeginDrawing();
	if (IsWindowResized()) {
	    on_manual_resize(context);
	}
	if (IsWindowMaximized()) {
	    on_manual_resize(context);
	}
	if (context.frame_counter % 10 == 0) {
	    std::cout << "window size = " << context.window.width  << ", " << context.window.height << "\n";
	    std::cout << "raylib window size = " << GetScreenWidth()  << ", " << GetScreenHeight() << "\n";
	}
	ClearBackground(RED);
	EndDrawing();
	context.frame_counter++;
    }
    return 0;
}
