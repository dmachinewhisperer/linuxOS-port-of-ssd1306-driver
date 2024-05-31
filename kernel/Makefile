ifneq ($(KERNELRELEASE),)

# kbuild part of makefile
obj-m  := ssd1306_oled_driver.o
ssd1306_oled_driver-y := linux_ssd1306_i2c.o linux_ssd1306_spi.o linux_ssd1306_staging.o linux_ssd1306_main.o

else

# normal makefile
KERNEL_DIR ?= $(HOME)/linux_rpi3/linux

all:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=armv8-rpi3-linux-gnueabihf- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=armv8-rpi3-linux-gnueabihf- M=$(PWD) clean

deploy:
	scp *.ko root@10.0.0.10:/home/pi

endif