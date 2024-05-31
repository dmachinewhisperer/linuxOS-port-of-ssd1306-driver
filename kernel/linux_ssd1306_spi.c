#include "linux_ssd1306.h"


s32 spi_init(SSD1306_t *dev, int width, int height){
    return 0;
}

s32 spi_display_image(SSD1306_t * dev, int page, int seg, uint8_t * images, int width){
    return 0;
}


s32 spi_contrast(SSD1306_t * dev, int contrast){
    return 0;
}

s32 spi_hardware_scroll(SSD1306_t * dev, ssd1306_scroll_type_t scroll){
    return 0;
}
