#include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "linux_ssd1306_ioctl.h"
 
 int main()
 {
    int fd;
    bool invert = 0;

    fd = open("/dev/ssd130601", O_RDWR);
    if (fd == -1){
        printf("Error while opening the display\n");
        return -1;
    }
    ioctl(fd, SSD1306_CLEAR_SCREEN, invert);      /*clear screen*/
    close(fd);
    printf("Display height: %d", (unsigned long) height);
    return 0;
 }