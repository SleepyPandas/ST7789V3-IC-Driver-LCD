

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

static void ST7789V3_WriteCommand(ST7789V3_Config *config, uint8_t cmd) {
  config->set_dc(CMD);
  config->set_cs(LOW);
  config->spi_write(1, &cmd);
  config->set_cs(HIGH);
}

static void ST7789V3_WriteData(ST7789V3_Config *config, uint8_t data) {
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
  config->delay_ms(120);
  config->set_rst(LOW);

  // SPI communication to WAKE
  ST7789V3_WriteCommand(config, Sleep_Out);
  config->delay_ms(120);

  // By default set the Color mode to 16 bit
  SetColorMode(config, bit_16);

  // Memory Access Control

  // configure offsets
  
  // Divide by 2 because Pixels are split equally between left and right sides of the lcd
  config->Col_Offset = (240 - config->LCD_Width) / 2;  
  config->Row_Offset = (320 - config->LCD_Height) / 2; 

  // Rotation, color Order default RGB

  // Display Inversion

  // Display On and Show Buffer items

  DISPLAYON(config);

  // Set backlight on
  config->set_backlight(HIGH);
}

int8_t SetColorMode(ST7789V3_Config *config, Color_Mode bitdepth) {

  ST7789V3_WriteCommand(config, bitdepth);
  config->Bit_Depth = bitdepth;

  return 0;
}

void DISPLAYON(ST7789V3_Config *config) {
  ST7789V3_WriteCommand(config, Display_On_Register);
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
  ST7789V3_WriteCommand(config, CASET);

  ST7789V3_WriteData(config, High_Temp);
  ST7789V3_WriteData(config, Low_Temp);
  High_Temp = (X_EndOffset >> 8);
  Low_Temp = (X_EndOffset & 0xFFU);

  ST7789V3_WriteData(config, High_Temp);
  ST7789V3_WriteData(config, Low_Temp);

  ST7789V3_WriteCommand(config, RASET);

  High_Temp = (Y_StartOffset >> 8);
  Low_Temp = (Y_StartOffset & 0xFFU);

  ST7789V3_WriteData(config, High_Temp);
  ST7789V3_WriteData(config, Low_Temp);

  High_Temp = (Y_EndOffset >> 8);
  Low_Temp = (Y_EndOffset & 0xFFU);

  ST7789V3_WriteData(config, High_Temp);
  ST7789V3_WriteData(config, Low_Temp);

  ST7789V3_WriteCommand(config, RAMWR);
  // Error code for wrong bounds? or exceeded bounds?
  return 0;
}