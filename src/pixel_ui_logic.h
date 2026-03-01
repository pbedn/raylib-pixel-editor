#ifndef PIXEL_UI_LOGIC_H
#define PIXEL_UI_LOGIC_H

#include <stdbool.h>

typedef enum {
  PIXEL_DIALOG_NONE = 0,
  PIXEL_DIALOG_SAVE_PNG,
  PIXEL_DIALOG_SAVE_TXT,
  PIXEL_DIALOG_LOAD_TXT
} PixelDialogType;

typedef struct {
  bool showSavePngDialog;
  bool showSaveTxtDialog;
  bool showLoadTxtDialog;
  bool textInputEditMode;
  bool showQuitConfirm;
  bool shouldQuit;
} PixelUiLogic;

void PixelUiLogicInit(PixelUiLogic *ui);
void PixelUiLogicOpenDialog(PixelUiLogic *ui, PixelDialogType dialogType);
void PixelUiLogicOpenQuitConfirm(PixelUiLogic *ui);
void PixelUiLogicCloseDialog(PixelUiLogic *ui, PixelDialogType dialogType);
void PixelUiLogicCancelQuit(PixelUiLogic *ui);
void PixelUiLogicAcceptQuit(PixelUiLogic *ui);

#endif
