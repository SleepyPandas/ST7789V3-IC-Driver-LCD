
![Language](https://img.shields.io/badge/Language-C-white) ![Architecture](https://img.shields.io/badge/Architecture-Bare_Metal-blue) ![Platform](https://img.shields.io/badge/Platform-STM32_ArmCortex_M33-42f5da) ![Display-IC](https://img.shields.io/badge/Display_Driver_IC-ST7789V3-a442f5) ![I/O](https://img.shields.io/badge/I%2FO-SPI_DMA-f7b80a) ![Build](https://img.shields.io/badge/Build-CMake-e81526)

A from-scratch, register-level display driver for the ST7789V3 1.47″ LCD (172×320), running on an STM32H503RB (Arm Cortex-M33) with no external graphics libraries. Features a platform-agnostic architecture, non-blocking DMA transfers, a full 2D graphics engine (lines, circles, rectangles, text), RGB888->RGB565 color conversion, and 10 built-in bitmap fonts.

### Please see Documentation Here 
#### [Documentation](https://sleepypandas.github.io/ST7789V3-IC-Driver-LCD/)


## Short Demo 


https://github.com/user-attachments/assets/07f95d09-789a-4d38-9fc8-debc32ab18c1

---

## Key Features

| Feature | Implementation |
| :--- | :--- |
| **Platform-Agnostic Driver Design** | Function-pointer abstraction (`ST7789V3_Config`) decouples all SPI, GPIO, and timing from any specific HAL - portable to any MCU |
| **Non-Blocking DMA Transfers** | `READY -> BUSY -> ERROR` state machine with ISR-driven callbacks; CPU is free during pixel writes |
| **RGB888 -> RGB565 Color Pipeline** | Accepts intuitive 24-bit hex colors (`0xFF6A00`), bit-shifts to 16-bit RGB565 on the fly - no manual conversion needed |
| **Full 2D Graphics Engine** | Bresenham's line algorithm, midpoint circle algorithm, filled shapes - all integer-only arithmetic (no FPU dependency) |
| **Bitmap Font Renderer** | MSB->LSB bitmap parsing with per-row byte indexing; supports `\n`, `\r`, and automatic line wrap; 10 font sizes (8×8 -> 64×64) |
| **Chunked SPI Writes** | `FillScreen` builds 64-pixel buffers to minimize SPI transaction overhead instead of pixel-by-pixel writes |
| **Automatic Memory Offsets** | Column/row offsets are auto-calculated from the physical panel size (240×320) vs. active area (172×320), handling the display's memory window |
| **4-Orientation Rotation** | MADCTL register writes with automatic offset recalculation when `MV` bit swaps hardware axes |

---

## Project Structure
```
ST7789V3-Driver-for-1.47-INCH-LCD/
├── ST7789V3_Driver/
│   ├── ST7789V3.c              # Driver implementation (graphics + DMA)
│   ├── ST7789V3.h              # Public API, register map, config struct
│   └── fonts/
│       ├── fonts.h             # Font struct definition
│       ├── font_8x8.c          # 8×8 bitmap font data
│       ├── font_16x16.c        # …through 64×64
│       └── ...                 # 10 font sizes total
├── Core/Src/
│   └── main.c                  # Platform wrappers, DMA ISR hookup, demo
├── Drivers/                    # STM32 HAL & CMSIS (vendor-provided)
├── CMakeLists.txt
└── STM32H503xx_FLASH.ld       # Linker script
```

* [ST7789V3_Driver/ST7789V3.c](ST7789V3_Driver/ST7789V3.c)
* [ST7789V3_Driver/ST7789V3.h](ST7789V3_Driver/ST7789V3.h)
* [Core/Src/main.c](Core/Src/main.c)

---

### 1. HAL Abstraction Layer

The driver never calls STM32 HAL directly. All platform I/O - SPI, GPIO control, and timing - is injected through function pointers at initialization. The same driver source compiles unchanged on any microcontroller; only the seven function pointers need to change.

```c
struct ST7789V3_Config {
    /* Platform I/O - user fills these in */
    int8_t (*spi_write)(uint16_t len, const uint8_t *pData);
    int8_t (*spi_write_dma)(uint16_t len, const uint8_t *pData);  // NULL = no DMA
    void   (*delay_ms)(uint32_t milliseconds);

    int8_t (*set_cs)(GPIO_Pinstate state);
    int8_t (*set_dc)(Trans_State state);
    int8_t (*set_rst)(GPIO_Pinstate state);
    int8_t (*set_backlight)(GPIO_Pinstate state);

    /* Display attributes */
    uint16_t LCD_Width;
    uint16_t LCD_Height;
    uint8_t  Col_Offset;
    uint8_t  Row_Offset;
    Color_Mode Bit_Depth;

    /* DMA state machine */
    volatile ST7789V3_State State;          // READY, BUSY, ERROR
    ST7789V3_Callback tx_complete_callback; // Fired from ISR on success
    ST7789V3_Callback tx_error_callback;    // Fired from ISR on failure
    void *callback_user_data;
};
```

On the STM32 side, the platform wrappers map directly to HAL calls:

```c
/* Blocking SPI transmit */
int8_t spi_write(uint16_t len, const uint8_t *pData) {
    return (HAL_SPI_Transmit(&hspi1, pData, len, HAL_MAX_DELAY) == HAL_OK) ? 0 : -1;
}

/* Non-blocking SPI transmit via DMA */
int8_t spi_write_dma(uint16_t len, const uint8_t *pData) {
    return (HAL_SPI_Transmit_DMA(&hspi1, pData, len) == HAL_OK) ? 0 : -1;
}

/* Wire the platform layer to the driver */
ST7789V3_Config config = {
    .spi_write     = spi_write,
    .spi_write_dma = spi_write_dma,   // Set to NULL to disable DMA
    .delay_ms      = HAL_Delay,
    .set_cs        = set_cs,
    .set_dc        = set_dc,
    .set_rst       = set_rst,
    .LCD_Width     = 172,
    .LCD_Height    = 320,
    .State         = ST7789_STATE_READY,
};
```

### 2. Initialization Sequence

The ST7789V3 requires a specific startup sequence. The driver executes a hardware reset pulse via GPIO (no SPI yet), waits the datasheet-mandated 120 ms recovery, then begins SPI communication:

```c
// 1. Hardware reset pulse (GPIO only, no SPI)
config->set_rst(HIGH);  config->delay_ms(10);
config->set_rst(LOW);   config->delay_ms(10);
config->set_rst(HIGH);  config->delay_ms(120);

// 2. Wake from sleep via SPI command
WriteCommand(config, 0x11);   // SLPOUT
config->delay_ms(120);

// 3. Configure color depth (default 16-bit RGB565)
SetColorMode(config, bit_16);

// 4. Compute memory offsets for undersized panels
//    Physical memory: 240×320, Active area: 172×320
config->Col_Offset = (240 - config->LCD_Width) / 2;   // = 34
config->Row_Offset = (320 - config->LCD_Height) / 2;   // = 0

// 5. Enable display inversion (required for correct colors on this panel)
InvertDisplay(config, INVON);

// 6. Turn on display
DISPLAYON(config);
```

### 3. RGB888 -> RGB565 Color Pipeline 

The public API accepts colors as intuitive 24-bit hex values (`0xFF6A00` for orange). Internally, the driver bit-shifts each channel down to 5-6-5 bits. This keeps the user-facing API clean while sending the format the display hardware requires.

```c
static uint16_t ConvertRGB888toRGB565(uint32_t hexcolor) {
    uint8_t Red5   = (hexcolor >> 19) & 0x1F;   // 8-bit -> 5-bit
    uint8_t Green6 = (hexcolor >> 10) & 0x3F;   // 8-bit -> 6-bit
    uint8_t Blue5  = (hexcolor >>  3) & 0x1F;   // 8-bit -> 5-bit
    return (Red5 << 11) | (Green6 << 5) | Blue5;
}
```

### 4. Window Addressing (CASET / RASET / RAMWR)

Before writing pixel data, the display needs to know which rectangular region to fill. The driver sets the column/row address window in a single CS-held-LOW transaction to minimize bus overhead - three commands without releasing the chip select line:

```c
// Keep CS low for the full window setup
config->set_cs(LOW);

// Column Address Set (CASET 0x2A) - X boundaries with offset
config->set_dc(CMD);   config->spi_write(1, &CASET);
config->set_dc(DATA);  config->spi_write(4, Column_Data);  // [X_Start, X_End]

// Row Address Set (RASET 0x2B) - Y boundaries with offset
config->set_dc(CMD);   config->spi_write(1, &RASET);
config->set_dc(DATA);  config->spi_write(4, Row_Data);     // [Y_Start, Y_End]

// Memory Write command (RAMWR 0x2C) - display is now ready for pixel data
config->set_dc(CMD);   config->spi_write(1, &RAMWR);

config->set_cs(HIGH);
```

### 5. Graphics Engine

**Shapes** - Lines use Bresenham's algorithm and circles use the midpoint circle algorithm, both integer-only to avoid FPU dependency. Filled shapes use optimized horizontal scanlines (`DrawHLine`) with boundary clamping.

```c
// Bresenham's line - integer-only, no floats
int16_t err = dx + dy;
while (1) {
    DrawPixel(config, cx, cy, hexcolor);
    if (cx == x1 && cy == y1) break;
    int16_t e2 = 2 * err;
    if (e2 >= dy) { err += dy; cx += sx; }
    if (e2 <= dx) { err += dx; cy += sy; }
}
```

**Text** - The font renderer walks each character's bitmap data MSB->LSB, extracting pixel-on/off state from packed byte arrays. It supports `\n` for newlines, `\r` for carriage return, and automatically wraps text when the next character would exceed the display width.

```c
// Extract a single pixel from the font bitmap
uint8_t byte_index = col / 8;
uint8_t bit_index  = col % 8;
uint8_t line_data  = font->data[char_offset + (row * font->bytes_per_row) + byte_index];

if (line_data & (1 << (7 - bit_index))) {
    DrawPixel(config, x + col, y + row, hexcolor);   // Foreground pixel
}
```

### 6. Non-Blocking DMA

The DMA subsystem uses a three-state machine (`READY`, `BUSY`, `ERROR`) with ISR-driven callbacks. When `spi_write_dma` is not `NULL`, large pixel buffers can be sent without blocking the CPU. CS is held LOW during the transfer and released by the ISR handler.

```c
int8_t ST7789V3_WriteBuffer_DMA(ST7789V3_Config *config,
                                const uint8_t *buf, uint16_t len) {
    if (config->spi_write_dma == NULL) return -1;   // DMA not available
    if (config->State != ST7789_STATE_READY) return -1;   // Busy guard

    config->State = ST7789_STATE_BUSY;
    config->set_dc(DATA);
    config->set_cs(LOW);   // Held LOW until ISR calls DMA_Complete

    int8_t ret = config->spi_write_dma(len, buf);
    if (ret != 0) {
        config->set_cs(HIGH);
        config->State = ST7789_STATE_ERROR;
    }
    return ret;
}

/* Called from your SPI DMA transfer-complete ISR */
void ST7789V3_DMA_Complete(ST7789V3_Config *config) {
    config->set_cs(HIGH);
    config->State = ST7789_STATE_READY;
    if (config->tx_complete_callback)
        config->tx_complete_callback(config, config->callback_user_data);
}
```

Platform-side ISR hookup:

```c
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1)
        ST7789V3_DMA_Complete(&config);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1)
        ST7789V3_DMA_Error(&config);
}
```

---

### Hardware
| Component | Detail |
| :--- | :--- |
| **Microcontroller** | STM32H503RB Nucleo-64 (Arm Cortex-M33, 250 MHz) |
| **Display** | Waveshare 1.47″ LCD Module (172×320, IPS, 262K colors) |
| **Display Controller** | Sitronix ST7789V3 |
| **Communication** | SPI (Master, TX-only, 8-bit, CPOL=0/CPHA=0) + SPI DMA (GPDMA1 Ch7) |
| **Toolchain** | CMake, GCC ARM None EABI |

### Wiring
| LCD Pin | STM32 Pin | Function |
| :---: | :---: | :---: |
| VCC | 3.3V | Power |
| GND | GND | Ground |
| DIN (MOSI) | PA7 (SPI1_MOSI) | SPI Data |
| CLK (SCK) | PA5 (SPI1_SCK) | SPI Clock |
| CS | PB5 | Chip Select (software-managed) |
| DC | PB4 | Data/Command select |
| RST | PC8 | Hardware reset |
| BL | - | Backlight (active by default) |

### Build & Flash
```bash
# Configure
cmake --preset Debug

# Build
cmake --build build/Debug

# Flash with your preferred tool (e.g. STM32CubeProgrammer, OpenOCD)
```

### Usage Example
```c
/* Wire platform I/O into the driver */
ST7789V3_Config config = {
    .spi_write     = spi_write,
    .spi_write_dma = spi_write_dma,
    .delay_ms      = HAL_Delay,
    .set_cs        = set_cs,
    .set_dc        = set_dc,
    .set_rst       = set_rst,
    .LCD_Width     = 172,
    .LCD_Height    = 320,
    .State         = ST7789_STATE_READY,
};

/* Initialize display (reset + wake + color mode + offsets) */
ST7789V3_init(&config);

/* Fill screen orange */
FillScreen(&config, 0xFF6A00);

/* Rotate to landscape */
SetRotation(&config, Landscape);

/* Draw some shapes */
DrawFilledRectangle(&config, 50, 50, 100, 100, GREEN);
DrawCircle(&config, 86, 160, 50, WHITE);
DrawFilledCircle(&config, 86, 160, 25, RED);

/* Render text with auto-wrap */
DrawString(&config, 20, 20, "Hello World\nST7789V3 Driver", CYAN, &Font_16x16);
```

---

## References
- [ST7789V3 Datasheet](https://files.waveshare.com/upload/c/c1/ST7789V3_V0.1.pdf)
- [STM32H503RB Nucleo-64 User Manual](https://www.st.com/resource/en/user_manual/um3121-stm32h5-nucleo64-board-mb1814-stmicroelectronics.pdf)
- [Waveshare 1.47″ LCD Module Wiki](https://www.waveshare.com/wiki/1.47inch_LCD_Module)

---

**Driver Source:** [`ST7789V3.c`](ST7789V3_Driver/ST7789V3.c) | [`ST7789V3.h`](ST7789V3_Driver/ST7789V3.h) | **Application:** [`main.c`](Core/Src/main.c)
