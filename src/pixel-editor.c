#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "pixel_core.h"
#include "pixel_ui_logic.h"

#define RAYGUI_IMPLEMENTATION  // Define this in one source file
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-result"
#endif
#include "raygui.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include "../styles/style_dark.h"              // raygui style: dark


// Define the maximum number of colors and palettes
#define MAX_COLORS 64
#define MAX_PALETTES 16
#define MAX_PALETTE_NAME 64

// Define UI dimensions
#define TOP_BAR_HEIGHT 30
#define BOTTOM_BAR_HEIGHT 24
#define PALETTE_WIDTH 150
#define MARGIN 10
#define PADDING 2
#define GRID_SIZE 16
#define PIXEL_SIZE 32
#define PALLETE_SIZE 64

// Structure to hold color palette data
typedef struct {
  Color colors[MAX_COLORS];     // Array of colors in the palette
  int count;                    // Number of colors in the palette
  char name[MAX_PALETTE_NAME];  // Name of the palette
} Palette;

// Runtime paths (local repo by default, overridden for installed runs)
static char libraryDir[512] = "library";
static char fontPath[512] = "fonts/PressStart2P-Regular.ttf";
static char palettesDir[512] = "palettes";

// Global variables for palettes and UI state
Palette palettes[MAX_PALETTES];  // Array of palettes
int paletteCount = 0;            // Current number of loaded palettes
int currentPaletteIndex = 6;     // Index of the currently selected palette
PixelUiLogic uiState = {0};
char textInput[256] = { 0 };

// Rectangle for dropdown menu bounds
Rectangle dropdownBounds;

// Canvas grid for pixel drawing
Color canvas[GRID_SIZE][GRID_SIZE];  // 2D array for pixel colors
Color currentColor;                  // Currently selected color

// Origin coordinates for the grid
int gridOriginX, gridOriginY;

// Buffer for concatenated palette names for dropdown
char dropdownBuffer[1024] = {0};

//----------------------------------------------------------------------------------
// Functions Declaration
//----------------------------------------------------------------------------------
void ShowTextInputBox(bool *showBox, const char *title, void (*callback)(const char *));
static void btnSaveAsPNG(const char *);
static void btnSaveText(const char *filename);
static void btnLoadText(const char *filename);
static void NewCanvas();
static void InitRuntimePaths(void);
static void InitUserLibraryDir(void);

void LoadPalettesFromDir(const char *dirPath);
void DropdownBufferString();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  const int gridPixels = GRID_SIZE * PIXEL_SIZE;
  const int screenWidth = gridPixels + PALETTE_WIDTH + 3 * MARGIN;
  const int screenHeight = gridPixels + TOP_BAR_HEIGHT + BOTTOM_BAR_HEIGHT + 2 * MARGIN;

  InitWindow(screenWidth, screenHeight, "Raylib Pixel Editor");
  SetExitKey(KEY_NULL);
  SetTargetFPS(60);

  InitRuntimePaths();

  Font uiFont = LoadFont(fontPath);

  // Load palettes from directory
  LoadPalettesFromDir(palettesDir);
  if (paletteCount == 0) {
    TraceLog(LOG_WARNING, "No palettes found.");
    CloseWindow();
    return 1;
  }

  // create 'library' folder for user saving
  MakeDirectory(libraryDir);

  // Set initial color and grid origin
  currentColor = palettes[0].colors[0];
  gridOriginX = MARGIN;
  gridOriginY = TOP_BAR_HEIGHT + MARGIN;

  // Define the grid rectangle for drawing
  Rectangle gridBounds = {
      gridOriginX, gridOriginY,
      GRID_SIZE * PIXEL_SIZE,  // Width of the grid
      GRID_SIZE * PIXEL_SIZE   // Height of the grid
  };

  // Initialize the canvas with blank colors
  NewCanvas();
  PixelUiLogicInit(&uiState);

  // Create a string for the dropdown containing palette names
  DropdownBufferString();

  int dropdownActive = 0;
  int selectedPaletteIndex = 6;
  int toggleThemeSliderActive = 0;
  int prevToggleThemeSliderActive = 1;
  int brushSize = 1;
  bool drawingStrokeActive = false;

  while (!WindowShouldClose()) {
    bool suppressUiActionsThisFrame = false;

    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Q)) {
      PixelUiLogicOpenQuitConfirm(&uiState);
    }

    dropdownBounds = (Rectangle){gridOriginX + GRID_SIZE * PIXEL_SIZE + MARGIN, 5, PALLETE_SIZE * 2 + MARGIN, 30};

    // Handle mouse
    Vector2 mouse = GetMousePosition();
    int gx = (mouse.x - gridOriginX) / PIXEL_SIZE;
    int gy = (mouse.y - gridOriginY) / PIXEL_SIZE;
    float wheel = GetMouseWheelMove();
    if (wheel > 0.0f) brushSize++;
    else if (wheel < 0.0f) brushSize--;
    if (brushSize < 1) brushSize = 1;
    if (brushSize > GRID_SIZE) brushSize = GRID_SIZE;

    // ─────────── Logic ─────────────
    if (!uiState.showQuitConfirm && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(mouse, gridBounds) &&
        !uiState.showSavePngDialog && !uiState.showSaveTxtDialog && !uiState.showLoadTxtDialog) {
      drawingStrokeActive = true;
    }

    if (drawingStrokeActive && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      drawingStrokeActive = false;
      suppressUiActionsThisFrame = true;
    }

    if (!uiState.showQuitConfirm && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      if (drawingStrokeActive) {
        if (CheckCollisionPointRec(mouse, gridBounds)) {
          PixelPaintBrush(&canvas[0][0], GRID_SIZE, gx, gy, currentColor, brushSize);
        }
      } else if (CheckCollisionPointRec(mouse, dropdownBounds)) {
        selectedPaletteIndex = !selectedPaletteIndex;  // Toggle dropdown

        // Set the canvas color at the calculated grid position
      } else if (CheckCollisionPointRec(mouse, gridBounds) && !uiState.showSavePngDialog && !uiState.showSaveTxtDialog && !uiState.showLoadTxtDialog) {
        PixelPaintBrush(&canvas[0][0], GRID_SIZE, gx, gy, currentColor, brushSize);

        // Set the palette color at the calculated palette position
      } else {
        int px = gridOriginX + GRID_SIZE * PIXEL_SIZE + MARGIN;
        int count = palettes[currentPaletteIndex].count;
        int maxPerColumn = 8;  // Maximum number of items per column

        for (int i = 0; i < count; i++) {
          // Calculate the column and row based on the index
          int column = i / maxPerColumn;  // 0 for first column, 1 for second column, etc.
          int row = i % maxPerColumn;     // Row within the column

          // Define rectangle for the color
          Rectangle colRect = {px + column * (PALLETE_SIZE + MARGIN),
                               gridOriginY + row * PALLETE_SIZE, PALLETE_SIZE, PALLETE_SIZE};

          // Check if the mouse is over the color rectangle
          if (CheckCollisionPointRec(mouse, colRect)) {
            currentColor = palettes[currentPaletteIndex].colors[i];
          }
        }
      }
    } else if (!uiState.showQuitConfirm && IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && !GuiIsLocked()) {
      // Clear pixel on right-click if within bounds
      if (CheckCollisionPointRec(mouse, gridBounds)) PixelPaintBrush(&canvas[0][0], GRID_SIZE, gx, gy, BLANK, brushSize);
    }

    // ─────────── Drawing UI ─────────────
    // GuiLoadStyleDefault();
    BeginDrawing();
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    // ==== Top bar ====
    if (!uiState.showQuitConfirm &&
        GuiButton((Rectangle){ 10, 5, 100, 30 }, GuiIconText(ICON_FILE_SAVE, "Save as PNG")) &&
        !drawingStrokeActive && !suppressUiActionsThisFrame) {
      PixelUiLogicOpenDialog(&uiState, PIXEL_DIALOG_SAVE_PNG);
    }

    if (!uiState.showQuitConfirm &&
        GuiButton((Rectangle){ 120, 5, 100, 30 }, GuiIconText(ICON_FILE_EXPORT, "Save as TXT")) &&
        !drawingStrokeActive && !suppressUiActionsThisFrame) {
      PixelUiLogicOpenDialog(&uiState, PIXEL_DIALOG_SAVE_TXT);
    }

    if (!uiState.showQuitConfirm &&
        GuiButton((Rectangle){ 230, 5, 100, 30 }, GuiIconText(ICON_FILE_OPEN, "Load TXT")) &&
        !drawingStrokeActive && !suppressUiActionsThisFrame) {
      PixelUiLogicOpenDialog(&uiState, PIXEL_DIALOG_LOAD_TXT);
    }

    if (!uiState.showQuitConfirm &&
        GuiButton((Rectangle){ 340, 5, 100, 30 }, GuiIconText(ICON_RUBBER, "New Canvas")) &&
        !drawingStrokeActive && !suppressUiActionsThisFrame) NewCanvas();

    // Light / Dark Slider
    GuiSetStyle(SLIDER, SLIDER_PADDING, 2);
    GuiToggleSlider((Rectangle){ 450, 5, 60, 30 }, "#142#;#142#", &toggleThemeSliderActive);
    GuiSetStyle(SLIDER, SLIDER_PADDING, 0);

    // Grid
    for (int y = 0; y < GRID_SIZE; y++) {
      for (int x = 0; x < GRID_SIZE; x++) {
        Color col = (canvas[y][x].a == 0) ? GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)) : canvas[y][x];
        DrawRectangle(gridOriginX + x * PIXEL_SIZE, gridOriginY + y * PIXEL_SIZE, PIXEL_SIZE,
                      PIXEL_SIZE, col);
        DrawRectangleLines(gridOriginX + x * PIXEL_SIZE, gridOriginY + y * PIXEL_SIZE, PIXEL_SIZE,
                           PIXEL_SIZE, GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));
      }
    }

    // Palette
    int paletteX = gridOriginX + GRID_SIZE * PIXEL_SIZE + MARGIN;
    int count = palettes[currentPaletteIndex].count;
    int maxPerColumn = 8;  // Maximum number of items per column
    for (int i = 0; i < count; i++) {
      // Calculate the column and row based on the index
      int column = i / maxPerColumn;  // 0 for first column, 1 for second column, etc.
      int row = i % maxPerColumn;     // Row within the column
      // Calculate the x and y positions
      int xPosition = paletteX + column * (PALLETE_SIZE + MARGIN);  // Adjust x for each column
      int yPosition = gridOriginY + row * PALLETE_SIZE;             // Y position based on row
      // 1st option draw full rectangle (white palette colors not visible in light theme )
      // DrawRectangle(xPosition, yPosition, PALLETE_SIZE, PALLETE_SIZE,
      //               palettes[currentPaletteIndex].colors[i]);
      // 2nd option - add paddings to rectangle and draw lines to get better contrast (though I do like it as much)
      Rectangle recLines = {xPosition + PADDING, yPosition + PADDING, PALLETE_SIZE - PADDING * 2, PALLETE_SIZE - PADDING * 2};
      DrawRectanglePro(recLines, (Vector2){0,0}, 0.0f, palettes[currentPaletteIndex].colors[i]);
      DrawRectangleLinesEx(recLines, 1.0f, GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));
    }

    // Draw dropdown for palette selection using Raygui
    if (!uiState.showQuitConfirm &&
        GuiDropdownBox(dropdownBounds, dropdownBuffer, &selectedPaletteIndex, dropdownActive) &&
        !drawingStrokeActive && !suppressUiActionsThisFrame) {
      dropdownActive = !dropdownActive;                        // Toggle dropdown state
      currentPaletteIndex = selectedPaletteIndex;              // Update the current palette index
      currentColor = palettes[currentPaletteIndex].colors[0];  // Set the current color to the first
                                                               // color of the selected palette
    }

    if (!uiState.showQuitConfirm && uiState.showSavePngDialog) {
        ShowTextInputBox(&uiState.showSavePngDialog, "Save file as PNG", btnSaveAsPNG);
    } else if (!uiState.showQuitConfirm && uiState.showSaveTxtDialog) {
        ShowTextInputBox(&uiState.showSaveTxtDialog, "Save file as TXT", btnSaveText);
    } else if (!uiState.showQuitConfirm && uiState.showLoadTxtDialog) {
        ShowTextInputBox(&uiState.showLoadTxtDialog, "Load TXT file", btnLoadText);
    }

    // Bottom status bar
    DrawRectangle(0, screenHeight - BOTTOM_BAR_HEIGHT, screenWidth, BOTTOM_BAR_HEIGHT, LIGHTGRAY);
    DrawTextEx(uiFont,
               TextFormat("Palette: %s | Color: #%02X%02X%02X | Brush: %d", palettes[currentPaletteIndex].name,
                          currentColor.r, currentColor.g, currentColor.b, brushSize),
               (Vector2){10, screenHeight - BOTTOM_BAR_HEIGHT + 8}, uiFont.baseSize * 0.26f, 1,
               BLACK);
    const char *quitHint = "Quit: Ctrl+Q";
    DrawTextEx(uiFont, quitHint,
               (Vector2){screenWidth - MeasureTextEx(uiFont, quitHint, uiFont.baseSize * 0.26f, 1).x - 10,
                         screenHeight - BOTTOM_BAR_HEIGHT + 8},
               uiFont.baseSize * 0.26f, 1, DARKGRAY);

    // Switch Light / Dark
    if (toggleThemeSliderActive != prevToggleThemeSliderActive)
    {
      if (toggleThemeSliderActive) GuiLoadStyleDark(); else GuiLoadStyleDefault();
      prevToggleThemeSliderActive = toggleThemeSliderActive;
    }

    if (uiState.showQuitConfirm) {
      DrawRectangle(0, 0, screenWidth, screenHeight, Fade(DARKGRAY, 0.8f));
      Rectangle bounds = { (float)screenWidth/2 - 140, (float)screenHeight/2 - 70, 280, 140 };
      Rectangle yesBounds = { bounds.x + 16, bounds.y + bounds.height - 38, (bounds.width - 48)/2, 24 };
      Rectangle noBounds = { yesBounds.x + yesBounds.width + 16, yesBounds.y, yesBounds.width, 24 };

      if (GuiWindowBox(bounds, "#119#Confirm Quit")) PixelUiLogicCancelQuit(&uiState);
      GuiLabel((Rectangle){ bounds.x + 16, bounds.y + 48, bounds.width - 32, 20 }, "Quit pixel editor?");

      if (GuiButton(yesBounds, "Yes") || IsKeyPressed(KEY_ENTER)) {
        PixelUiLogicAcceptQuit(&uiState);
      }
      if (GuiButton(noBounds, "No") || IsKeyPressed(KEY_ESCAPE)) {
        PixelUiLogicCancelQuit(&uiState);
      }
    }

    EndDrawing();
    if (uiState.shouldQuit) break;
  }

  UnloadFont(uiFont);
  CloseWindow();
  return 0;
}

//------------------------------------------------------------------------------------
// Controls Functions Definitions
//------------------------------------------------------------------------------------
// Render filename modal and dispatch callback on confirm.
// Side effects: mutates UI dialog state and input buffer.
void ShowTextInputBox(bool *showBox, const char *title, void (*callback)(const char *)) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(DARKGRAY, 0.8f));
    Rectangle bounds = { (float)GetScreenWidth()/2 - 120, (float)GetScreenHeight()/2 - 60, 240, 140 };
    Rectangle textBoxBounds = { bounds.x + 12, bounds.y + 56, bounds.width - 24, 26 };
    Rectangle okBounds = { bounds.x + 12, bounds.y + bounds.height - 36, (bounds.width - 36)/2, 24 };
    Rectangle cancelBounds = { okBounds.x + okBounds.width + 12, okBounds.y, okBounds.width, 24 };

    if (GuiWindowBox(bounds, GuiIconText(ICON_FILE_SAVE, title))) {
        *showBox = false;
        uiState.textInputEditMode = false;
        TextCopy(textInput, "\0");
        return;
    }

    GuiLabel((Rectangle){ bounds.x + 12, bounds.y + 32, bounds.width - 24, 20 }, "Specify file name:");
    if (GuiTextBox(textBoxBounds, textInput, 255, uiState.textInputEditMode)) uiState.textInputEditMode = !uiState.textInputEditMode;

    if (GuiButton(okBounds, "Ok") || IsKeyPressed(KEY_ENTER)) {
        callback(textInput);
        *showBox = false;
        uiState.textInputEditMode = false;
        TextCopy(textInput, "\0");
        return;
    }

    if (GuiButton(cancelBounds, "Cancel") || IsKeyPressed(KEY_ESCAPE)) {
        *showBox = false;
        uiState.textInputEditMode = false;
        TextCopy(textInput, "\0");
    }
}

// Export current canvas to PNG and keep matching project text file.
static void btnSaveAsPNG(const char * textInput) {
  char pngPath[1024];
  if (!PixelBuildFilePath(libraryDir, textInput, ".png", pngPath, sizeof(pngPath))) return;

  Image image = GenImageColor(GRID_SIZE, GRID_SIZE, BLANK);
  for (int y = 0; y < GRID_SIZE; y++)
    for (int x = 0; x < GRID_SIZE; x++) ImageDrawPixel(&image, x, y, canvas[y][x]);
  ExportImage(image, pngPath);
  UnloadImage(image);

  // Keep a loadable project snapshot alongside every PNG export.
  btnSaveText(textInput);
}

// Save current canvas in reloadable text format.
static void btnSaveText(const char *filename) {
    char newFilename[1024];
    if (!PixelBuildFilePath(libraryDir, filename, ".txt", newFilename, sizeof(newFilename))) return;
    if (!PixelSaveCanvasText(newFilename, &canvas[0][0], GRID_SIZE)) {
      TraceLog(LOG_ERROR, "Error saving file: %s", newFilename);
    }
}

// Load canvas from text project format.
static void btnLoadText(const char *filename) {
    char newFilename[1024];
    if (!PixelBuildFilePath(libraryDir, filename, ".txt", newFilename, sizeof(newFilename))) return;
    if (!FileExists(newFilename)) {
        TraceLog(LOG_ERROR, "Could not load file: %s", filename);
        return;
    }

    // Reset the canvas to transparent values
    NewCanvas();

    if (!PixelLoadCanvasText(newFilename, &canvas[0][0], GRID_SIZE)) {
      TraceLog(LOG_ERROR, "Could not parse file: %s", newFilename);
    }
}

// Reset all canvas cells to transparent.
static void NewCanvas() {
  for (int y = 0; y < GRID_SIZE; y++) {
      for (int x = 0; x < GRID_SIZE; x++)
          canvas[y][x] = BLANK;
  }
}

// Resolve runtime asset and user data paths for the active platform/layout.
static void InitRuntimePaths(void) {
  // Development mode defaults (assets from repository root)
  TextCopy(fontPath, "fonts/PressStart2P-Regular.ttf");
  TextCopy(palettesDir, "palettes");

  // Installed layout fallback: /usr/bin/pixel -> /usr/share/pixel/{fonts,palettes}
  if (!FileExists(fontPath) || !DirectoryExists(palettesDir)) {
    const char *appDir = GetApplicationDirectory();
    char shareDir[512] = {0};
    if (appDir && appDir[0] != '\0') {
      snprintf(shareDir, sizeof(shareDir), "%s../share/pixel", appDir);
      snprintf(fontPath, sizeof(fontPath), "%s/fonts/PressStart2P-Regular.ttf", shareDir);
      snprintf(palettesDir, sizeof(palettesDir), "%s/palettes", shareDir);
    }
  }

  InitUserLibraryDir();
}

// Initialize user-writable library directory on Linux/Windows with fallbacks.
static void InitUserLibraryDir(void) {
#if defined(_WIN32)
  const char *base = getenv("LOCALAPPDATA");
  if (!base || base[0] == '\0') base = getenv("APPDATA");
  if (base && base[0] != '\0') {
    char appDir[512] = {0};
    snprintf(appDir, sizeof(appDir), "%s/pixel", base);
    MakeDirectory(appDir);
    snprintf(libraryDir, sizeof(libraryDir), "%s/library", appDir);
    MakeDirectory(libraryDir);
    return;
  }
#else
  const char *xdgData = getenv("XDG_DATA_HOME");
  if (xdgData && xdgData[0] != '\0') {
    char appDir[512] = {0};
    MakeDirectory(xdgData);
    snprintf(appDir, sizeof(appDir), "%s/pixel", xdgData);
    MakeDirectory(appDir);
    snprintf(libraryDir, sizeof(libraryDir), "%s/library", appDir);
    MakeDirectory(libraryDir);
    return;
  }

  const char *home = getenv("HOME");
  if (home && home[0] != '\0') {
    char path[512] = {0};
    snprintf(path, sizeof(path), "%s/.local", home);
    MakeDirectory(path);
    snprintf(path, sizeof(path), "%s/.local/share", home);
    MakeDirectory(path);
    snprintf(path, sizeof(path), "%s/.local/share/pixel", home);
    MakeDirectory(path);
    snprintf(libraryDir, sizeof(libraryDir), "%s/.local/share/pixel/library", home);
    MakeDirectory(libraryDir);
    return;
  }
#endif

  // Last-resort fallback if no platform env path is available.
  TextCopy(libraryDir, "library");
  MakeDirectory(libraryDir);
}

//------------------------------------------------------------------------------------
// Helper Functions Definitions
//------------------------------------------------------------------------------------
// Load palette files from directory into in-memory palette list.
void LoadPalettesFromDir(const char *dirPath) {
  FilePathList files = LoadDirectoryFiles(dirPath);
  for (unsigned int i = 0; i < files.count && paletteCount < MAX_PALETTES; i++) {
    if (!IsPathFile(files.paths[i])) continue;

    FILE *fp = fopen(files.paths[i], "r");
    if (!fp) continue;

    Palette p = {0};
    strncpy(p.name, GetFileNameWithoutExt(files.paths[i]), MAX_PALETTE_NAME - 1);
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
      if (line[0] == ';' || strlen(line) < 8) continue;
      unsigned int r, g, b;
      if (sscanf(line, "FF%02x%02x%02x", &r, &g, &b) == 3 && p.count < MAX_COLORS) {
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

// Build semicolon-separated palette list expected by raygui dropdown.
void DropdownBufferString() {
  size_t currentLength = strlen(dropdownBuffer);  // Start with the current length (initially 0)

  for (int i = 0; i < paletteCount; i++) {
    // Format the new item
    char item[256];  // Temporary buffer for the item
    snprintf(item, sizeof(item), "%s",
             palettes[i].name);  // Create the item string

    // Calculate the length of the new item
    size_t itemLength = strlen(item);

    // Check if adding this item would exceed the buffer size
    if (currentLength + itemLength + (size_t)(i > 0 ? 1 : 0) < sizeof(dropdownBuffer)) {
      if (i > 0) {
        strncat(dropdownBuffer, ";",
                sizeof(dropdownBuffer) - currentLength - 1);  // Add separator
        currentLength += 1;  // Update the current length for the separator
      }
      strncat(dropdownBuffer, item,
              sizeof(dropdownBuffer) - currentLength - 1);  // Append the item
      currentLength += itemLength;                          // Update the current length
    } else {
      // Handle the case where the buffer is too small
      TraceLog(LOG_WARNING, "Dropdown buffer size exceeded. Some palettes may not be displayed.");
      break;  // Exit the loop if the buffer is full
    }
  }
}
