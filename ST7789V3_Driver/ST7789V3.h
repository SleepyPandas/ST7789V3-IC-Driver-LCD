/**
 * @file ST7789V3.h
 * @brief Public API for the ST7789V3 SPI display driver.
 *
 * This module provides a small, platform-agnostic interface for controlling an
 * ST7789V3-based display over SPI. The application supplies the hardware
 * access callbacks in ::ST7789V3_Config, then uses the driver to initialize
 * the display, configure core display settings, draw graphics primitives, and
 * optionally stream pixel buffers with DMA.
 *
 * Typical responsibilities handled by this header include:
 * - Display initialization and reset flow
 * - Display mode control such as sleep, inversion, and rotation
 * - Drawing text, pixels, lines, rectangles, and circles
 * - Optional non-blocking DMA pixel transfers with completion callbacks
 *
 * Example setup:
 * @code{.c}
 * ST7789V3_Config config = {0};
 *
 * config.set_cs = set_cs;
 * config.set_dc = set_dc;
 * config.set_rst = set_rst;
 * config.spi_write = spi_write;
 * config.spi_write_dma = spi_write_dma;
 * config.delay_ms = HAL_Delay;
 * config.State = ST7789_STATE_READY;
 * config.LCD_Width = 172;
 * config.LCD_Height = 320;
 *
 * ST7789V3_init(&config);
 * FillScreen(&config, BLACK);
 * DrawString(&config, 16, 32, "Hello", WHITE, &Font_16x16);
 * @endcode
 */

#ifndef __ST7789V3_H
#define __ST7789V3_H

#include <fonts/fonts.h>
#include <stdint.h>

/**
 * @defgroup ST7789V3_Driver ST7789V3 Display Driver
 * @brief Public interface for the ST7789V3 display driver.
 * @{
 */

/** @name Controller Command Macros
 * Controller command values used by the ST7789V3 display.
 * @{
 */

#define ST7789V3_rst /**< Reserved placeholder for board-specific reset mapping. */
#define ST7789V3_backlight /**< Reserved placeholder for board-specific backlight mapping. */

#define Sleep_Out 0x11U /**< Exit sleep mode command. */
#define Sleep_In 0x10U /**< Enter sleep mode command. */

#define Display_On_Register 0x29U /**< Turn the display output on. */
#define Display_Off_Register 0x28U /**< Turn the display output off. */

#define CASET 0x2AU /**< Column address set command. */
#define RASET 0x2BU /**< Row address set command. */
#define RAMWR 0x2CU /**< Memory write command. */
#define COLMODE 0x3AU /**< Interface pixel format command. */
#define MADCTL 0x36U /**< Memory data access control command. */

#define INVOFF_REG 0x20U /**< Disable display color inversion. */
#define INVON_REG 0x21U /**< Enable display color inversion. */

/** @} */

/** @name Color Constants
 * Predefined 24-bit RGB888 colors in 0xRRGGBB format.
 * @{
 */

#define BLACK 0x000000U /**< Black in RGB888 format. */
#define WHITE 0xFFFFFFU /**< White in RGB888 format. */
#define RED 0xFF0000U /**< Red in RGB888 format. */
#define GREEN 0x00FF00U /**< Green in RGB888 format. */
#define BLUE 0x0000FFU /**< Blue in RGB888 format. */
#define YELLOW 0xFFFF00U /**< Yellow in RGB888 format. */
#define CYAN 0x00FFFFU /**< Cyan in RGB888 format. */
#define MAGENTA 0xFF00FFU /**< Magenta in RGB888 format. */
#define ORANGE 0xFF6A00U /**< Orange in RGB888 format. */
#define PINK 0xFF69B4U /**< Pink in RGB888 format. */
#define PURPLE 0x800080U /**< Purple in RGB888 format. */
#define LIME 0x32FF00U /**< Lime in RGB888 format. */
#define NAVY 0x000080U /**< Navy in RGB888 format. */
#define DARK_GREEN 0x006400U /**< Dark green in RGB888 format. */
#define MAROON 0x800000U /**< Maroon in RGB888 format. */
#define OLIVE 0x808000U /**< Olive in RGB888 format. */
#define TEAL 0x008080U /**< Teal in RGB888 format. */
#define SILVER 0xC0C0C0U /**< Silver in RGB888 format. */
#define GRAY 0x808080U /**< Gray in RGB888 format. */
#define DARK_GRAY 0x404040U /**< Dark gray in RGB888 format. */
#define LIGHT_GRAY 0xD3D3D3U /**< Light gray in RGB888 format. */
#define GOLD 0xFFD700U /**< Gold in RGB888 format. */
#define SKY_BLUE 0x87CEEBU /**< Sky blue in RGB888 format. */

/** @} */

/** @name Enums and Types
 * Public state, mode, and callback types used by the driver.
 * @{
 */

/**
 * @brief Logical output level used by GPIO control callbacks.
 */
typedef enum {
  HIGH = 1, /**< Drive the output high. */
  LOW = 0, /**< Drive the output low. */
} GPIO_Pinstate;

/**
 * @brief Select whether the current SPI byte stream is command or pixel/data content.
 */
typedef enum {
  DATA = 1, /**< Treat the transfer as display data. */
  CMD = 0, /**< Treat the transfer as a controller command. */
} Trans_State;

/**
 * @brief Pixel format written to the display controller.
 */
typedef enum {
  bit_12 = 0x03U, /**< 12-bit color mode. */
  bit_16 = 0x05U, /**< 16-bit color mode. */
  bit_18 = 0x06U, /**< 18-bit color mode. */
} Color_Mode;

/**
 * @brief Display inversion mode selection.
 */
typedef enum {
  INVON = 1, /**< Enable inversion. */
  INVOFF = 0, /**< Disable inversion. */
} Inversion_Mode;

/**
 * @brief Sleep state selection for the display.
 */
typedef enum {
  Asleep = 0, /**< Request sleep mode. */
  Awake = 1, /**< Request normal awake mode. */
} Sleep_State;

/**
 * @brief Logical display orientation used by ::SetRotation.
 */
typedef enum {
  Portrait = 0, /**< Upright portrait orientation. */
  Landscape = 1, /**< 90-degree landscape orientation. */
  Portrait_Inverted = 2, /**< 180-degree portrait orientation. */
  Landscape_Inverted = 3, /**< 270-degree landscape orientation. */
} Orientation;

/**
 * @brief Driver transfer state used for DMA operations.
 */
typedef enum {
  ST7789_STATE_READY = 0, /**< Ready to start a new transfer. */
  ST7789_STATE_BUSY = 1, /**< A transfer is currently in progress. */
  ST7789_STATE_ERROR = 2, /**< The last DMA transfer ended in an error state. */
} ST7789V3_State;

/**
 * @brief Forward declaration of the driver configuration structure.
 */
typedef struct ST7789V3_Config ST7789V3_Config;

/**
 * @brief Callback signature used for DMA completion and error notifications.
 *
 * @param config Pointer to the active driver configuration.
 * @param user_data Application-defined context pointer from
 *        ::ST7789V3_Config::callback_user_data.
 */
typedef void (*ST7789V3_Callback)(ST7789V3_Config *config, void *user_data);

/** @} */

/** @name Driver Configuration
 * Types and fields required to bind the driver to platform-specific hardware.
 * @{
 */

/**
 * @brief Runtime configuration and hardware binding for one display instance.
 *
 * The application owns this structure and must populate the required callback
 * pointers and display dimensions before calling ::ST7789V3_init. The driver
 * then updates runtime fields such as offsets, color mode, inversion state,
 * and DMA transfer state as it operates.
 */
struct ST7789V3_Config {
  int8_t (*spi_write)(uint16_t len,
                      const uint8_t *pData); /**< Blocking SPI write callback. */
  int8_t (*spi_write_dma)(uint16_t len, const uint8_t *pData); /**< Optional DMA SPI write callback. Set to `NULL` to disable DMA support. */
  void (*delay_ms)(uint32_t milliseconds); /**< Millisecond delay callback used during reset and initialization sequencing. */

  int8_t (*set_cs)(GPIO_Pinstate state); /**< Chip select control callback. */
  int8_t (*set_dc)(Trans_State state); /**< Data/command select callback. */
  int8_t (*set_rst)(GPIO_Pinstate state); /**< Hardware reset control callback. */
  int8_t (*set_backlight)(GPIO_Pinstate state); /**< Optional backlight control callback. */

  uint16_t LCD_Width; /**< Current logical display width in pixels. */
  uint16_t LCD_Height; /**< Current logical display height in pixels. */

  uint8_t Col_Offset; /**< Column offset applied when mapping logical coordinates to controller memory. */
  uint8_t Row_Offset; /**< Row offset applied when mapping logical coordinates to controller memory. */

  Color_Mode Bit_Depth; /**< Cached controller color mode. */
  Inversion_Mode Inversion_Mode; /**< Cached display inversion setting. */
  volatile ST7789V3_State State; /**< Current DMA transfer state. */

  ST7789V3_Callback tx_complete_callback; /**< Optional callback invoked when a DMA transfer completes successfully. */
  ST7789V3_Callback tx_error_callback; /**< Optional callback invoked when a DMA transfer fails. */
  void *callback_user_data; /**< Application-owned context pointer forwarded to DMA callbacks. */
};

/** @} */

/** @name Core Control API
 * Functions for initialization and controller-level display control.
 * @{
 */

/**
 * @brief Initialize the display and apply the default startup sequence.
 *
 * @param config Pointer to a configured driver instance.
 *
 * @note Before calling this function, populate the required GPIO, SPI, and
 *       delay callbacks and set the initial display width and height fields.
 */
void ST7789V3_init(ST7789V3_Config *config);

/**
 * @brief Set the active drawing window on the display.
 *
 * @param config Pointer to the active driver configuration.
 * @param X_Start Inclusive starting column in logical display coordinates.
 * @param X_End Inclusive ending column in logical display coordinates.
 * @param Y_Start Inclusive starting row in logical display coordinates.
 * @param Y_End Inclusive ending row in logical display coordinates.
 * @return `0` on success.
 *
 * @note This function applies the configured row and column offsets before the
 *       address values are written to the controller.
 */
int8_t SetWindow(ST7789V3_Config *config, uint16_t X_Start, uint16_t X_End,
                 uint16_t Y_Start, uint16_t Y_End);

/**
 * @brief Set the controller pixel format.
 *
 * @param config Pointer to the active driver configuration.
 * @param bitdepth Requested color mode.
 * @return `0` on success.
 *
 * @note The selected mode is also cached in ::ST7789V3_Config::Bit_Depth.
 */
int8_t SetColorMode(ST7789V3_Config *config, Color_Mode bitdepth);

/**
 * @brief Enable display output.
 *
 * @param config Pointer to the active driver configuration.
 */
void DISPLAYON(ST7789V3_Config *config);

/**
 * @brief Disable display output.
 *
 * @param config Pointer to the active driver configuration.
 */
void DISPLAYOFF(ST7789V3_Config *config);

/**
 * @brief Toggle the hardware reset line using the driver's reset sequence.
 *
 * @param config Pointer to the active driver configuration.
 */
void HardReset(ST7789V3_Config *config);

/**
 * @brief Enable or disable display inversion.
 *
 * @param config Pointer to the active driver configuration.
 * @param Inversion Requested inversion mode.
 */
void InvertDisplay(ST7789V3_Config *config, Inversion_Mode Inversion);

/**
 * @brief Put the display into sleep mode or wake it up.
 *
 * @param config Pointer to the active driver configuration.
 * @param sleepstate Requested sleep state.
 */
void SleepMode(ST7789V3_Config *config, Sleep_State sleepstate);

/** @} */

/** @name Drawing API
 * Functions for rendering pixels, text, and basic shapes.
 * @{
 */

/**
 * @brief Draw a single pixel using a 24-bit RGB888 input color.
 *
 * @param config Pointer to the active driver configuration.
 * @param x Pixel X coordinate.
 * @param y Pixel Y coordinate.
 * @param hexcolor Color in `0xRRGGBB` RGB888 format.
 * @return `0` on success, or `-1` if the coordinates are outside the current
 *         logical display bounds.
 */
int8_t DrawPixel(ST7789V3_Config *config, uint16_t x, uint16_t y,
                 uint32_t hexcolor);

/**
 * @brief Draw a single printable ASCII character.
 *
 * @param config Pointer to the active driver configuration.
 * @param x Leftmost X position of the character.
 * @param y Topmost Y position of the character.
 * @param user_char Printable ASCII character to draw.
 * @param hexcolor Foreground color in `0xRRGGBB` RGB888 format.
 * @param font Pointer to the font definition used for rendering.
 *
 * @note Characters outside the printable ASCII range are ignored.
 */
void DrawChar(ST7789V3_Config *config, uint16_t x, uint16_t y, char user_char,
              uint32_t hexcolor, const FontDef *font);

/**
 * @brief Fill the entire display with a single color.
 *
 * @param config Pointer to the active driver configuration.
 * @param hexcolor Fill color in `0xRRGGBB` RGB888 format.
 */
void FillScreen(ST7789V3_Config *config, uint32_t hexcolor);

/**
 * @brief Change the logical display rotation.
 *
 * @param config Pointer to the active driver configuration.
 * @param orientation Requested orientation.
 *
 * @note This function updates the cached width, height, and controller offsets
 *       to match the selected orientation.
 */
void SetRotation(ST7789V3_Config *config, Orientation orientation);

/**
 * @brief Draw a null-terminated string.
 *
 * @param config Pointer to the active driver configuration.
 * @param x Starting X position of the first character.
 * @param y Starting Y position of the first character.
 * @param str Null-terminated string to draw.
 * @param hexcolor Text color in `0xRRGGBB` RGB888 format.
 * @param font Pointer to the font definition used for rendering.
 *
 * @note Supports `\n` for newline, `\r` for carriage return, and simple line
 *       wrapping when the next character would exceed the display width.
 */
void DrawString(ST7789V3_Config *config, uint16_t x, uint16_t y,
                const char *str, uint32_t hexcolor, const FontDef *font);

/**
 * @brief Draw a line between two points using Bresenham's algorithm.
 *
 * @param config Pointer to the active driver configuration.
 * @param x0 Starting X coordinate.
 * @param y0 Starting Y coordinate.
 * @param x1 Ending X coordinate.
 * @param y1 Ending Y coordinate.
 * @param hexcolor Line color in `0xRRGGBB` RGB888 format.
 */
void DrawLine(ST7789V3_Config *config, uint16_t x0, uint16_t y0, uint16_t x1,
              uint16_t y1, uint32_t hexcolor);

/**
 * @brief Draw an unfilled rectangle outline.
 *
 * @param config Pointer to the active driver configuration.
 * @param x Top-left X coordinate.
 * @param y Top-left Y coordinate.
 * @param width Rectangle width in pixels.
 * @param height Rectangle height in pixels.
 * @param hexcolor Outline color in `0xRRGGBB` RGB888 format.
 */
void DrawRectangle(ST7789V3_Config *config, uint16_t x, uint16_t y,
                   uint16_t width, uint16_t height, uint32_t hexcolor);

/**
 * @brief Draw a filled rectangle.
 *
 * @param config Pointer to the active driver configuration.
 * @param x Top-left X coordinate.
 * @param y Top-left Y coordinate.
 * @param width Rectangle width in pixels.
 * @param height Rectangle height in pixels.
 * @param hexcolor Fill color in `0xRRGGBB` RGB888 format.
 */
void DrawFilledRectangle(ST7789V3_Config *config, uint16_t x, uint16_t y,
                         uint16_t width, uint16_t height, uint32_t hexcolor);

/**
 * @brief Draw a circle outline using the midpoint circle algorithm.
 *
 * @param config Pointer to the active driver configuration.
 * @param x_center Center X coordinate.
 * @param y_center Center Y coordinate.
 * @param radius Circle radius in pixels.
 * @param hexcolor Outline color in `0xRRGGBB` RGB888 format.
 */
void DrawCircle(ST7789V3_Config *config, uint16_t x_center, uint16_t y_center,
                uint16_t radius, uint32_t hexcolor);

/**
 * @brief Draw a filled circle using the midpoint circle algorithm.
 *
 * @param config Pointer to the active driver configuration.
 * @param x_center Center X coordinate.
 * @param y_center Center Y coordinate.
 * @param radius Circle radius in pixels.
 * @param hexcolor Fill color in `0xRRGGBB` RGB888 format.
 */
void DrawFilledCircle(ST7789V3_Config *config, uint16_t x_center,
                      uint16_t y_center, uint16_t radius, uint32_t hexcolor);

/** @} */

/** @name DMA API
 * Optional non-blocking transfer helpers.
 * @{
 */

/**
 * @brief Start a non-blocking DMA write of pixel data to the display.
 *
 * @param config Pointer to the active driver configuration.
 * @param buf Pointer to the pixel data buffer.
 * @param len Number of bytes to transfer.
 * @return `0` on success, `-1` if DMA support is unavailable or the driver is
 *         not ready, or another negative value from the platform DMA callback.
 *
 * @note Call ::SetWindow before starting the transfer.
 * @note Chip select remains asserted until ::ST7789V3_DMA_Complete or
 *       ::ST7789V3_DMA_Error is called from the matching ISR path.
 */
int8_t ST7789V3_WriteBuffer_DMA(ST7789V3_Config *config,
                                const uint8_t *buf, uint16_t len);

/**
 * @brief Finish a successful DMA transfer and run the completion callback.
 *
 * @param config Pointer to the active driver configuration.
 *
 * @note Call this from the application's SPI or DMA transfer-complete ISR
 *       bridge.
 */
void ST7789V3_DMA_Complete(ST7789V3_Config *config);

/**
 * @brief Finish a failed DMA transfer and run the error callback.
 *
 * @param config Pointer to the active driver configuration.
 *
 * @note Call this from the application's SPI or DMA error ISR bridge.
 */
void ST7789V3_DMA_Error(ST7789V3_Config *config);

/** @} */

/** @} */

#endif /* __ST7789V3_H */
