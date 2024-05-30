#include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "linux_ssd1306_ioctl.h"
 
  #include "../bitmap-samples/128x32/arm-logo.h"    /*arm_logo bitmap array*/

 int main()
 {
    int fd;
    int result;

    fd = open("/dev/ssd130601", O_RDWR);
    if (fd == -1){
        printf("Error while opening the display\n");
        return -1;
    }

    /*Note: any buffer sent in must be u8 type, and of lenght device width * no pages*/

    ioctl(fd, SSD1306_SET_BUFFER, arm_logo);        /*load the arm-logo bitmap into the display buffer*/
    ioctl(fd, SSD1306_SHOW_BUFFER, &result);        /*call this function to flush buffer to display*/
    close(fd);

    if(result){
        printf("Could not send buffer to display.");
        return -1; 
    } else{
        printf("Buffer show succeded.");
    }

    return 0;
 }