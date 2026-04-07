@page project_overview Project Overview

Platform-agnostic C driver for an ST7789V3-based 1.47-inch SPI LCD, intended for bare-metal firmware such as STM32.

## Start Here

- Quick setup: use the main **Quick Start** page
- API reference: [ST7789V3 Display Driver](@ref ST7789V3_Driver)
- Repository details: [README on GitHub](https://github.com/SleepyPandas/ST7789V3-Driver-for-1.47-INCH-LCD/blob/main/README.md)

## Main Files

| File | Purpose |
| --- | --- |
| `ST7789V3_Driver/ST7789V3.h` | Public API and `ST7789V3_Config` |
| `ST7789V3_Driver/ST7789V3.c` | Driver implementation |
| `ST7789V3_Driver/fonts/fonts.h` | Font definitions |
| `Core/Src/main.c` | Example platform wrappers and startup flow |
