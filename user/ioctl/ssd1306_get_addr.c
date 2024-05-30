#include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "linux_ssd1306_ioctl.h"
 
 int main()
 {
    int fd;
    int addr = 0;

    fd = open("/dev/ssd130601", O_RDWR);
    if (fd == -1){
        printf("Error while opening the display\n");
        return -1;
    }
    ioctl(fd, SSD1306_GET_ADDR, &addr); 
    close(fd);
    if(addr < 0xFF){        /*8 bit i2c address*/
        printf("Address: %02X", (uint8_t)addr ); 
    } else{
        printf("Address: %d", addr);
    }
    return 0;
 }