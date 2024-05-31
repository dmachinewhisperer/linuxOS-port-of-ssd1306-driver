#include <linux/string.h>
#include <linux/uaccess.h>

#include "linux_ssd1306.h"
#include "font8x8_basic.h"

#define PACK8 __attribute__((aligned( __alignof__( u8 ) ), packed ))

/*
Porting Infos added by Asogwa Emmanuel (asogwaemmanuel36@gmail.com)
Documents changes to original file and functions that can be called via ioctl

General Infos:

1. All uintx_t changed to the equivalent kernels ux type
*/

typedef union out_column_t {
	u32 _u32;
	u8  _u8[4];
} PACK8 out_column_t;

/* static function prototypes; unused helper funcs disabled */
static u8 ssd1306_rotate_byte(u8 ch1);

#if 0
    static u8 ssd1306_copy_bit(u8 src, int srcBits, u8 dst, int dstBits);
#endif

static s32 ssd1306_display_image(SSD1306_t * dev, int page, int seg, u8 * images, int width);

//Info: changed return from void to s32
s32 ssd1306_init(SSD1306_t * dev, int width, int height)
{
	int ret;
	if (dev->_address == SPIAddress) {
		ret = spi_init(dev, width, height);
	} else {
		ret = i2c_init(dev, width, height);
	}
	// Initialize internal buffer
	int i;
	for (i=0;i<dev->_pages;i++) {
		memset(dev->_page[i]._segs, 0, 128);
	}

	return ret;
}

//Info: Added new func: ioctl enabled
int ssd1306_get_address(SSD1306_t * dev)
{
	return dev->_address;
}

//Info: ioctl enabled
int ssd1306_get_width(SSD1306_t * dev)
{
	return dev->_width;
}

//Info: ioctl enabled
int ssd1306_get_height(SSD1306_t * dev)
{
	return dev->_height;
}

//Info: ioctl enabled
int ssd1306_get_pages(SSD1306_t * dev)
{
	return dev->_pages;
}

//Info: changed return from void to s32: ioctl enabled
s32 ssd1306_show_buffer(SSD1306_t * dev)
{
    s32 ret;
    int page;
	if (dev->_address == SPIAddress) {
		for (page=0; page<dev->_pages;page++) {
			ret = spi_display_image(dev, page, 0, dev->_page[page]._segs, dev->_width);
            if(!ret){
                return ret;
            }
		}
	} else {
		for (page=0; page<dev->_pages;page++) {
			ret = i2c_display_image(dev, page, 0, dev->_page[page]._segs, dev->_width);
            if(!ret){
                return ret;
            }
		}
	}

    return ret;
}

//Info: ioctl enabled
void ssd1306_set_buffer(SSD1306_t * dev, u8 * buffer)
{
	int index = 0;
    int page;
	for (page=0; page<dev->_pages;page++) {
		memcpy(&dev->_page[page]._segs, &buffer[index], 128);
		index = index + 128;
	}
}

//Info: ioctl enabled
void ssd1306_get_buffer(SSD1306_t * dev, u8 * buffer)
{
	int index = 0;
    int page;
	for (page=0; page<dev->_pages;page++) {
		memcpy(&buffer[index], &dev->_page[page]._segs, 128);
		index = index + 128;
	}
}

//Info: changed scope to static: ioctl enabled
static s32 ssd1306_display_image(SSD1306_t * dev, int page, int seg, u8 * images, int width)
{
    s32 ret;
	if (dev->_address == SPIAddress) {
		ret = spi_display_image(dev, page, seg, images, width);
	} else {
		ret = i2c_display_image(dev, page, seg, images, width);
	}
	// Set to internal buffer
	memcpy(&dev->_page[page]._segs[seg], images, width);

    return ret;
}

//Info: reimplemented to support write() syscall
s32 ssd1306_display_text(SSD1306_t * dev, int page, char * text, int text_len, bool invert)
{
	//Added line-wrap around functionality
	//Added protection to prevent overrunning the text when text_len > the supplied text lenght. 
	//TODO: implement segment addressing. 
	//_seg = _seg - _seg%8;
	//returns number of character written successfully or error code if any image write operation fails

	int ret;
	
	if (page >= dev->_pages) return 0;
	int _text_len = text_len;
	if (_text_len > 16) _text_len = 16;
	int _page = page;

	u8 i;
	u8 seg = 0;
	u8 image[8];
	for (i = 0; (i < _text_len && text[i]!='\0'); i++) {
		memcpy(image, font8x8_basic_tr[(u8)text[i]], 8);
		if (invert) ssd1306_invert(image, 8);
		if (dev->_flip) ssd1306_flip(image, 8);
		ret = ssd1306_display_image(dev, page, seg, image, 8);

		if(ret) return ret; 

		seg = seg + 8;

		/*seg in the range 119 - 127 means we are in the last memory bank
		 go to the next page and make seg point to the first memory bank */
		if(seg > 120){
			//_page++;
			if(++_page >= dev->_pages) return i;
			seg = 0;
		}
		
	}

	return i;
}

//Info: ioctl enabled
// by Coert Vonk
void 
ssd1306_display_text_x3(SSD1306_t * dev, int page, char * text, int text_len, bool invert)
{
	if (page >= dev->_pages) return;
	int _text_len = text_len;
	if (_text_len > 5) _text_len = 5;

	u8 seg = 0;

    u8 nn, xx, yy;

	for (nn = 0; nn < _text_len; nn++) {

		u8 const * const in_columns = font8x8_basic_tr[(u8)text[nn]];

		// make the character 3x as high
		out_column_t out_columns[8];
		memset(out_columns, 0, sizeof(out_columns));

		for (xx = 0; xx < 8; xx++) { // for each column (x-direction)

			u32 in_bitmask = 0b1;
			u32 out_bitmask = 0b111;

			for (yy = 0; yy < 8; yy++) { // for pixel (y-direction)
				if (in_columns[xx] & in_bitmask) {
					out_columns[xx]._u32 |= out_bitmask;
				}
				in_bitmask <<= 1;
				out_bitmask <<= 3;
			}
		}

		// render character in 8 column high pieces, making them 3x as wide
		for (yy = 0; yy < 3; yy++)	{ // for each group of 8 pixels high (y-direction)

			u8 image[24];
			for (xx = 0; xx < 8; xx++) { // for each column (x-direction)
				image[xx*3+0] = 
				image[xx*3+1] = 
				image[xx*3+2] = out_columns[xx]._u8[yy];
			}
			if (invert) ssd1306_invert(image, 24);
			if (dev->_flip) ssd1306_flip(image, 24);  
			if (dev->_address == SPIAddress) {
				spi_display_image(dev, page+yy, seg, image, 24);
			} else {
				i2c_display_image(dev, page+yy, seg, image, 24);
			}
			memcpy(&dev->_page[page+yy]._segs[seg], image, 24);
		}
		seg = seg + 24;
	}
}

//ioctl enabled
void ssd1306_clear_screen(SSD1306_t * dev, bool invert)
{
	char space[16];
    int page;
	memset(space, 0x00, sizeof(space));
	for (page = 0; page < dev->_pages; page++) {
		ssd1306_display_text(dev, page, space, sizeof(space), invert);
	}
}

//ioctl enabled
void ssd1306_clear_line(SSD1306_t * dev, int page, bool invert)
{
	char space[16];
	memset(space, 0x00, sizeof(space));
	ssd1306_display_text(dev, page, space, sizeof(space), invert);
}

//ioctl enabled
void ssd1306_contrast(SSD1306_t * dev, int contrast)
{
	if (dev->_address == SPIAddress) {
		spi_contrast(dev, contrast);
	} else {
		i2c_contrast(dev, contrast);
	}
}

//Info: block not included in port. 
#if 0
    void ssd1306_software_scroll(SSD1306_t * dev, int start, int end)
    {
        ESP_LOGD(TAG, "software_scroll start=%d end=%d _pages=%d", start, end, dev->_pages);
        if (start < 0 || end < 0) {
            dev->_scEnable = false;
        } else if (start >= dev->_pages || end >= dev->_pages) {
            dev->_scEnable = false;
        } else {
            dev->_scEnable = true;
            dev->_scStart = start;
            dev->_scEnd = end;
            dev->_scDirection = 1;
            if (start > end ) dev->_scDirection = -1;
        }
    }


    void ssd1306_scroll_text(SSD1306_t * dev, char * text, int text_len, bool invert)
    {
        ESP_LOGD(TAG, "dev->_scEnable=%d", dev->_scEnable);
        if (dev->_scEnable == false) return;

        void (*func)(SSD1306_t * dev, int page, int seg, u8 * images, int width);
        if (dev->_address == SPIAddress) {
            func = spi_display_image;
        } else {
            func = i2c_display_image;
        }

        int srcIndex = dev->_scEnd - dev->_scDirection;
        while(1) {
            int dstIndex = srcIndex + dev->_scDirection;
            ESP_LOGD(TAG, "srcIndex=%d dstIndex=%d", srcIndex,dstIndex);
            for(int seg = 0; seg < dev->_width; seg++) {
                dev->_page[dstIndex]._segs[seg] = dev->_page[srcIndex]._segs[seg];
            }
            (*func)(dev, dstIndex, 0, dev->_page[dstIndex]._segs, sizeof(dev->_page[dstIndex]._segs));
            if (srcIndex == dev->_scStart) break;
            srcIndex = srcIndex - dev->_scDirection;
        }
        
        int _text_len = text_len;
        if (_text_len > 16) _text_len = 16;
        
        ssd1306_display_text(dev, srcIndex, text, text_len, invert);
    }

    void ssd1306_scroll_clear(SSD1306_t * dev)
    {
        ESP_LOGD(TAG, "dev->_scEnable=%d", dev->_scEnable);
        if (dev->_scEnable == false) return;

        int srcIndex = dev->_scEnd - dev->_scDirection;
        while(1) {
            int dstIndex = srcIndex + dev->_scDirection;
            ESP_LOGD(TAG, "srcIndex=%d dstIndex=%d", srcIndex,dstIndex);
            ssd1306_clear_line(dev, dstIndex, false);
            if (dstIndex == dev->_scStart) break;
            srcIndex = srcIndex - dev->_scDirection;
        }
    }
#endif

//ioctl enabled
void ssd1306_hardware_scroll(SSD1306_t * dev, ssd1306_scroll_type_t scroll)
{
	if (dev->_address == SPIAddress) {
		spi_hardware_scroll(dev, scroll);
	} else {
		i2c_hardware_scroll(dev, scroll);
	}
}

//Info: block not included in port
#if 0
    // delay = 0 : display with no wait
    // delay > 0 : display with wait
    // delay < 0 : no display
    void ssd1306_wrap_arround(SSD1306_t * dev, ssd1306_scroll_type_t scroll, int start, int end, int8_t delay)
    {
        if (scroll == SCROLL_RIGHT) {
            int _start = start; // 0 to 7
            int _end = end; // 0 to 7
            if (_end >= dev->_pages) _end = dev->_pages - 1;
            u8 wk;
            //for (int page=0;page<dev->_pages;page++) {
            for (int page=_start;page<=_end;page++) {
                wk = dev->_page[page]._segs[127];
                for (int seg=127;seg>0;seg--) {
                    dev->_page[page]._segs[seg] = dev->_page[page]._segs[seg-1];
                }
                dev->_page[page]._segs[0] = wk;
            }

        } else if (scroll == SCROLL_LEFT) {
            int _start = start; // 0 to 7
            int _end = end; // 0 to 7
            if (_end >= dev->_pages) _end = dev->_pages - 1;
            u8 wk;
            //for (int page=0;page<dev->_pages;page++) {
            for (int page=_start;page<=_end;page++) {
                wk = dev->_page[page]._segs[0];
                for (int seg=0;seg<127;seg++) {
                    dev->_page[page]._segs[seg] = dev->_page[page]._segs[seg+1];
                }
                dev->_page[page]._segs[127] = wk;
            }

        } else if (scroll == SCROLL_UP) {
            int _start = start; // 0 to {width-1}
            int _end = end; // 0 to {width-1}
            if (_end >= dev->_width) _end = dev->_width - 1;
            u8 wk0;
            u8 wk1;
            u8 wk2;
            u8 save[128];
            // Save pages 0
            for (int seg=0;seg<128;seg++) {
                save[seg] = dev->_page[0]._segs[seg];
            }
            // Page0 to Page6
            for (int page=0;page<dev->_pages-1;page++) {
                //for (int seg=0;seg<128;seg++) {
                for (int seg=_start;seg<=_end;seg++) {
                    wk0 = dev->_page[page]._segs[seg];
                    wk1 = dev->_page[page+1]._segs[seg];
                    if (dev->_flip) wk0 = ssd1306_rotate_byte(wk0);
                    if (dev->_flip) wk1 = ssd1306_rotate_byte(wk1);
                    if (seg == 0) {
                        ESP_LOGD(TAG, "b page=%d wk0=%02x wk1=%02x", page, wk0, wk1);
                    }
                    wk0 = wk0 >> 1;
                    wk1 = wk1 & 0x01;
                    wk1 = wk1 << 7;
                    wk2 = wk0 | wk1;
                    if (seg == 0) {
                        ESP_LOGD(TAG, "a page=%d wk0=%02x wk1=%02x wk2=%02x", page, wk0, wk1, wk2);
                    }
                    if (dev->_flip) wk2 = ssd1306_rotate_byte(wk2);
                    dev->_page[page]._segs[seg] = wk2;
                }
            }
            // Page7
            int pages = dev->_pages-1;
            //for (int seg=0;seg<128;seg++) {
            for (int seg=_start;seg<=_end;seg++) {
                wk0 = dev->_page[pages]._segs[seg];
                wk1 = save[seg];
                if (dev->_flip) wk0 = ssd1306_rotate_byte(wk0);
                if (dev->_flip) wk1 = ssd1306_rotate_byte(wk1);
                wk0 = wk0 >> 1;
                wk1 = wk1 & 0x01;
                wk1 = wk1 << 7;
                wk2 = wk0 | wk1;
                if (dev->_flip) wk2 = ssd1306_rotate_byte(wk2);
                dev->_page[pages]._segs[seg] = wk2;
            }

        } else if (scroll == SCROLL_DOWN) {
            int _start = start; // 0 to {width-1}
            int _end = end; // 0 to {width-1}
            if (_end >= dev->_width) _end = dev->_width - 1;
            u8 wk0;
            u8 wk1;
            u8 wk2;
            u8 save[128];
            // Save pages 7
            int pages = dev->_pages-1;
            for (int seg=0;seg<128;seg++) {
                save[seg] = dev->_page[pages]._segs[seg];
            }
            // Page7 to Page1
            for (int page=pages;page>0;page--) {
                //for (int seg=0;seg<128;seg++) {
                for (int seg=_start;seg<=_end;seg++) {
                    wk0 = dev->_page[page]._segs[seg];
                    wk1 = dev->_page[page-1]._segs[seg];
                    if (dev->_flip) wk0 = ssd1306_rotate_byte(wk0);
                    if (dev->_flip) wk1 = ssd1306_rotate_byte(wk1);
                    if (seg == 0) {
                        ESP_LOGD(TAG, "b page=%d wk0=%02x wk1=%02x", page, wk0, wk1);
                    }
                    wk0 = wk0 << 1;
                    wk1 = wk1 & 0x80;
                    wk1 = wk1 >> 7;
                    wk2 = wk0 | wk1;
                    if (seg == 0) {
                        ESP_LOGD(TAG, "a page=%d wk0=%02x wk1=%02x wk2=%02x", page, wk0, wk1, wk2);
                    }
                    if (dev->_flip) wk2 = ssd1306_rotate_byte(wk2);
                    dev->_page[page]._segs[seg] = wk2;
                }
            }
            // Page0
            //for (int seg=0;seg<128;seg++) {
            for (int seg=_start;seg<=_end;seg++) {
                wk0 = dev->_page[0]._segs[seg];
                wk1 = save[seg];
                if (dev->_flip) wk0 = ssd1306_rotate_byte(wk0);
                if (dev->_flip) wk1 = ssd1306_rotate_byte(wk1);
                wk0 = wk0 << 1;
                wk1 = wk1 & 0x80;
                wk1 = wk1 >> 7;
                wk2 = wk0 | wk1;
                if (dev->_flip) wk2 = ssd1306_rotate_byte(wk2);
                dev->_page[0]._segs[seg] = wk2;
            }

        }

        if (delay >= 0) {
            for (int page=0;page<dev->_pages;page++) {
                if (dev->_address == SPIAddress) {
                    spi_display_image(dev, page, 0, dev->_page[page]._segs, 128);
                } else {
                    i2c_display_image(dev, page, 0, dev->_page[page]._segs, 128);
                }
                if (delay) vTaskDelay(delay);
            }
        }

    }

    void ssd1306_bitmaps(SSD1306_t * dev, int xpos, int ypos, u8 * bitmap, int width, int height, bool invert)
    {
        if ( (width % 8) != 0) {
            ESP_LOGE(TAG, "width must be a multiple of 8");
            return;
        }
        int _width = width / 8;
        u8 wk0;
        u8 wk1;
        u8 wk2;
        u8 page = (ypos / 8);
        u8 _seg = xpos;
        u8 dstBits = (ypos % 8);
        ESP_LOGD(TAG, "ypos=%d page=%d dstBits=%d", ypos, page, dstBits);
        int offset = 0;
        for(int _height=0;_height<height;_height++) {
            for (int index=0;index<_width;index++) {
                for (int srcBits=7; srcBits>=0; srcBits--) {
                    wk0 = dev->_page[page]._segs[_seg];
                    if (dev->_flip) wk0 = ssd1306_rotate_byte(wk0);

                    wk1 = bitmap[index+offset];
                    if (invert) wk1 = ~wk1;

                    //wk2 = ssd1306_copy_bit(bitmap[index+offset], srcBits, wk0, dstBits);
                    wk2 = ssd1306_copy_bit(wk1, srcBits, wk0, dstBits);
                    if (dev->_flip) wk2 = ssd1306_rotate_byte(wk2);

                    ESP_LOGD(TAG, "index=%d offset=%d page=%d _seg=%d, wk2=%02x", index, offset, page, _seg, wk2);
                    dev->_page[page]._segs[_seg] = wk2;
                    _seg++;
                }
            }
            vTaskDelay(1);
            offset = offset + _width;
            dstBits++;
            _seg = xpos;
            if (dstBits == 8) {
                page++;
                dstBits=0;
            }
        }

    #if 0
        for (int _seg=ypos;_seg<ypos+width;_seg++) {
            ssd1306_dump_page(dev, page-1, _seg);
        }
        for (int _seg=ypos;_seg<ypos+width;_seg++) {
            ssd1306_dump_page(dev, page, _seg);
        }
    #endif
        ssd1306_show_buffer(dev);
    }

    // Set pixel to internal buffer. Not show it.
    static void _ssd1306_pixel(SSD1306_t * dev, int xpos, int ypos, bool invert)
    {
        u8 _page = (ypos / 8);
        u8 _bits = (ypos % 8);
        u8 _seg = xpos;
        u8 wk0 = dev->_page[_page]._segs[_seg];
        u8 wk1 = 1 << _bits;
        if (invert) {
            wk0 = wk0 & ~wk1;
        } else {
            wk0 = wk0 | wk1;
        }
        if (dev->_flip) wk0 = ssd1306_rotate_byte(wk0);
        dev->_page[_page]._segs[_seg] = wk0;
    }

    #if 0
        // Set line to internal buffer. Not show it.
        void _ssd1306_line(SSD1306_t * dev, int x1, int y1, int x2, int y2,  bool invert)
        {
            int i;
            int dx,dy;
            int sx,sy;
            int E;

            /* distance between two points */
            dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
            dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

            /* direction of two point */
            sx = ( x2 > x1 ) ? 1 : -1;
            sy = ( y2 > y1 ) ? 1 : -1;

            /* inclination < 1 */
            if ( dx > dy ) {
                E = -dx;
                for ( i = 0 ; i <= dx ; i++ ) {
                    _ssd1306_pixel(dev, x1, y1, invert);
                    x1 += sx;
                    E += 2 * dy;
                    if ( E >= 0 ) {
                    y1 += sy;
                    E -= 2 * dx;
                }
            }

            /* inclination >= 1 */
            } else {
                E = -dy;
                for ( i = 0 ; i <= dy ; i++ ) {
                    _ssd1306_pixel(dev, x1, y1, invert);
                    y1 += sy;
                    E += 2 * dx;
                    if ( E >= 0 ) {
                        x1 += sx;
                        E -= 2 * dy;
                    }
                }
            }
        }
    #endif
#endif


void ssd1306_invert(u8 *buf, size_t blen)
{
    int i;
    u8 wk;
    for(i=0; i<blen; i++){
        wk = buf[i];
        buf[i] = ~wk;
    }
}

//Info: changed scope to static
// Flip upside down
void ssd1306_flip(u8 *buf, size_t blen)
{
    int i;
    for(i=0; i<blen; i++){
        buf[i] = ssd1306_rotate_byte(buf[i]);
    }
}

#if 0
    //Info: changed scope to static; rm log func: not inc in port
    static u8 ssd1306_copy_bit(u8 src, int srcBits, u8 dst, int dstBits)
    {
        u8 smask = 0x01 << srcBits;
        u8 dmask = 0x01 << dstBits;
        u8 _src = src & smask;
    #if 0
        if (_src != 0) _src = 1;
        u8 _wk = _src << dstBits;
        u8 _dst = dst | _wk;
    #endif
        u8 _dst;
        if (_src != 0) {
            _dst = dst | dmask; // set bit
        } else {
            _dst = dst & ~(dmask); // clear bit
        }
        return _dst;
    }
#endif

//Info: changed scope to static   
// Rotate 8-bit data
// 0x12-->0x48
static u8 ssd1306_rotate_byte(u8 ch1) {
    int j;
    u8 ch2 = 0;
    for (j=0;j<8;j++) {
        ch2 = (ch2 << 1) + (ch1 & 0x01);
        ch1 = ch1 >> 1;
    }
    return ch2;
}

//Info: ioctl enabled
void ssd1306_fadeout(SSD1306_t * dev)
{
    int seg;
    int line;
    int page;
	u8 image[1];

	s32 (*func)(SSD1306_t * dev, int page, int seg, u8 * images, int width);
	if (dev->_address == SPIAddress) {
		func = spi_display_image;
	} else {
		func = i2c_display_image;
	}

	for(page=0; page<dev->_pages; page++) {
		image[0] = 0xFF;
		for(line=0; line<8; line++) {
			if (dev->_flip) {
				image[0] = image[0] >> 1;
			} else {
				image[0] = image[0] << 1;
			}
			for(seg=0; seg<128; seg++) {
				(*func)(dev, page, seg, image, 1);
				dev->_page[page]._segs[seg] = image[0];
			}
		}
	}
}


//Info: block not included in port
#if 0
    void ssd1306_dump(SSD1306_t dev)
    {
        printf("_address=%x\n",dev._address);
        printf("_width=%x\n",dev._width);
        printf("_height=%x\n",dev._height);
        printf("_pages=%x\n",dev._pages);
    }
    *

    void ssd1306_dump_page(SSD1306_t * dev, int page, int seg)
    {
        ESP_LOGI(TAG, "dev->_page[%d]._segs[%d]=%02x", page, seg, dev->_page[page]._segs[seg]);
    }
#endif

