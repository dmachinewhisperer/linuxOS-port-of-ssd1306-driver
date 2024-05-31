# Linux Port of SSD1306 Oled Controller Driver

## Introduction
This is a Linux port of the SSD1306 driver originally implemented for the ESP-IDF framework by `uwelch001` [here](github.com/uwelch001/espidf-ssd1306). The SSD1306 is a monochrome OLED display controller. This implementation is based on the linux `i2c subsystem`

### Features Implemented
(Still in progress)
- [x] Basic initialization and configuration.
- [x] Device file Operations
  - [x] Open
  - [x] Read
  - [x] Write
  - [x] ioctl
- [ ] Drawing primitives: lines, rectangles, circles.
- [x] Text rendering.
- [ ] Screen rotation support.
- [x] I2C communication protocol support.
- [ ] SPI communication protocol support.
- [x] User space sample applications
  - [ ] Open
  - [ ] Write
  - [ ] Read
  - [ ] ioctl



## Environment

### Toolchain
The toolchain was built with crosstool-NG using the `armv8-rpi3-linux-gnueabihf` sample config

### Linux Kernel
The Linux Kernel I used is Raspberry Pi Foundations fork of the mainline kernel found [here](https://github.com/raspberrypifoundation)

### Development Board
Tested on Raspberrypi 3A+ and 3B+ models (with BCM2837B0 SOC)
 
## Usage

To use this driver, follow these steps:

1. Setup a build environment (get or build a toolchain)
2. Build the kernel (mainline Linux kernel or Raspberry Pi's fork) and boot your board
3. Clone the repository.
4. Copy the device tree to the arch/arm(64)/boot/dt
2. Compile the driver and device trees with Kbuild and transfer the built module and device tree binary to the target.
4. Use the userspace application to interact with the device in `/dev/ssd1306X`

## Notes
### The Porting Process
The architecture independent part of the orginal driver ssd1306.c is largely untouched. The corresponding original sources files were renamed to `linux_xxxx`. Mostly the function prototypes were modified to return values to the caller as it typical with linux driver functions and logging functions changed appropirately. 

### Device Tree to Use
The bcm28xx* device tree are the upsteam Linux support files for the bcm SOC family while the bcm27xx* are downstream code by Rasberry Pi Foundations. See [here](https://forums.raspberrypi.com/viewtopic.php?t=238262)

### Modifying the Device Tree to Include the ssd1306 controller
Here is a sample device tree modification for using the i2c1 controller. (also see the modfied .dts file on this repo. )
```
&i2c1 {
	pinctrl-names = "default";
	pinctrl-0 = <&i2c1_pins>;
	clock-frequency = <100000>;

    //start
	status = "okay";
	ssd1306@3c{
		compactible = "solomon,ssd1306";
		reg = <0x3c>;
	};
    //end
};
```
## License
This software is released under the MIT License. See the `LICENSE` file for more information.

