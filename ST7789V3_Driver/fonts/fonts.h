/**
 * @file fonts.h
 * @brief Bitmap font definitions used by the ST7789V3 text drawing API.
 *
 * Depending on your microcontroller, you may not want to include every font
 * size in the final build. Larger fonts consume more flash memory.
 */

#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>

/**
 * @brief Describes one monochrome bitmap font.
 */
typedef struct {
  const uint8_t *data; /**< Pointer to raw glyph bitmap data. */
  uint8_t width; /**< Glyph width in pixels. */
  uint8_t height; /**< Glyph height in pixels. */
  uint8_t bytes_per_row; /**< Number of bytes used to store one glyph row. */
} FontDef;

/**
 * @name Available Fonts
 * List of pre-defined font sizes.
 * @{
 */

extern const FontDef Font_8x8;   /**< 8x8 pixel font. */
extern const FontDef Font_8x16;  /**< 8x16 pixel font. */
extern const FontDef Font_16x16; /**< 16x16 pixel font. */
extern const FontDef Font_16x24; /**< 16x24 pixel font. */
extern const FontDef Font_16x32; /**< 16x32 pixel font. */
extern const FontDef Font_24x24; /**< 24x24 pixel font. */
extern const FontDef Font_24x32; /**< 24x32 pixel font. */
extern const FontDef Font_32x32; /**< 32x32 pixel font. */
extern const FontDef Font_48x48; /**< 48x48 pixel font. */
extern const FontDef Font_64x64; /**< 64x64 pixel font. */

/**
 * @}
 */

#endif /* FONTS_H */
