/**
* @brief fonts.h defines the "shape" of a font / struct 
* @note  Depending on your Microcontroller, 
*        you may not be able to fit all of these fonts in memory.
*        If you are running out of memory, try removing some of the larger fonts.
*        More fonts are just included for convience. 
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

/**
 * @name Available Fonts
 * List of pre-defined font sizes.
 * @{
 */

extern const FontDef Font_8x8;  
extern const FontDef Font_8x16;
extern const FontDef Font_16x16;
extern const FontDef Font_16x24;
extern const FontDef Font_16x32;
extern const FontDef Font_24x24;
extern const FontDef Font_24x32;
extern const FontDef Font_32x32;
extern const FontDef Font_48x48;
extern const FontDef Font_64x64;

/**
 * @}
 */

#endif /* FONTS_H */