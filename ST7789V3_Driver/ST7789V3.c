

/**
 *
 * @file         ST7789V3.c
 * @brief
 *               This file will contain the main logic for using the ST7789V3
 * chips
 *
 */

#include "ST7789V3.h"


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

  config->set_rst(High);
  config->delay_ms(120);
  config->set_rst(Low);

  // SPI communication to WAKE
  ST7789V3_WriteCommand(config, Sleep_Out);
  config->delay_ms(120);

  // Color Mode 16 bit TBD

  // Memory Access Control

  // Rotation, color Order default RGB
  
  // Display Inversion 

  // Display On and Show Buffer items
}
