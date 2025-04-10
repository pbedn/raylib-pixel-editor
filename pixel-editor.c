#include "include/raylib.h"
#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define INIT_WINDOW_WIDTH 800.0f
#define INIT_WINDOW_HEIGHT 600.0f

typedef struct {
    float width;
    float height;
} Size;

Size initButtonSize   = { 120.0f, 24.0f };
Size initDropdownSize = { 120.0f, 24.0f };
Size initMarginSize   = { 6.0f, 6.0f };

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void btnSaveAsPNG();
static void Button007();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    SetTraceLogLevel(LOG_DEBUG);
    float screenWidth = INIT_WINDOW_WIDTH;
    float screenHeight = INIT_WINDOW_HEIGHT;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Pixel Editor");

    // Define controls variables
    bool dropdownEditMode = false;
    int dropdownActive = 0;
    Size buttonSize   = { };
    Size dropdownSize = { };
    Size marginSize   = { };


    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Scaling
        //----------------------------------------------------------------------------------
        float scaleFactor = GetScreenWidth() / INIT_WINDOW_WIDTH; // Calculate scale factor based on initial width
        TraceLog(LOG_DEBUG, "%f", scaleFactor);
        scaleFactor = Clamp(scaleFactor, 0.75, 1.25);

        buttonSize.width = initButtonSize.width * scaleFactor;
        buttonSize.height = initButtonSize.height * scaleFactor;
        dropdownSize.width = initButtonSize.width * scaleFactor;
        dropdownSize.height = initButtonSize.height * scaleFactor;
        marginSize.width = initMarginSize.width * scaleFactor;
        marginSize.height = initMarginSize.height * scaleFactor;
        //----------------------------------------------------------------------------------

        // Top
        Rectangle buttonRect1      = { marginSize.width, marginSize.height, buttonSize.width, buttonSize.height };
        Rectangle buttonRect2      = { marginSize.width * 2 + buttonSize.width, marginSize.height, buttonSize.width, buttonSize.height };
        // Define the label rectangle above the dropdown
        Rectangle labelRect = { 
            GetScreenWidth() - marginSize.width - dropdownSize.width, 
            buttonSize.height + marginSize.height * 2 - buttonSize.height - marginSize.height,
            dropdownSize.width, 
            buttonSize.height
        };

        // Middle left
        Rectangle canvasRect       = { marginSize.width, marginSize.height * 2 + buttonSize.height, GetScreenWidth() - (marginSize.width * 3 + dropdownSize.width), GetScreenHeight() - (marginSize.height * 4 + buttonSize.height * 2) };

        // Middle right
        Rectangle dropdownBoxRect  = { GetScreenWidth() - marginSize.width - dropdownSize.width, buttonSize.height + marginSize.height * 2, buttonSize.width, buttonSize.height };
        Rectangle paletteRect      = { GetScreenWidth() - marginSize.width - dropdownSize.width, buttonSize.height + marginSize.height * 3 + dropdownSize.height, buttonSize.width, GetScreenHeight() - (marginSize.height * 5 + buttonSize.height * 3) };
        paletteRect.height = paletteRect.height / 2 - marginSize.height / 2; // Half to make the rest for toolRect
        
        Rectangle toolRect = { 
            GetScreenWidth() - marginSize.width - dropdownSize.width, 
            GetScreenHeight() - marginSize.height - buttonSize.height - (marginSize.height + paletteRect.height), 
            buttonSize.width, 
            paletteRect.height 
        };

        // Bottom
        Rectangle BottomStatusRect = { marginSize.width, GetScreenHeight() - marginSize.height - buttonSize.height, GetScreenWidth() - marginSize.width * 2, buttonSize.height};
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 

            if (dropdownEditMode) GuiLock();

            GuiStatusBar(BottomStatusRect, "some status bar");
            GuiPanel(canvasRect, NULL);
            GuiPanel(paletteRect, NULL);
            GuiPanel(toolRect, NULL);
            if (GuiButton(buttonRect1, "Save as PNG")) btnSaveAsPNG(); 
            if (GuiButton(buttonRect2, "SAMPLE TEXT")) Button007();
            GuiLabel(labelRect, "Choosee Palette");
            if (GuiDropdownBox(dropdownBoxRect, "ONE;TWO;THREE", &dropdownActive, dropdownEditMode)) dropdownEditMode = !dropdownEditMode;

            GuiUnlock();
            //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Controls Functions Definitions (local)
//------------------------------------------------------------------------------------
static void btnSaveAsPNG()
{
    // TODO: Implement control logic
}
static void Button007()
{
    // TODO: Implement control logic
}
