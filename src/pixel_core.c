#include "pixel_core.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int PixelIndex(int x, int y, int gridSize) {
  return y * gridSize + x;
}

static const char *SkipSpaces(const char *s) {
  while (*s != '\0' && isspace((unsigned char)*s)) s++;
  return s;
}

// Normalize user input to a safe basename without extension or trailing spaces.
bool PixelNormalizeBaseName(const char *input, char *out, size_t outSize) {
  if (!input || !out || outSize == 0) return false;

  const char *start = SkipSpaces(input);
  if (*start == '\0' || *start == '.') return false;

  size_t i = 0;
  while (start[i] != '\0' && start[i] != '.' && !isspace((unsigned char)start[i])) {
    if (i + 1 >= outSize) return false;
    out[i] = start[i];
    i++;
  }

  if (i == 0) return false;
  out[i] = '\0';
  return true;
}

// Build "<dir>/<normalized basename><ext>" and return false on invalid input.
bool PixelBuildFilePath(const char *dir, const char *input, const char *ext, char *out, size_t outSize) {
  if (!dir || !input || !ext || !out || outSize == 0) return false;

  char base[256] = {0};
  if (!PixelNormalizeBaseName(input, base, sizeof(base))) return false;

  int written = snprintf(out, outSize, "%s/%s%s", dir, base, ext);
  return written > 0 && (size_t)written < outSize;
}

// Paint square brush area centered on grid cell and clamp to canvas bounds.
void PixelPaintBrush(Color *canvas, int gridSize, int gx, int gy, Color color, int brushSize) {
  if (!canvas || gridSize <= 0 || brushSize <= 0) return;

  int startX = gx - brushSize / 2;
  int startY = gy - brushSize / 2;

  for (int y = 0; y < brushSize; y++) {
    for (int x = 0; x < brushSize; x++) {
      int px = startX + x;
      int py = startY + y;
      if (px < 0 || px >= gridSize || py < 0 || py >= gridSize) continue;
      canvas[PixelIndex(px, py, gridSize)] = color;
    }
  }
}

// Save canvas as row-based text format that can be reloaded robustly.
bool PixelSaveCanvasText(const char *path, const Color *canvas, int gridSize) {
  if (!path || !canvas || gridSize <= 0) return false;

  FILE *fp = fopen(path, "w");
  if (!fp) return false;

  fprintf(fp, "Canvas Data (GRID_SIZE: %d)\n", gridSize);
  fprintf(fp, "# Format: r,g,b,a\n\n");

  for (int y = 0; y < gridSize; y++) {
    fprintf(fp, "Row %03d: ", y);
    for (int x = 0; x < gridSize; x++) {
      Color c = canvas[PixelIndex(x, y, gridSize)];
      fprintf(fp, "%03d,%03d,%03d,%03d", c.r, c.g, c.b, c.a);
      if (x < gridSize - 1) fprintf(fp, " | ");
    }
    fputc('\n', fp);
  }

  return fclose(fp) == 0;
}

// Parse one "Row NNN:" line into canvas row, ignoring malformed values.
static void PixelParseLine(char *line, Color *canvas, int gridSize) {
  int rowIndex = -1;
  if (sscanf(line, "Row %d:", &rowIndex) != 1) return;
  if (rowIndex < 0 || rowIndex >= gridSize) return;

  char *dataPart = strchr(line, ':');
  if (!dataPart) return;
  dataPart++;
  while (*dataPart != '\0' && isspace((unsigned char)*dataPart)) dataPart++;

  char *token = strtok(dataPart, "|");
  for (int x = 0; x < gridSize && token != NULL; x++) {
    token = (char *)SkipSpaces(token);

    int r, g, b, a;
    if (sscanf(token, "%3d,%3d,%3d,%3d", &r, &g, &b, &a) == 4 &&
        r >= 0 && r <= 255 && g >= 0 && g <= 255 &&
        b >= 0 && b <= 255 && a >= 0 && a <= 255) {
      canvas[PixelIndex(x, rowIndex, gridSize)] =
          (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
    }
    token = strtok(NULL, "|");
  }
}

// Load canvas text format by row labels, independent of file line ordering.
bool PixelLoadCanvasText(const char *path, Color *canvas, int gridSize) {
  if (!path || !canvas || gridSize <= 0) return false;

  FILE *fp = fopen(path, "r");
  if (!fp) return false;

  char line[8192];
  while (fgets(line, sizeof(line), fp)) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      line[len - 1] = '\0';
      len--;
    }
    PixelParseLine(line, canvas, gridSize);
  }

  return fclose(fp) == 0;
}
