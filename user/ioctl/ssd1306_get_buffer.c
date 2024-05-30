#include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "linux_ssd1306_ioctl.h"
 
 int main()
 {
    int fd;
    int pages;
    int width;
    int size;
    u8 *buffer = malloc();

    fd = open("/dev/ssd130601", O_RDWR);
    if (fd == -1){
        printf("Error while opening the display\n");
        return -1;
    }
    ioctl(fd, SSD1306_GET_PAGES, &pages);       /*get the number of pages*/
    ioctl(fd, SSD1306_GET_WIDTH, &width);       /*get the display width*/
    size = pages * width;   

    u8 *buffer = malloc(size);                  /*allocate enough memory store buffer*/
    ioctl(fd, SSD1306_GET_BUFFER, (unsigned long) buffer);      /*retrive the buffer*/
    close(fd);

    printf("Buffer:\n");
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");

    return 0;
 }