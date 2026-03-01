#ifndef PIXEL_CORE_H
#define PIXEL_CORE_H

#include <stdbool.h>
#include <stddef.h>

#include "raylib.h"

bool PixelNormalizeBaseName(const char *input, char *out, size_t outSize);
bool PixelBuildFilePath(const char *dir, const char *input, const char *ext, char *out, size_t outSize);
void PixelPaintBrush(Color *canvas, int gridSize, int gx, int gy, Color color, int brushSize);
bool PixelSaveCanvasText(const char *path, const Color *canvas, int gridSize);
bool PixelLoadCanvasText(const char *path, Color *canvas, int gridSize);

#endif
