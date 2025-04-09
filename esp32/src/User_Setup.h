#define USER_SETUP_ID 70

#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ESP32 pins
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   4
#define TFT_BL    5

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000 