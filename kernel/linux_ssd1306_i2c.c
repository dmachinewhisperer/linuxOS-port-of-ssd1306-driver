#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#include "linux_ssd1306.h"

/* These commands initialize the display */
s32 i2c_init(SSD1306_t *dev, int width, int height)                              
{
	int ret;
    int n_commands;
    u8 OLED_CMD_SET_SEGMENT_REMAP;

    dev->_pages = 8;
    u8 OLED_CMD_SET_HEIGHT_0 = 0x3F;
    u8 OLED_CMD_SET_HEIGHT_1 = 0x12;

    dev->_width = width;
	dev->_height = height;
	
	if (dev->_height == 32){
        dev->_pages = 4;
        OLED_CMD_SET_HEIGHT_0 = 0x1F;
        OLED_CMD_SET_HEIGHT_1 = 0x02;
        }
    if (dev->_flip){
        OLED_CMD_SET_SEGMENT_REMAP = OLED_CMD_SET_SEGMENT_REMAP_0;
    } else{
        OLED_CMD_SET_SEGMENT_REMAP = OLED_CMD_SET_SEGMENT_REMAP_1;
    }


    const u8 ssd1306_init_commands[] = {
        OLED_CMD_DISPLAY_OFF,
        OLED_CMD_SET_MUX_RATIO,
        OLED_CMD_SET_HEIGHT_0,
        OLED_CMD_SET_DISPLAY_OFFSET,
        0x00,
        OLED_CMD_SET_DISPLAY_START_LINE,
        OLED_CMD_SET_SEGMENT_REMAP, 
        OLED_CMD_SET_COM_SCAN_MODE,
        OLED_CMD_SET_DISPLAY_CLK_DIV,
        0x80,
        OLED_CMD_SET_COM_PIN_MAP,
        OLED_CMD_SET_HEIGHT_1,
        OLED_CMD_SET_CONTRAST,
        0xFF,
        OLED_CMD_DISPLAY_RAM,
        OLED_CMD_SET_VCOMH_DESELCT,
        0x40,
        OLED_CMD_SET_MEMORY_ADDR_MODE,
        OLED_CMD_SET_PAGE_ADDR_MODE,
        0x00,
        0x10,
        OLED_CMD_SET_CHARGE_PUMP,
        0x14,
        OLED_CMD_DEACTIVE_SCROLL,
        OLED_CMD_DISPLAY_NORMAL,
        OLED_CMD_DISPLAY_ON
    };

    n_commands = sizeof(ssd1306_init_commands)/sizeof(u8)

    struct ssd1306_dev *ssd1306 = container_of(dev, struct ssd1306_dev, oled_dev);

    /* Write the init commands to oled*/
    ret = i2c_smbus_write_i2c_block_data(ssd1306->client, OLED_CONTROL_BYTE_CMD_STREAM, n_commands, ssd1306_init_commands);

    if(ret){
        pr_info("Failed to init the display\n");
		return -EFAULT;
    }

    pr_info("Display configured successfully\n");

    return 0;
}

s32 i2c_display_image(SSD1306_t *dev, int page, int seg, u8 * images, int width) {
    int ret;
    int n_commands;
    u8 *ssd1306_image_data;

    //Asogwa Emmanuel: CONFIG_OFFSETX defined in kprjbuild of the original source: default is 0
    //int _seg = seg + CONFIG_OFFSETX; 
    int _seg = seg + 0;
    int _page = page;

	if (page >= dev->_pages) return -EFAULT;
	if (seg >= dev->_width) return -EFAULT;

	
	u8 columLow = _seg & 0x0F;
	u8 columHigh = (_seg >> 4) & 0x0F;

	if (dev->_flip) {
		_page = (dev->_pages - page) - 1;
	}

	
    const u8 ssd1306_commands[] = {
        (0x00 + columLow),          // Set Lower Column Start Address for Page Addressing Mode
        (0x10 + columHigh),         // Set Higher Column Start Address for Page Addressing Mode
        (0xB0 | _page)              // Set Page Start Address for Page Addressing Mode
    };

    n_commands = sizeof(ssd1306_commands)/sizeof(u8)

    struct ssd1306_dev *ssd1306 = container_of(dev, struct ssd1306_dev, oled_dev);
    ret = i2c_smbus_write_i2c_block_data(ssd1306->client, OLED_CONTROL_BYTE_CMD_STREAM, n_commands, ssd1306_commands);

    if(ret){
        pr_info("Failed to write image\n");
		return -EFAULT;
    }

    //the first command selects the page. the subsquent are the data to fill in the segments
    //append the command that selects the page to write at the beginning of the array.
    //we need 1 + width bytes to store the cmd stream

    ssd1306_image_data = kmalloc(sizeof(u8) * (1 + width), GFP_KERNEL);
    if (!ssd1306_image_data) {
        return -ENOMEM;
    }

    memmove(ssd1306_image_data + 1, images, width);
    ssd1306_image_data[0] = (0xB0 | _page);

    ret = i2c_smbus_write_i2c_block_data(ssd1306->client, OLED_CONTROL_BYTE_DATA_STREAM, 1 + width, ssd1306_image_data);

    kfree(ssd1306_image_data);


    if(ret){
        pr_info("Failed to write image\n");
		return -EFAULT;
    }

    pr_info("Image written successfully\n");  

    return 0;

}

/*
s32 i2c_contrast(SSD1306_t * dev, int contrast) {
	u8 _contrast = contrast;
	if (contrast < 0x0) _contrast = 0;
	if (contrast > 0xFF) _contrast = 0xFF;

    const u8 ssd1306_commands[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        _contrast
    }
    
    struct ssd1306_dev *ssd1306 = container_of(dev, struct ssd1306_dev, oled_dev);
    ret = i2c_smbus_write_i2c_block_data(ssd1306->client, OLED_CONTROL_BYTE_CMD_STREAM, sizeof(ssd1306_commands)/sizeof(u8), ssd1306_commands);

    if(ret){
        pr_info("Failed to set contrast\n");
		return -EFAULT;
    }

    return 0;
}


s32 i2c_hardware_scroll(SSD1306_t * dev, ssd1306_scroll_type_t scroll) {
    u8 *ssd1306_commands;
    int n_commands;
    if(scroll == SCROLL_RIGHT || scroll == SCROLL_LEFT){
        u8 OLED_CMD_HORIZONTAL_DIR;
        if(scroll == SCROLL_RIGHT){
            OLED_CMD_HORIZONTAL_DIR = OLED_CMD_HORIZONTAL_RIGHT;
        } else{
            OLED_CMD_HORIZONTAL_DIR = OLED_CMD_HORIZONTAL_LEFT;
        }
        
        u8 cmds[] = {
            OLED_CMD_HORIZONTAL_DIR,            //26
            0x00,                               // Dummy byte
            0x00,                               // Define start page address
            0x07,                               // Frame frequency
            0x07,                               // Define end page address
            0x00,                               
            0xFF,
            OLED_CMD_ACTIVE_SCROLL              //2F
        }
        ssd1306_commands = cmds; 
        n_commands = sizeof(cmds)/sizeof(u8);
    }

    if(scroll==SCROLL_DOWN || scroll==SCROLL_UP){
        u8 OLED_CMD_HEIGHT; 
        u8 OLED_CMD_VERTICAL_SCROLL_OFFSET;
        if(dev->_height == 64){
            OLED_CMD_HEIGHT = 0x40;
        } else{
            OLED_CMD_HEIGHT = 0x20;
        }
        if(scroll == SCROLL_DOWN){
            OLED_CMD_VERTICAL_SCROLL_OFFSET = 0x3F;
        } else{
            OLED_CMD_VERTICAL_SCROLL_OFFSET = 0x01;
        }
        u8 cmds[] = {
            OLED_CMD_CONTINUOUS_SCROLL,         //29
            0x00,                               //Dummy byte
            0x00,                               // Define start page address
            0x07,                               // Frame frequency
            0x00,                               // Define end page address
            0x3F,                               // Vertical scrolling offset
            OLED_CMD_VERTICAL,                  //A3
            0x00,
            OLED_CMD_HEIGHT,
            OLED_CMD_ACTIVE_SCROLL              //2F
        }
        ssd1306_commands = cmds;
        n_commands = sizeof(cmds)/sizeof(u8);
    }
	

	if (scroll == SCROLL_STOP) {
        u8 cmds[] = {
            OLED_CMD_DEACTIVE_SCROLL            //2E
        }

        ssd1306_commands = cmds;
        n_commands = sizeof(cmds)/sizeof(u8);
		
	}

    struct ssd1306_dev *ssd1306 = container_of(dev, struct ssd1306_dev, oled_dev);
    ret = i2c_smbus_write_i2c_block_data(ssd1306->client, OLED_CONTROL_BYTE_CMD_STREAM, n_commands, ssd1306_commands);

    if(ret){
        pr_info("Failed to set scroll\n");
        return -EFAULT;
    }

    return 0;
}

**/