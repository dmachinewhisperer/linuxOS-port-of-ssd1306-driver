#ifndef LINUX_SSD1306_IOCTL_H
#define LINUX_SSD1306_IOCTL_H


#define SSD1306_MAGIC 'S'

#define SSD1306_GET_ADDR_SEQ_NO         0x0E
#define SSD1306_GET_WIDTH_SEQ_NO        0x00
#define SSD1306_GET_HEIGHT_SEQ_NO       0x01
#define SSD1306_GET_PAGES_SEQ_NO        0x02
#define SSD1306_SHOW_BUFFER_SEG_NO      0x03
#define SSD1306_SET_BUFFER_SEQ_NO       0x04
#define SSD1306_GET_BUFFER_SEQ_NO       0x05
#define SSD1306_DISPLAY_TEXT_X3_SEQ_NO  0x06
#define SSD1306_CLEAR_LINE_SEQ_NO       0x07
#define SSD1306_CLEAR_SCREEN_SEQ_NO     0x08
#define SSD1306_SET_CONTRAST_SEQ_NO     0x0A
#define SSD1306_FADEOUT_SEQ_NO          0x0B
//#define SSD1306_DUMP_DEV_INFO_SEQ_NO    0x0C
//#define SSD1306_DUMP_PAGE_SEQ_NO        0x0D

#define SSD1306_GET_ADDR                _IOR(SSD1306_MAGIC, SSD1306_GET_ADDR_SEQ_NO, int *)
#define SSD1306_GET_WIDTH               _IOR(SSD1306_MAGIC, SSD1306_GET_WIDTH_SEQ_NO, int *)
#define SSD1306_GET_HEIGHT              _IOR(SSD1306_MAGIC, SSD1306_GET_HEIGHT_SEQ_NO, int *)
#define SSD1306_GET_PAGES               _IOR(SSD1306_MAGIC, SSD1306_GET_PAGES_SEQ_NO, int *)
#define SSD1306_SHOW_BUFFER             _IO(SSD1306_MAGIC,SSD1306_SHOW_BUFFER_SEG_NO)
#define SSD1306_SET_BUFFER              _IOW(SSD1306_MAGIC, SSD1306_SET_BUFFER_SEQ_NO, u8 *)
#define SSD1306_GET_BUFFER              _IOR(SSD1306_MAGIC, SSD1306_GET_BUFFER_SEQ_NO, u8 *)
#define SSD1306_DISPLAY_TEXT_X3         _IOW(SSD1306_MAGIC, SSD1306_DISPLAY_TEXT_X3_SEQ_NO, unsigned long)
#define SSD1306_CLEAR_LINE              _IOW(SSD1306_MAGIC,SSD1306_CLEAR_LINE_SEQ_NO, unsigned long)
#define SSD1306_CLEAR_SCREEN            _IOW(SSD1306_MAGIC,SSD1306_CLEAR_SCREEN_SEQ_NO, bool)
#define SSD1306_SET_CONTRAST            _IOW(SSD1306_MAGIC, SSD1306_SET_CONTRAST_SEQ_NO, int)
#define SSD1306_FADEOUT                 _IO(SSD1306_MAGIC,SSD1306_FADEOUT_SEQ_NO)
//#define SSD1306_DUMP_DEV_INFO           _IOR(SSD1306_MAGIC, SSD1306_DUMP_DEV_INFO_SEQ_NO, char *)
//#define SSD1306_DUMP_PAGE               _IOR(SSD1306_MAGIC, SSD1306_DUMP_PAGE_SEQ_NO, char *)


struct function_args{
    int page;           /*Page to apply the operation to*/
    char *text;         /*Text to write, if any*/
    int text_len;       /*Text lenght, if any*/
    bool invert;        /*Invert text*/
    int contrast;       /*Setting contrast*/
    int line;           /*Line to apply operation to*/
};

#endif