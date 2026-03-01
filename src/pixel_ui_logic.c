#include "pixel_ui_logic.h"

// Reset all dialog/quit state to defaults.
void PixelUiLogicInit(PixelUiLogic *ui) {
  if (!ui) return;
  *ui = (PixelUiLogic){0};
}

// Open a specific dialog, closing others and focusing text input.
void PixelUiLogicOpenDialog(PixelUiLogic *ui, PixelDialogType dialogType) {
  if (!ui) return;
  ui->showSavePngDialog = false;
  ui->showSaveTxtDialog = false;
  ui->showLoadTxtDialog = false;
  ui->showQuitConfirm = false;
  ui->textInputEditMode = true;

  if (dialogType == PIXEL_DIALOG_SAVE_PNG) ui->showSavePngDialog = true;
  else if (dialogType == PIXEL_DIALOG_SAVE_TXT) ui->showSaveTxtDialog = true;
  else if (dialogType == PIXEL_DIALOG_LOAD_TXT) ui->showLoadTxtDialog = true;
}

// Open quit confirmation and block all text dialogs.
void PixelUiLogicOpenQuitConfirm(PixelUiLogic *ui) {
  if (!ui) return;
  ui->showSavePngDialog = false;
  ui->showSaveTxtDialog = false;
  ui->showLoadTxtDialog = false;
  ui->textInputEditMode = false;
  ui->showQuitConfirm = true;
}

// Close one dialog and leave text edit mode.
void PixelUiLogicCloseDialog(PixelUiLogic *ui, PixelDialogType dialogType) {
  if (!ui) return;
  if (dialogType == PIXEL_DIALOG_SAVE_PNG) ui->showSavePngDialog = false;
  else if (dialogType == PIXEL_DIALOG_SAVE_TXT) ui->showSaveTxtDialog = false;
  else if (dialogType == PIXEL_DIALOG_LOAD_TXT) ui->showLoadTxtDialog = false;
  ui->textInputEditMode = false;
}

// Cancel quit flow and return to normal interaction.
void PixelUiLogicCancelQuit(PixelUiLogic *ui) {
  if (!ui) return;
  ui->showQuitConfirm = false;
}

// Accept quit flow and request app shutdown.
void PixelUiLogicAcceptQuit(PixelUiLogic *ui) {
  if (!ui) return;
  ui->shouldQuit = true;
}
