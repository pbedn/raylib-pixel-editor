#include <stdio.h>
#include <string.h>

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION  // Define this in one source file
#include "raygui.h"

// Define the maximum number of colors and palettes
#define MAX_COLORS 64
#define MAX_PALETTES 16
#define MAX_PALETTE_NAME 64

// Define UI dimensions
#define TOP_BAR_HEIGHT 30
#define BOTTOM_BAR_HEIGHT 24
#define PALETTE_WIDTH 150
#define MARGIN 10
#define GRID_SIZE 16
#define PIXEL_SIZE 32
#define PALLETE_SIZE 64

// Structure to hold color palette data
typedef struct {
  Color colors[MAX_COLORS];     // Array of colors in the palette
  int count;                    // Number of colors in the palette
  char name[MAX_PALETTE_NAME];  // Name of the palette
} Palette;

// Global variables for palettes and UI state
Palette palettes[MAX_PALETTES];  // Array of palettes
int paletteCount = 0;            // Current number of loaded palettes
int currentPaletteIndex = 6;     // Index of the currently selected palette
int dropdownActive = 0;          // State of the dropdown menu
bool showTextInputBox = false;
bool showTextInputBox2 = false;
bool showTextInputBox3 = false;
char textInput[256] = { 0 };
char textBoxText[64] = "filename";
bool textBoxEditMode = false;

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
static void parseLine(const char *line, int rowIndex);

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
  SetTargetFPS(60);

  Font uiFont = LoadFont("fonts/PressStart2P-Regular.ttf");

  // Load palettes from directory
  LoadPalettesFromDir("palettes");
  if (paletteCount == 0) {
    TraceLog(LOG_WARNING, "No palettes found.");
    CloseWindow();
    return 1;
  }

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
  for (int y = 0; y < GRID_SIZE; y++)
    for (int x = 0; x < GRID_SIZE; x++) canvas[y][x] = BLANK;

  // Create a string for the dropdown containing palette names
  DropdownBufferString();

  int dropdownActive = 0;
  int selectedPaletteIndex = 6;

  while (!WindowShouldClose()) {
    // Handle mouse
    Vector2 mouse = GetMousePosition();
    int gx = (mouse.x - gridOriginX) / PIXEL_SIZE;
    int gy = (mouse.y - gridOriginY) / PIXEL_SIZE;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      if (CheckCollisionPointRec(mouse, dropdownBounds)) {
        selectedPaletteIndex = !selectedPaletteIndex;  // Toggle dropdown

        // Set the canvas color at the calculated grid position
      } else if (CheckCollisionPointRec(mouse, gridBounds) && !showTextInputBox && !showTextInputBox2 && !showTextInputBox3) {
        canvas[gy][gx] = currentColor;

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
    } else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && !GuiIsLocked()) {
      // Clear pixel on right-click if within bounds
      if (CheckCollisionPointRec(mouse, gridBounds)) canvas[gy][gx] = BLANK;
    }

    // ─────────── Drawing UI ─────────────
    GuiLoadStyleDefault();
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Top bar
    if (GuiButton((Rectangle){ 10, 5, 100, 30 }, GuiIconText(ICON_FILE_SAVE, "Save as PNG")) || IsKeyPressed(KEY_S)) showTextInputBox = true;

    if (GuiButton((Rectangle){ 120, 5, 100, 30 }, GuiIconText(ICON_FILE_EXPORT, "Save as TXT")) || IsKeyPressed(KEY_B)) showTextInputBox2 = true;

    if (GuiButton((Rectangle){ 230, 5, 100, 30 }, GuiIconText(ICON_FILE_OPEN, "Load TXT")) || IsKeyPressed(KEY_L)) showTextInputBox3 = true;

    // Grid
    for (int y = 0; y < GRID_SIZE; y++) {
      for (int x = 0; x < GRID_SIZE; x++) {
        Color col = (canvas[y][x].a == 0) ? DARKGRAY : canvas[y][x];
        DrawRectangle(gridOriginX + x * PIXEL_SIZE, gridOriginY + y * PIXEL_SIZE, PIXEL_SIZE,
                      PIXEL_SIZE, col);
        DrawRectangleLines(gridOriginX + x * PIXEL_SIZE, gridOriginY + y * PIXEL_SIZE, PIXEL_SIZE,
                           PIXEL_SIZE, GRAY);
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
      DrawRectangle(xPosition, yPosition, PALLETE_SIZE, PALLETE_SIZE,
                    palettes[currentPaletteIndex].colors[i]);
    }

    // Draw dropdown for palette selection using Raygui
    if (GuiDropdownBox((Rectangle){paletteX, 5, PALLETE_SIZE * 2 + MARGIN, 30}, dropdownBuffer,
                       &selectedPaletteIndex, dropdownActive)) {
      dropdownActive = !dropdownActive;                        // Toggle dropdown state
      currentPaletteIndex = selectedPaletteIndex;              // Update the current palette index
      currentColor = palettes[currentPaletteIndex].colors[0];  // Set the current color to the first
                                                               // color of the selected palette
    }

    if (showTextInputBox) {
        ShowTextInputBox(&showTextInputBox, "Save file as PNG", btnSaveAsPNG);
    } else if (showTextInputBox2) {
        ShowTextInputBox(&showTextInputBox2, "Save file as TXT", btnSaveText);
    } else if (showTextInputBox3) {
        ShowTextInputBox(&showTextInputBox3, "Load TXT file", btnLoadText);
    }

    // Bottom status bar
    DrawRectangle(0, screenHeight - BOTTOM_BAR_HEIGHT, screenWidth, BOTTOM_BAR_HEIGHT, LIGHTGRAY);
    DrawTextEx(uiFont,
               TextFormat("Palette: %s | Color: #%02X%02X%02X", palettes[currentPaletteIndex].name,
                          currentColor.r, currentColor.g, currentColor.b),
               (Vector2){10, screenHeight - BOTTOM_BAR_HEIGHT + 8}, uiFont.baseSize * 0.26f, 1,
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
void ShowTextInputBox(bool *showBox, const char *title, void (*callback)(const char *)) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(DARKGRAY, 0.8f));
    int result = GuiTextInputBox(
        (Rectangle){ (float)GetScreenWidth()/2 - 120, (float)GetScreenHeight()/2 - 60, 240, 140 },
        GuiIconText(ICON_FILE_SAVE, title), "Specify file name:", "Ok;Cancel", textInput, 255, NULL);

    if (result == 1) {
        callback(textInput);
    }

    if (result == 0 || result == 1 || result == 2) {
        *showBox = false;
        TextCopy(textInput, "\0");
    }
}

static void btnSaveAsPNG(const char * textInput) {
  Image image = GenImageColor(GRID_SIZE, GRID_SIZE, BLANK);
  for (int y = 0; y < GRID_SIZE; y++)
    for (int x = 0; x < GRID_SIZE; x++) ImageDrawPixel(&image, x, y, canvas[y][x]);
  char filename[256];
  sprintf(filename, "%s.png", textInput);
  ExportImage(image, filename);
  UnloadImage(image);
}

static void btnSaveText(const char *filename) {
    // Create a buffer to hold the text data
    char *textBuffer = (char *)malloc(GRID_SIZE * GRID_SIZE * 30); // Allocate enough space
    if (textBuffer == NULL) {
        TraceLog(LOG_ERROR, "Memory allocation failed.\n");
        return;
    }

    // Fill the buffer with canvas data
    int offset = 0;
    offset += sprintf(textBuffer + offset, "Canvas Data (GRID_SIZE: %d)\n", GRID_SIZE);
    offset += sprintf(textBuffer + offset, "# Format: r,g,b,a\n\n"); // Add a comment about the format

    for (int y = 0; y < GRID_SIZE; y++) {
        offset += sprintf(textBuffer + offset, "Row %03d: ", y); // Indicate the row number
        for (int x = 0; x < GRID_SIZE; x++) {
            // Format: r,g,b,a with zero padding
            offset += sprintf(textBuffer + offset, "%03d,%03d,%03d,%03d", canvas[y][x].r, canvas[y][x].g, canvas[y][x].b, canvas[y][x].a);
            if (x < GRID_SIZE - 1) {
                offset += sprintf(textBuffer + offset, " | "); // Use a separator between colors
            }
        }
        offset += sprintf(textBuffer + offset, "\n"); // Newline after each row
    }

    // Save the text data to a file
    char newFilename[256];
    sprintf(newFilename, "%s.bin", filename);
    if (!SaveFileText(newFilename, textBuffer)) {
        TraceLog(LOG_ERROR, "Error saving file.\n");
    }

    free(textBuffer);
}

static void btnLoadText(const char *filename) {
    // Load the text data from the file
    char newFilename[256];
    sprintf(newFilename, "%s.bin", filename);
    char *textData = LoadFileText(newFilename);
    if (textData == NULL) {
        TraceLog(LOG_ERROR, "Could not load file: %s", filename);
        return;
    }

    // Reset the canvas to transparent values
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            canvas[y][x] = BLANK;
        }
    }
    // Parse the text data
    char *lineStart = textData;
    int lineCount = 0;

    while (*lineStart != '\0') {
        // Find the end of the line
        char *lineEnd = strchr(lineStart, '\n');
        if (lineEnd == NULL) lineEnd = lineStart + strlen(lineStart); // Handle last line

        // Null-terminate the line
        char temp = *lineEnd;
        *lineEnd = '\0';

        // Skip header and comment lines
        if (lineCount >= 2) { // Start processing from the third line (index 2)
            // TraceLog(LOG_INFO, "Processing line: %s", lineStart);
            parseLine(lineStart, lineCount - 3); // Adjust for skipped lines
        }

        // Restore the original character
        *lineEnd = temp;

        // Move to the next line
        lineStart = (*lineEnd == '\n') ? lineEnd + 1 : lineEnd; // Skip \n
        lineCount++;
    }
    UnloadFileText(textData);
}

static void parseLine(const char *line, int rowIndex) {
    char *dataPart = strchr(line, ':'); // Find the colon
    if (dataPart != NULL) {
        dataPart += 2; // Move past ": "
        char *colorToken = strtok(dataPart, " | "); // Split by separator

        for (int x = 0; x < GRID_SIZE && colorToken != NULL; x++) {
            int r, g, b, a;
            if (sscanf(colorToken, "%3d,%3d,%3d,%3d", &r, &g, &b, &a) == 4) {
                canvas[rowIndex][x] = (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
                // TraceLog(LOG_DEBUG, "Loaded color at [%d][%d]: R=%d, G=%d, B=%d, A=%d", rowIndex, x, r, g, b, a);
            } else {
                TraceLog(LOG_WARNING, "Error parsing color at [%d][%d]: %s", rowIndex, x, colorToken);
            }
            colorToken = strtok(NULL, " | "); // Get the next color token
        }
    } else {
        TraceLog(LOG_WARNING, "No data found for Row %03d", rowIndex);
    }
}

//------------------------------------------------------------------------------------
// Helper Functions Definitions
//------------------------------------------------------------------------------------
void LoadPalettesFromDir(const char *dirPath) {
  FilePathList files = LoadDirectoryFiles(dirPath);
  for (int i = 0; i < files.count && paletteCount < MAX_PALETTES; i++) {
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

void DropdownBufferString() {
  int currentLength = strlen(dropdownBuffer);  // Start with the current length (initially 0)

  for (int i = 0; i < paletteCount; i++) {
    // Format the new item
    char item[256];  // Temporary buffer for the item
    snprintf(item, sizeof(item), "%s",
             palettes[i].name);  // Create the item string

    // Calculate the length of the new item
    int itemLength = strlen(item);

    // Check if adding this item would exceed the buffer size
    if (currentLength + itemLength + (i > 0 ? 1 : 0) < sizeof(dropdownBuffer)) {
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