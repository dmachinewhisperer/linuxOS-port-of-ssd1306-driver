#include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "linux_ssd1306_ioctl.h"
 
 int main()
 {
    int fd;
    int width = 0;

    fd = open("/dev/ssd130601", O_RDWR);
    if (fd == -1){
        printf("Error while opening the display\n");
        return -1;
    }
    ioctl(fd, SSD1306_GET_WIDTH, &width); 
    close(fd);
    printf("Display width: %d", width);
    return 0;
 }a