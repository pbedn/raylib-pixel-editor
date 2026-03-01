#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pixel_core.h"
#include "pixel_ui_logic.h"

static int failures = 0;

#define EXPECT_TRUE(expr) do { \
  if (!(expr)) { \
    fprintf(stderr, "FAIL:%s:%d: %s\n", __FILE__, __LINE__, #expr); \
    failures++; \
  } \
} while (0)

static bool ColorEq(Color a, Color b) {
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static Color *AllocCanvas(int grid) {
  return (Color *)calloc((size_t)grid * (size_t)grid, sizeof(Color));
}

static void FillPattern(Color *canvas, int grid) {
  for (int y = 0; y < grid; y++) {
    for (int x = 0; x < grid; x++) {
      canvas[y * grid + x] = (Color){
          (unsigned char)(x * 10 + y),
          (unsigned char)(x + y * 10),
          (unsigned char)(x * y),
          255};
    }
  }
}

static void TestNormalizeBaseName(void) {
  char out[64];
  EXPECT_TRUE(PixelNormalizeBaseName("sprite", out, sizeof(out)));
  EXPECT_TRUE(strcmp(out, "sprite") == 0);

  EXPECT_TRUE(PixelNormalizeBaseName("sprite.txt", out, sizeof(out)));
  EXPECT_TRUE(strcmp(out, "sprite") == 0);

  EXPECT_TRUE(PixelNormalizeBaseName("sprite.bin", out, sizeof(out)));
  EXPECT_TRUE(strcmp(out, "sprite") == 0);

  EXPECT_TRUE(!PixelNormalizeBaseName("", out, sizeof(out)));
  EXPECT_TRUE(!PixelNormalizeBaseName(".txt", out, sizeof(out)));
}

static void TestBuildFilePath(void) {
  char path[256];
  EXPECT_TRUE(PixelBuildFilePath("/tmp/pixel", "sprite", ".txt", path, sizeof(path)));
  EXPECT_TRUE(strcmp(path, "/tmp/pixel/sprite.txt") == 0);

  EXPECT_TRUE(PixelBuildFilePath("/tmp/pixel", "sprite.png", ".txt", path, sizeof(path)));
  EXPECT_TRUE(strcmp(path, "/tmp/pixel/sprite.txt") == 0);

  EXPECT_TRUE(PixelBuildFilePath("/tmp/pixel", "sprite.txt", ".png", path, sizeof(path)));
  EXPECT_TRUE(strcmp(path, "/tmp/pixel/sprite.png") == 0);
}

static void TestPaintBrushClamp(void) {
  int grid = 5;
  Color *canvas = AllocCanvas(grid);
  Color red = {255, 0, 0, 255};
  PixelPaintBrush(canvas, grid, 0, 0, red, 3);

  int painted = 0;
  for (int y = 0; y < grid; y++) {
    for (int x = 0; x < grid; x++) {
      if (ColorEq(canvas[y * grid + x], red)) painted++;
    }
  }
  EXPECT_TRUE(painted == 4);
  free(canvas);
}

static void TestSaveLoadRoundTrip(void) {
  int grid = 16;
  Color *a = AllocCanvas(grid);
  Color *b = AllocCanvas(grid);
  FillPattern(a, grid);

  char path[] = "/tmp/pixel-core-roundtrip-XXXXXX";
  int fd = mkstemp(path);
  EXPECT_TRUE(fd >= 0);
  if (fd >= 0) close(fd);

  EXPECT_TRUE(PixelSaveCanvasText(path, a, grid));
  EXPECT_TRUE(PixelLoadCanvasText(path, b, grid));

  for (int i = 0; i < grid * grid; i++) {
    EXPECT_TRUE(ColorEq(a[i], b[i]));
  }

  unlink(path);
  free(a);
  free(b);
}

static void TestLoadParsesRowsByIndex(void) {
  int grid = 4;
  Color *canvas = AllocCanvas(grid);

  char path[] = "/tmp/pixel-core-rows-XXXXXX";
  int fd = mkstemp(path);
  EXPECT_TRUE(fd >= 0);
  FILE *fp = fdopen(fd, "w");
  EXPECT_TRUE(fp != NULL);
  if (!fp) return;

  fprintf(fp, "Canvas Data (GRID_SIZE: 4)\n");
  fprintf(fp, "# comment\n\n");
  fprintf(fp, "Row 002: 001,002,003,255 | 004,005,006,255 | 007,008,009,255 | 010,011,012,255\n");
  fprintf(fp, "Row 000: 013,014,015,255 | 016,017,018,255 | 019,020,021,255 | 022,023,024,255\n");
  fprintf(fp, "Row 999: 255,255,255,255 | 255,255,255,255 | 255,255,255,255 | 255,255,255,255\n");
  fclose(fp);

  EXPECT_TRUE(PixelLoadCanvasText(path, canvas, grid));
  EXPECT_TRUE(ColorEq(canvas[2 * grid + 0], (Color){1, 2, 3, 255}));
  EXPECT_TRUE(ColorEq(canvas[0 * grid + 0], (Color){13, 14, 15, 255}));
  EXPECT_TRUE(ColorEq(canvas[1 * grid + 0], (Color){0, 0, 0, 0}));

  unlink(path);
  free(canvas);
}

static void TestUiDialogTransitions(void) {
  PixelUiLogic ui;
  PixelUiLogicInit(&ui);
  EXPECT_TRUE(!ui.showSavePngDialog && !ui.showSaveTxtDialog && !ui.showLoadTxtDialog);
  EXPECT_TRUE(!ui.showQuitConfirm);

  PixelUiLogicOpenDialog(&ui, PIXEL_DIALOG_SAVE_PNG);
  EXPECT_TRUE(ui.showSavePngDialog);
  EXPECT_TRUE(!ui.showSaveTxtDialog && !ui.showLoadTxtDialog);
  EXPECT_TRUE(ui.textInputEditMode);

  PixelUiLogicOpenDialog(&ui, PIXEL_DIALOG_LOAD_TXT);
  EXPECT_TRUE(ui.showLoadTxtDialog);
  EXPECT_TRUE(!ui.showSavePngDialog && !ui.showSaveTxtDialog);
  EXPECT_TRUE(ui.textInputEditMode);

  PixelUiLogicOpenQuitConfirm(&ui);
  EXPECT_TRUE(ui.showQuitConfirm);
  EXPECT_TRUE(!ui.showSavePngDialog && !ui.showSaveTxtDialog && !ui.showLoadTxtDialog);
  EXPECT_TRUE(!ui.textInputEditMode);

  PixelUiLogicCancelQuit(&ui);
  EXPECT_TRUE(!ui.showQuitConfirm);

  PixelUiLogicAcceptQuit(&ui);
  EXPECT_TRUE(ui.shouldQuit);
}

int main(void) {
  TestNormalizeBaseName();
  TestBuildFilePath();
  TestPaintBrushClamp();
  TestSaveLoadRoundTrip();
  TestLoadParsesRowsByIndex();
  TestUiDialogTransitions();

  if (failures > 0) {
    fprintf(stderr, "Tests failed: %d\n", failures);
    return 1;
  }

  printf("All tests passed\n");
  return 0;
}
