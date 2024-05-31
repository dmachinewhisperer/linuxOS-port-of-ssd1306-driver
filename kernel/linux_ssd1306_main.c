#include <linux/fs.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>

#include "linux_ssd1306.h"
#include "linux_ssd1306_ioctl.h"

/* Device paramters: Here configured for a 128 * 32 OLED display */
static unsigned  width 		= 128;
static unsigned  height 	= 32;
#if 0
module_param(width, unsigned, S_IRUGO);
module_param(height, unsigned, S_IRUGO);
#endif

/* These commands initialize the display */
static ssize_t ssd1306_open(struct inode *inode, struct file *file)
{
	struct ssd1306_dev * ssd1306;

	ssd1306 = container_of(file->private_data, struct ssd1306_dev, ssd1306_miscdevice);
    ssd1306_init(&ssd1306->oled_dev, width, height);

    return 0;

}

/* User is reading data from /dev/ssd1306XX */
static ssize_t ssd1306_read(struct file *file, const char __user *userbuf, size_t count, loff_t *ppos)
{
	/* the serial interface of ssd1306 does not support reading the buffer directly. 
     read the ._pages[] field of the dev struture that mirrows the ram contents instead
	 convert *ppos to the appropirate page and segment
	 
	 From the point of view of the user application, the oled buffer is a 'quantized' linear buffer.
	 *ppos is decomposed into the corresponding page and segment. To ensure the byte array returned are valid
	 characters, we read the segments orderly in groups of eight. 
	 
	 The read operation assumes that characters are represented using 8 segments in a page*/

	struct ssd1306_dev * ssd1306;

	ssd1306 = container_of(file->private_data, struct ssd1306_dev, ssd1306_miscdevice);
	int filesize = ssd1306->oled_dev._pages * 16;

	if(*ppos >= filesize)
		return 0;		/*0 means EOF*/
	
	if (*ppos + count > filesize)
    	count = filesize - (*ppos); 
	
	char buf[count][8];

#if 0
    int start_page = *ppos / 16;
	int start_seg   = (*ppos % 16) * 8;
	int n_read_bytes = 0;
    for(n_page = start_page; n_page < ssd1306->oled_dev._pages; n_page++){
		if(n_read_bytes == count)
			break;
        for(n_seg = start_seg; n_seg< 128; n_seg+=8){
			memcpy(buf + n_read_bytes, ssd1306->oled_dev._page  + n_read_bytes, 8 )
            buf[n_page][n_seg] = ssd1306->oled_dev._page[n_page][n_seg];
			n_read_bytes +=1;
			if(n_read_bytes == count)
				break;
        }
    }
#endif

	int start_page = (*ppos) / 16;
	int start_seg   = ((*ppos) % 16) * 8;
	int n_read_bytes = 0;
	int n_page = start_page;
	int n_seg = start_seg;

	int i;
	for(i = 0; i < count; i++){
		if(n_read_bytes == count)
			break;
		memcpy(buf + i*8, &ssd1306->oled_dev._page + n_page * 128 + n_seg, 8);
		n_seg+=8;
		if(n_seg > 120){
			n_page++;
			n_seg = 0;
		}
	}


	if(copy_to_user(userbuf, buf, count)){
		pr_info("Failed to copy buffer to user\n");
		return -EFAULT;
	}

	*ppos +=  count;

	return count;
}


/* Writing from the terminal command line */
static ssize_t ssd1306_write(struct file *file, char __user *userbuf, size_t count, loff_t *ppos)
{
	int ret;
	int filesize;
	bool invert = 0;

	struct ssd1306_dev * ssd1306;

	ssd1306 = container_of(file->private_data, struct ssd1306_dev, ssd1306_miscdevice);
	filesize = ssd1306->oled_dev._pages * 16;
	
	if ( *ppos >= filesize ) 
		return -EINVAL;
	if (*ppos + count > filesize)
	 	count = filesize - *ppos;
	
	char buf[count];

	dev_info(&ssd1306->client->dev, "ssd1306_write_file entered on %s\n", ssd1306->name);


	if(copy_from_user(buf, userbuf, count)) {
		dev_err(&ssd1306->client->dev, "Bad copied value\n");
		return -EFAULT;
	}

	//*ppos points to the page to begin writing.
	//The underlying driver does not support starting from seg > 0 yet.

    ret = ssd1306_display_text(&ssd1306->oled_dev , *ppos, buf, count, invert);
	if(ret < 0){
		dev_err(&ssd1306->client->dev, "the device is not found\n");
		return ret; 
	} else{
		dev_info(&ssd1306->client->dev, "we have written %zu characters\n", ret); 
	}

	dev_info(&ssd1306->client->dev, "ssd1306_write exited on %s\n", ssd1306->name);

	*ppos += count;

	return count;
}

static long ssd1306_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	int temp; 
	int size;
	u8 *buf = NULL; 
	SSD1306_t *dev;
	struct ssd1306_dev *ssd1306;
	struct function_args *args = NULL;
	
	ssd1306 = container_of(file->private_data, struct ssd1306_dev, ssd1306_miscdevice);
	dev = &ssd1306->oled_dev; 
	size = dev->_width * dev->_pages;
	
    switch(cmd){
        case SSD1306_GET_ADDR:
			temp = ssd1306_get_address(dev);
			if(copy_to_user((int*)arg, &temp, sizeof(int))){
				pr_info("Failed to copy value to user\n");
				return -EFAULT;
			}
            break;

        case SSD1306_GET_WIDTH:
			temp = ssd1306_get_width(dev);
			if(copy_to_user((int*)arg, &temp, sizeof(int))){
				pr_info("Failed to copy value to user\n");
				return -EFAULT;
			}
            break;
 
        case SSD1306_GET_HEIGHT:
			temp = ssd1306_get_height(dev);
			if(copy_to_user((int*)arg, &temp, sizeof(int))){
				pr_info("Failed to copy value to user\n");
				return -EFAULT;
			}
            break;

        case SSD1306_GET_PAGES:
			temp = ssd1306_get_pages(dev);
			if(copy_to_user((int*)arg, &temp, sizeof(int))){
				pr_info("Failed to copy value to user\n");
				return -EFAULT;
			}
            break;

        case SSD1306_SHOW_BUFFER:
			temp = ssd1306_show_buffer(dev);
			if(copy_to_user((int*)arg, &temp, sizeof(int))){
				pr_info("Failed to copy value to user\n");
				return -EFAULT;
			}
            break;
			
        case SSD1306_SET_BUFFER:
            buf = kmalloc(size, GFP_KERNEL);
			if(!buf){
				return -ENOMEM;
			}
			if(copy_from_user(buf, (u8 *)arg, size)) {
				dev_err(&ssd1306->client->dev, "Bad copied value\n");
				return -EFAULT;
			}
			ssd1306_set_buffer(dev, buf);
			kfree(buf);
            break;

        case SSD1306_GET_BUFFER:
            buf = kmalloc(size, GFP_KERNEL);
			if(!buf){
				return -ENOMEM;
			}
			ssd1306_get_buffer(dev, buf);
            if(copy_to_user((u8 *)arg, buf, size)){
				pr_info("Failed to copy value to user\n");
				return -EFAULT;
			}
			kfree(buf);
            break;		

        case SSD1306_DISPLAY_TEXT_X3:
			args = (struct function_args*)kmalloc(sizeof(struct function_args), GFP_KERNEL);
			if(copy_from_user(args, (struct function_args *)arg, sizeof(struct function_args))){
				dev_err(&ssd1306->client->dev, "Bad copied value\n");
				return -EFAULT;
			}
			ssd1306_display_text_x3(dev, args->page, args->text, args->text_len, args->invert);
			kfree(args);
            break;
			
		case SSD1306_CLEAR_LINE:
			args = (struct function_args*)kmalloc(sizeof(struct function_args), GFP_KERNEL);
			if(copy_from_user(args, (struct function_args *)arg, sizeof(struct function_args))){
				dev_err(&ssd1306->client->dev, "Bad copied value\n");
				return -EFAULT;
			}
			ssd1306_clear_line(dev, args->line, args->invert);
			kfree(args);
			break;
			
		case SSD1306_CLEAR_SCREEN:
			ssd1306_clear_screen(dev, (bool)arg);
			break;	

		case SSD1306_SET_CONTRAST:
			ssd1306_contrast(dev, (int)arg);
			break;	

		case SSD1306_FADEOUT:
			ssd1306_fadeout(dev);
			break;	

        default:
            return -ENOTTY;
    }

    return 0;
}

static const struct file_operations ssd1306_fops = {
	.owner = THIS_MODULE,
	.open = ssd1306_open,
	.read = ssd1306_write,
	.write = ssd1306_read,
	.unlocked_ioctl = ssd1306_ioctl,
};

static int ssd1306_probe(struct i2c_client * client, const struct i2c_device_id * id)
{
	static int counter = 0;

	struct ssd1306_dev * ssd1306;

	ssd1306 = devm_kzalloc(&client->dev, sizeof(struct ssd1306_dev), GFP_KERNEL);
	if(!ssd1306){
		return -ENOMEM;
	}

	/* Store pointer to the device-structure in bus device context */
	i2c_set_clientdata(client,ssd1306);

	/* Store pointer to I2C device/client */
	ssd1306->client = client;

	/* Initialize the misc device, ssd1306 incremented after each probe call */
	sprintf(ssd1306->name, "ssd1306%02d", counter++); 
	dev_info(&client->dev, "ssd1306_probe is entered on %s\n", ssd1306->name);

	ssd1306->ssd1306_miscdevice.name = ssd1306->name;//

	ssd1306->ssd1306_miscdevice.name = ssd1306->name;
	ssd1306->ssd1306_miscdevice.minor = MISC_DYNAMIC_MINOR;
	ssd1306->ssd1306_miscdevice.fops = &ssd1306_fops;

	/* Register misc device */
	return misc_register(&ssd1306->ssd1306_miscdevice);

	dev_info(&client->dev, "ssd1306_probe is exited on %s\n", ssd1306->name);

	return 0;
}

static int ssd1306_remove(struct i2c_client * client)
{
	struct ssd1306_dev * ssd1306;

	/* Get device structure from bus device context */	
	ssd1306 = i2c_get_clientdata(client);

	dev_info(&client->dev, "ssd1306_remove is entered on %s\n", ssd1306->name);

	/* Deregister misc device */
	misc_deregister(&ssd1306->ssd1306_miscdevice);

	dev_info(&client->dev, "ssd1306_remove is exited on %s\n", ssd1306->name);

	return 0;
}

static const struct of_device_id ssd1306_dt_ids[] = {
	{ .compatible = "solomon,ssd1306", },
	{ }
};
MODULE_DEVICE_TABLE(of, ssd1306_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
	{ .name = "ssd1306", },
	{ }
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

static struct i2c_driver ssd1306_driver = {
	.driver = {
		.name = "ssd1306",
		.owner = THIS_MODULE,
		.of_match_table = ssd1306_dt_ids,
	},
	.probe = ssd1306_probe,
	.remove = ssd1306_remove,
	.id_table = i2c_ids,
};

module_i2c_driver(ssd1306_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Asogwa Emmanuel <asogwaemmanuel36@gmail.com>");
MODULE_DESCRIPTION("This is a driver for ssd1306 OLED controller");