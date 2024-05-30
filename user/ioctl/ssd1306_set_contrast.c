#include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "linux_ssd1306_ioctl.h"
 
 int main()
 {
    int fd;
    unsigned long contrast = 5;

    fd = open("/dev/ssd130601", O_RDWR);
    if (fd == -1){
        printf("Error while opening the display\n");
        return -1;
    }
    ioctl(fd, SSD1306_SET_CONTRAST, contrast);      /*set contrast*/
    close(fd);
    printf("Display constrast set to: %d", contrast);
    return 0;
 }