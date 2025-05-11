#include <iostream>
//#include "raylib/raylib/include/raylib.h"
#include "raylib.h"

int main() {
    InitWindow(300, 600, "Tower Defense"); 

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
