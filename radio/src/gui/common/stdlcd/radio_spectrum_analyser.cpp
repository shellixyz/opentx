/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"

extern uint8_t g_moduleIdx;

enum SpektrumFields {
  SPEKTRUM_FREQUENCY,
  SPEKTRUM_SPAN,
  SPEKTRUM_FIELDS_MAX
};

void menuRadioSpectrumAnalyser(event_t event)
{
  if (TELEMETRY_STREAMING()) {
    lcdDrawCenteredText(15, "Turn off receiver");
    if (event == EVT_KEY_FIRST(KEY_EXIT)) {
      killEvents(event);
      popMenu();
    }
    return;
  }

  SUBMENU("SPECTRUM ANALYSER", 1, {1});

  if (menuEvent) {
    pausePulses();
    moduleSettings[g_moduleIdx].mode = MODULE_MODE_NORMAL;
    /* wait 500ms off */
    watchdogSuspend(500);
    RTOS_WAIT_MS(500);
    resumePulses();
    /* wait 500ms to resume normal operation before leaving */
    watchdogSuspend(500);
    RTOS_WAIT_MS(500);
    return;
  }

  if (moduleSettings[g_moduleIdx].mode != MODULE_MODE_SPECTRUM_ANALYSER) {
    memclear(reusableBuffer.spectrumAnalyser.bars, sizeof(reusableBuffer.spectrumAnalyser.bars));
    reusableBuffer.spectrumAnalyser.span = 40000000;  // 40MHz
    reusableBuffer.spectrumAnalyser.freq = 2440000000;  // 2440MHz
    reusableBuffer.spectrumAnalyser.step = reusableBuffer.spectrumAnalyser.span / LCD_W;
    moduleSettings[g_moduleIdx].mode = MODULE_MODE_SPECTRUM_ANALYSER;
  }

  for (uint8_t i=0; i<SPEKTRUM_FIELDS_MAX+1; i++) {
    uint8_t blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);

    switch (i) {
      case SPEKTRUM_FREQUENCY: {
        uint16_t frequency = reusableBuffer.spectrumAnalyser.freq / 1000000;
        lcdDrawText(1, 10, "F:", 0);
        lcdDrawNumber(lcdLastRightPos + 2, 10, frequency, ((menuHorizontalPosition == 0 && s_editMode) ? blink : 0));
        lcdDrawText(lcdLastRightPos + 2, 10, "MHz", 0);
        if (menuHorizontalPosition == 0) {
          reusableBuffer.spectrumAnalyser.freq = checkIncDec(event, frequency, 2400, 2485, 0) * 1000000;
        }
        break;
      }

      case SPEKTRUM_SPAN:
        uint8_t span = reusableBuffer.spectrumAnalyser.span / 1000000;
        lcdDrawText(lcdLastRightPos + 5, 10, "S:", 0);
        lcdDrawNumber(lcdLastRightPos + 2, 10, reusableBuffer.spectrumAnalyser.span/1000000, (menuHorizontalPosition==1 ? blink : 0));
        lcdDrawText(lcdLastRightPos + 2, 10, "MHz", 0);
        if (menuHorizontalPosition == 1) {
          reusableBuffer.spectrumAnalyser.span = checkIncDec(event, span, 1, 80, 0) * 1000000;
        }
        break;
    }
  }
  uint8_t peak_y = 1;
  uint8_t peak_x = 0;
  for (uint8_t i=0; i<LCD_W; i++) {
    uint8_t h = min<uint8_t >(reusableBuffer.spectrumAnalyser.bars[i] >> 1, 128);
    if (h > peak_y) {
      peak_x = i;
      peak_y = h;
    }
    lcdDrawSolidVerticalLine(i, LCD_H - h, h);
  }

  int8_t y = max<int8_t>(FH, LCD_H - peak_y - FH);
  lcdDrawNumber(min<uint8_t>(100, peak_x), y, ((reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2) + peak_x * (reusableBuffer.spectrumAnalyser.span / 128)) / 1000000, TINSIZE);
  lcdDrawText(lcdLastRightPos, y, "M", TINSIZE);
}
