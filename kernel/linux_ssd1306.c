#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "linux_ssd1306.h"
#include "font8x8_basic.h"


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

static s32 ssd1306_display_image(SSD1306_t * dev, int page, int seg, u8 * images, int width)
{
	int ret;
	if (dev->_address == SPIAddress) {
		ret = spi_display_image(dev, page, seg, images, width);
	} else {
		ret = i2c_display_image(dev, page, seg, images, width);
	}
	// Set to internal buffer
	memcpy(&dev->_page[page]._segs[seg], images, width);

	return ret;
}

static void ssd1306_invert(u8 *buf, size_t blen)
{
	u8 wk;
	int i;
	for(i=0; i<blen; i++){
		wk = buf[i];
		buf[i] = ~wk;
	}
}

// Rotate 8-bit data
// 0x12-->0x48
static u8 ssd1306_rotate_byte(u8 ch1) {
	u8 ch2 = 0;
	int j;
	for (j=0;j<8;j++) {
		ch2 = (ch2 << 1) + (ch1 & 0x01);
		ch1 = ch1 >> 1;
	}
	return ch2;
}

// Flip upside down
static void ssd1306_flip(u8 *buf, size_t blen)
{
	int i;
	for(i=0; i<blen; i++){
		buf[i] = ssd1306_rotate_byte(buf[i]);
	}
}


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