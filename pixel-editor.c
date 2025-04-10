#include "raylib.h"
#include <stdio.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION // Define this in one source file
#include "raygui.h"

#define GRID_SIZE 16
#define PALLETE_SIZE 64
#define PIXEL_SIZE 32
#define MAX_COLORS 64
#define PALETTE_WIDTH 150

#define MAX_PALETTES 16
#define MAX_PALETTE_NAME 64

#define TOP_BAR_HEIGHT 30
#define BOTTOM_BAR_HEIGHT 24
#define MARGIN 10

typedef struct {
  Color colors[MAX_COLORS];
  int count;
  char name[MAX_PALETTE_NAME];
} Palette;

Palette palettes[MAX_PALETTES];
int paletteCount = 0;
int currentPaletteIndex = 0;
int dropdownActive = 0;

Rectangle dropdownBounds;

Color canvas[GRID_SIZE][GRID_SIZE];
Color currentColor;

int gridOriginX, gridOriginY;
char dropdownBuffer[1024] = {0}; // Buffer to hold the concatenated palette names

//----------------------------------------------------------------------------------
// Functions Declaration
//----------------------------------------------------------------------------------
static void btnSaveAsPNG();
void LoadPalettesFromDir(const char *dirPath);
void DropdownBufferString();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  const int gridPixels = GRID_SIZE * PIXEL_SIZE;
  const int screenWidth = gridPixels + PALETTE_WIDTH + 3 * MARGIN;
  const int screenHeight =
      gridPixels + TOP_BAR_HEIGHT + BOTTOM_BAR_HEIGHT + 2 * MARGIN;

  InitWindow(screenWidth, screenHeight, "Raylib Pixel Editor");
  SetTargetFPS(60);

  Font uiFont = LoadFont("fonts/PressStart2P-Regular.ttf");

  // Load palettes from directory
  LoadPalettesFromDir("palettes");
  if (paletteCount == 0) {
    TraceLog(LOG_WARNING, "No palettes found.");
    CloseWindow();
    return 1;
  }

  currentColor = palettes[0].colors[0];
  gridOriginX = MARGIN;
  gridOriginY = TOP_BAR_HEIGHT + MARGIN;

  // Define the grid rectangle based on your grid's origin and size
  Rectangle gridBounds = {
      gridOriginX,
      gridOriginY,
      GRID_SIZE * PIXEL_SIZE, // Width of the grid
      GRID_SIZE * PIXEL_SIZE   // Height of the grid
  };

  // Initialize the canvas
  for (int y = 0; y < GRID_SIZE; y++)
    for (int x = 0; x < GRID_SIZE; x++)
      canvas[y][x] = BLANK;

  // Create a string for the dropdown containing palette names
  DropdownBufferString();

  int dropdownActive = 0;
  int selectedPaletteIndex = 0;

  while (!WindowShouldClose()) {

    // Handle mouse
    Vector2 mouse = GetMousePosition();
    int gx = (mouse.x - gridOriginX) / PIXEL_SIZE;
    int gy = (mouse.y - gridOriginY) / PIXEL_SIZE;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      if (CheckCollisionPointRec(mouse, dropdownBounds)) {
        selectedPaletteIndex = !selectedPaletteIndex;

      // Set the canvas color at the calculated grid position
      } else if (CheckCollisionPointRec(mouse, gridBounds)) {
        canvas[gy][gx] = currentColor;

      // CAN THIS BE SIMPLIFIED & REFACTORED
     // Set the palette color at the calculated palette position
      } else {
        int px = gridOriginX + GRID_SIZE * PIXEL_SIZE + MARGIN;
        int ph = screenHeight - TOP_BAR_HEIGHT - BOTTOM_BAR_HEIGHT - 2 * MARGIN;
        int count = palettes[currentPaletteIndex].count;
        int maxPerColumn = 8; // Maximum number of items per column

        for (int i = 0; i < count; i++) {
          // Calculate the column and row based on the index
          int column =
              i / maxPerColumn; // 0 for first column, 1 for second column, etc.
          int row = i % maxPerColumn; // Row within the column

          // Calculate the rectangle for the color
          Rectangle colRect = {px + column * (PALLETE_SIZE + MARGIN),
                               gridOriginY + row * PALLETE_SIZE, PALLETE_SIZE,
                               PALLETE_SIZE};

          // Check if the mouse is over the color rectangle
          if (CheckCollisionPointRec(mouse, colRect)) {
            currentColor = palettes[currentPaletteIndex].colors[i];
          }
        }
      }
    } else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
      if (gx >= 1 && gy >= 1 && gx < GRID_SIZE - 1 && gy < GRID_SIZE - 1 &&
          mouse.y < screenHeight - BOTTOM_BAR_HEIGHT) {
        canvas[gy][gx] = BLANK;
      }
    }

    // IS THIS NEEDED ?
    // if (dropdownActive && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    //     for (int i = 0; i < paletteCount; i++) {
    //         Rectangle item = {dropdownBounds.x, dropdownBounds.y + 20 + i *
    //         20, dropdownBounds.width, 20}; if (CheckCollisionPointRec(mouse,
    //         item)) {
    //             currentPaletteIndex = i;
    //             currentColor = palettes[i].colors[0];
    //             dropdownActive = 0;
    //         }
    //     }
    // }

    if (IsKeyPressed(KEY_S)) btnSaveAsPNG();

    // ─────────── Drawing UI ─────────────
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Top bar
    Rectangle buttonRect1 = {10, 5, 100, 30};
    if (GuiButton(buttonRect1, "Save PNG")) btnSaveAsPNG();

    // Grid
    for (int y = 0; y < GRID_SIZE; y++) {
      for (int x = 0; x < GRID_SIZE; x++) {
        Color col = (canvas[y][x].a == 0) ? DARKGRAY : canvas[y][x];
        DrawRectangle(gridOriginX + x * PIXEL_SIZE,
                      gridOriginY + y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE,
                      col);
        DrawRectangleLines(gridOriginX + x * PIXEL_SIZE,
                           gridOriginY + y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE,
                           GRAY);
      }
    }

    // Palette
    int paletteX = gridOriginX + GRID_SIZE * PIXEL_SIZE + MARGIN;
    int paletteH =
        screenHeight - TOP_BAR_HEIGHT - BOTTOM_BAR_HEIGHT - 2 * MARGIN;
    int count = palettes[currentPaletteIndex].count;
    int maxPerColumn = 8; // Maximum number of items per column
    for (int i = 0; i < count; i++) {
      // Calculate the column and row based on the index
      int column =
          i / maxPerColumn; // 0 for first column, 1 for second column, etc.
      int row = i % maxPerColumn; // Row within the column
      // Calculate the x and y positions
      int xPosition = paletteX + column * (PALLETE_SIZE +
                                           MARGIN); // Adjust x for each column
      int yPosition =
          gridOriginY + row * PALLETE_SIZE; // Y position based on row
      DrawRectangle(xPosition, yPosition, PALLETE_SIZE, PALLETE_SIZE,
                    palettes[currentPaletteIndex].colors[i]);
    }

    // Dropdown for palette selection using Raygui
    if (GuiDropdownBox((Rectangle){paletteX, 5, PALLETE_SIZE * 2 + MARGIN, 30},
                       dropdownBuffer, &selectedPaletteIndex, dropdownActive)) {
      dropdownActive = !dropdownActive; // Toggle dropdown state
      currentPaletteIndex =
          selectedPaletteIndex; // Update the current palette index
      currentColor = palettes[currentPaletteIndex]
                         .colors[0]; // Set the current color to the first color
                                     // of the selected palette
    }

    // Bottom status bar
    DrawRectangle(0, screenHeight - BOTTOM_BAR_HEIGHT, screenWidth,
                  BOTTOM_BAR_HEIGHT, LIGHTGRAY);
    DrawTextEx(uiFont,
               TextFormat("Palette: %s | Color: #%02X%02X%02X",
                          palettes[currentPaletteIndex].name, currentColor.r,
                          currentColor.g, currentColor.b),
               (Vector2){10, screenHeight - BOTTOM_BAR_HEIGHT + 8}, uiFont.baseSize*0.26f, 1,
               BLACK);

    EndDrawing();
  }

  UnloadFont(uiFont);
  CloseWindow();
  return 0;
}

//------------------------------------------------------------------------------------
// Controls Functions Definitions (local)
//------------------------------------------------------------------------------------
static void btnSaveAsPNG() {
  Image image = GenImageColor(GRID_SIZE, GRID_SIZE, BLANK);
  for (int y = 0; y < GRID_SIZE; y++)
    for (int x = 0; x < GRID_SIZE; x++)
      ImageDrawPixel(&image, x, y, canvas[y][x]);
  ExportImage(image, "pixel_art.png");
  UnloadImage(image);
}

//------------------------------------------------------------------------------------
// Helper Functions Definitions
//------------------------------------------------------------------------------------
void LoadPalettesFromDir(const char *dirPath) {
  FilePathList files = LoadDirectoryFiles(dirPath);
  for (int i = 0; i < files.count && paletteCount < MAX_PALETTES; i++) {
    if (!IsPathFile(files.paths[i]))
      continue;

    FILE *fp = fopen(files.paths[i], "r");
    if (!fp)
      continue;

    Palette p = {0};
    strncpy(p.name, GetFileNameWithoutExt(files.paths[i]),
            MAX_PALETTE_NAME - 1);
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
      if (line[0] == ';' || strlen(line) < 8)
        continue;
      unsigned int r, g, b;
      if (sscanf(line, "FF%02x%02x%02x", &r, &g, &b) == 3 &&
          p.count < MAX_COLORS) {
        p.colors[p.count++] = (Color){r, g, b, 255};
      }
    }

    fclose(fp);
    if (p.count > 0) {
      palettes[paletteCount++] = p;
    }
  }
  UnloadDirectoryFiles(files);
}

void DropdownBufferString() {
  int currentLength =
      strlen(dropdownBuffer); // Start with the current length (initially 0)

  for (int i = 0; i < paletteCount; i++) {
    // Format the new item
    char item[256]; // Temporary buffer for the item
    snprintf(item, sizeof(item), "%s",
             palettes[i].name); // Create the item string

    // Calculate the length of the new item
    int itemLength = strlen(item);

    // Check if adding this item would exceed the buffer size
    if (currentLength + itemLength + (i > 0 ? 1 : 0) < sizeof(dropdownBuffer)) {
      if (i > 0) {
        strncat(dropdownBuffer, ";",
                sizeof(dropdownBuffer) - currentLength - 1); // Add separator
        currentLength += 1; // Update the current length for the separator
      }
      strncat(dropdownBuffer, item,
              sizeof(dropdownBuffer) - currentLength - 1); // Append the item
      currentLength += itemLength; // Update the current length
    } else {
      // Handle the case where the buffer is too small
      TraceLog(
          LOG_WARNING,
          "Dropdown buffer size exceeded. Some palettes may not be displayed.");
      break; // Exit the loop if the buffer is full
    }
  }
}