/**
* @brief fonts.h defines the "shape" of a font 
*/

#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>

typedef struct {
  const uint8_t *data;
  uint8_t width;
  uint8_t height;
  uint8_t bytes_per_row;
} FontDef;

extern const FontDef Font_8x8;  
extern const FontDef Font_8x16;
extern const FontDef Font_16x16;
extern const FontDef Font_16x24;
extern const FontDef Font_16x32;
extern const FontDef Font_24x32;

#endif /* FONTS_H */