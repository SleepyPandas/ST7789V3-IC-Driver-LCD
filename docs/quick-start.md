# Quick Start

This is the fastest way to bring up the driver on a new board.

## What You Fill In

The driver is platform-agnostic. You only need to provide a few small wrapper functions and a `ST7789V3_Config`.

| Field | Required | Notes |
| --- | --- | --- |
| `spi_write` | Yes | Blocking SPI transmit |
| `delay_ms` | Yes | Millisecond delay function |
| `set_cs` | Yes | Chip select pin control |
| `set_dc` | Yes | Data/command pin control |
| `set_rst` | Yes | Reset pin control |
| `LCD_Width` | Yes | `172` for this panel |
| `LCD_Height` | Yes | `320` for this panel |
| `State` | Yes | Start with `ST7789_STATE_READY` |
| `spi_write_dma` | No | Set to `NULL` if you do not use DMA |
| `set_backlight` | No | Set to `NULL` if not used |

## 1. Write The Platform Callbacks

```c
int8_t set_cs(GPIO_Pinstate state) {
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, state);
  return 0;
}

int8_t set_dc(Trans_State state) {
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, state);
  return 0;
}

int8_t set_rst(GPIO_Pinstate state) {
  HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, state);
  return 0;
}

int8_t spi_write(uint16_t len, const uint8_t *pData) {
  return (HAL_SPI_Transmit(&hspi1, (uint8_t *)pData, len, HAL_MAX_DELAY) == HAL_OK)
             ? 0
             : -1;
}
```

## 2. Build The Config

```c
ST7789V3_Config config = {
  .spi_write = spi_write,
  .spi_write_dma = NULL,
  .delay_ms = HAL_Delay,
  .set_cs = set_cs,
  .set_dc = set_dc,
  .set_rst = set_rst,
  .set_backlight = NULL,
  .LCD_Width = 172,
  .LCD_Height = 320,
  .State = ST7789_STATE_READY,
};
```

## 3. Initialize And Draw

```c
ST7789V3_init(&config);

SetRotation(&config, Landscape);
FillScreen(&config, BLACK);
DrawString(&config, 16, 16, "Hello", WHITE, &Font_16x16);
```

## 4. Optional DMA Hook

If you use DMA, set `spi_write_dma` and forward your HAL callbacks:

```c
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    ST7789V3_DMA_Complete(&config);
  }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    ST7789V3_DMA_Error(&config);
  }
}
```

## Next

- API reference: [ST7789V3 Display Driver](@ref ST7789V3_Driver)
- Project summary: [Project Overview](@ref project_overview)
- Full repository notes: `README.md`
