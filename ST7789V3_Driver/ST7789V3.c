

/**
 *
 * @file         ST7789V3.c
 * @brief
 *               This file will contain the main logic for using the ST7789V3
 * chips
 *
 */

#include "ST7789V3.h"
#include <stdint.h>
#include <sys/types.h>

// ------------ PRIVATE HELPER FUNCTIONS ------------------

static void WriteCommand(ST7789V3_Config *config, uint8_t cmd) {
  config->set_dc(CMD);
  config->set_cs(LOW);
  config->spi_write(1, &cmd);
  config->set_cs(HIGH);
}

static void WriteData(ST7789V3_Config *config, uint8_t data) {
  config->set_dc(DATA);
  config->set_cs(LOW);
  config->spi_write(1, &data);
  config->set_cs(HIGH);
}

void ST7789V3_init(ST7789V3_Config *config) {
  //   1. Hardware RST pulse GPIO only, no SPI
  //   2. Delay(~120ms after RST goes HIGH)
  //   3. Now start SPI communication
  //   4. SLPOUT(0x11) + delay
  //    other stuff.... for initilization Perhaps Based on users config?

  //

  config->set_rst(HIGH);
  config->delay_ms(10);
  config->set_rst(LOW);
  config->delay_ms(10);
  config->set_rst(HIGH);
  config->delay_ms(120);

  // SPI communication to WAKE
  WriteCommand(config, Sleep_Out);
  config->delay_ms(120);

  // By default set the Color mode to 16 bit
  SetColorMode(config, bit_16);

  // Memory Access Control

  // configure offsets

  // Divide by 2 because Pixels are split equally between left and right sides
  // of the lcd
  config->Col_Offset = (240 - config->LCD_Width) / 2;
  config->Row_Offset = (320 - config->LCD_Height) / 2;

  // Rotation, color Order default RGB

  // Display Inversion

  config->Inversion_Mode = INVOFF;

  // Display On and Show Buffer items

  DISPLAYON(config);

  // Set backlight on
  // config->set_backlight(HIGH);
}

int8_t SetColorMode(ST7789V3_Config *config, Color_Mode bitdepth) {
  WriteCommand(config, COLMODE);
  WriteData(config, bitdepth);
  config->Bit_Depth = bitdepth;

  return 0;
}

void DISPLAYON(ST7789V3_Config *config) {
  WriteCommand(config, Display_On_Register);
}

// Needs to know where the bounds of the lcd are
int8_t SetWindow(ST7789V3_Config *config, uint16_t X_Start, uint16_t X_End,
                 uint16_t Y_Start, uint16_t Y_End) {

  uint16_t X_StartOffset = X_Start + config->Col_Offset;
  uint16_t X_EndOffset = X_End + config->Col_Offset;
  uint16_t Y_StartOffset = Y_Start + config->Row_Offset;
  uint16_t Y_EndOffset = Y_End + config->Row_Offset;

  uint8_t High_Temp = (X_StartOffset >> 8);
  uint8_t Low_Temp = (X_StartOffset & 0xFFU);
  // Horizontal X , write the start write the end of X
  WriteCommand(config, CASET);

  WriteData(config, High_Temp);
  WriteData(config, Low_Temp);
  High_Temp = (X_EndOffset >> 8);
  Low_Temp = (X_EndOffset & 0xFFU);

  WriteData(config, High_Temp);
  WriteData(config, Low_Temp);

  WriteCommand(config, RASET);

  High_Temp = (Y_StartOffset >> 8);
  Low_Temp = (Y_StartOffset & 0xFFU);

  WriteData(config, High_Temp);
  WriteData(config, Low_Temp);

  High_Temp = (Y_EndOffset >> 8);
  Low_Temp = (Y_EndOffset & 0xFFU);

  WriteData(config, High_Temp);
  WriteData(config, Low_Temp);

  WriteCommand(config, RAMWR);
  // Error code for wrong bounds? or exceeded bounds?
  return 0;
}

/**
 * @brief Fill the entire LCD with a single RGB565 color. e.g
 * take orange convert it to 5 bit 6 bit 5 bit
 TODO: Make switch case for 12 bit 16, 18 bit
 */

void FillScreen(ST7789V3_Config *config, uint32_t hexcolor) {
  // Cover the whole display [0, Width-1] x [0, Height-1]
  SetWindow(config, 0, config->LCD_Width - 1, 0, config->LCD_Height - 1);

  uint32_t total_pixels = (config->LCD_Width) * (config->LCD_Height);
  // take a default Hex color say 24 bit and extract it #FF FF FF white
  uint8_t Red5 = (hexcolor >> 19) & 0x1FU;
  uint8_t Green6 = (hexcolor >> 10) & 0x3FU;
  uint8_t Blue5 = (hexcolor >> 3) & 0x1FU;
  uint16_t color565 = 0;
  color565 = (Red5 << 11) | color565;
  color565 = (Green6 << 5) | color565;
  color565 = Blue5 | color565;

  config->set_dc(DATA);
  config->set_cs(LOW);
  // Write Data

  uint8_t color565_High = (color565 >> 8) & 0xFFU;
  uint8_t color565_Low = color565 & 0xFFU;

  for (uint32_t i = 0; i < total_pixels; i++) {
    config->spi_write(1, &color565_High);
    config->spi_write(1, &color565_Low);
  }

  config->set_cs(HIGH);
}

/**
 * @brief Hard reset all settings to default
 */

void HardReset(ST7789V3_Config *config) {
  config->set_rst(HIGH);
  config->delay_ms(10);
  config->set_rst(LOW);
  config->delay_ms(10);
  config->set_rst(HIGH);
  config->delay_ms(120);
}

void InvertDisplay(ST7789V3_Config *config, Inversion_Mode Inversion) {
  config->set_dc(CMD);
  config->set_cs(LOW);

  if (Inversion == INVON) {
    WriteCommand(config, INVON_REG);
  } else {
    WriteCommand(config, INVOFF_REG);
  }

  config->set_cs(HIGH);
}