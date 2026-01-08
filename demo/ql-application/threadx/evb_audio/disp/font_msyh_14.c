/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef FONT_MSYH_14
#define FONT_MSYH_14 1
#endif

#if FONT_MSYH_14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0x60,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x12, 0x9, 0x4, 0x8f, 0xf1, 0x41, 0x23, 0xfc,
    0x48, 0x24, 0x14, 0x0,

    /* U+0024 "$" */
    0x20, 0x87, 0xa8, 0xa2, 0x8e, 0x1c, 0x38, 0x92,
    0x4b, 0xf0, 0x82, 0x0,

    /* U+0025 "%" */
    0x70, 0x91, 0x22, 0x24, 0x45, 0x8, 0xe0, 0xeb,
    0x82, 0x88, 0x51, 0x12, 0x26, 0x44, 0x87, 0x0,

    /* U+0026 "&" */
    0x3c, 0x8, 0x41, 0x9, 0x23, 0x23, 0xc4, 0xf8,
    0xb1, 0xa4, 0x1c, 0x81, 0x98, 0xd1, 0xf3, 0x80,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x29, 0x69, 0x24, 0x91, 0x26, 0x40,

    /* U+0029 ")" */
    0x89, 0x32, 0x49, 0x25, 0x2d, 0x0,

    /* U+002A "*" */
    0x21, 0x3e, 0xcd, 0x0,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0x56,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x4, 0x30, 0x86, 0x10, 0x43, 0x8, 0x21, 0x4,
    0x30, 0x80,

    /* U+0030 "0" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0031 "1" */
    0x33, 0xc1, 0x4, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+0032 "2" */
    0x79, 0x30, 0x41, 0x4, 0x21, 0x8c, 0x63, 0xf,
    0xc0,

    /* U+0033 "3" */
    0x78, 0x8, 0x10, 0x20, 0xc7, 0x1, 0x81, 0x3,
    0xf, 0xe0,

    /* U+0034 "4" */
    0xc, 0xc, 0x14, 0x34, 0x24, 0x44, 0xc4, 0xff,
    0x4, 0x4, 0x4,

    /* U+0035 "5" */
    0x7e, 0x81, 0x2, 0x7, 0x80, 0x80, 0x81, 0x3,
    0xb, 0xe0,

    /* U+0036 "6" */
    0x1e, 0xc1, 0x4, 0xb, 0xd8, 0xe0, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0037 "7" */
    0xfe, 0xc, 0x10, 0x60, 0x81, 0x6, 0x8, 0x30,
    0x40, 0x80,

    /* U+0038 "8" */
    0x38, 0x89, 0x12, 0x22, 0x87, 0x11, 0x41, 0x83,
    0x8d, 0xf0,

    /* U+0039 "9" */
    0x38, 0x8a, 0xc, 0x18, 0x38, 0xde, 0x81, 0x4,
    0x1b, 0xe0,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0x6c, 0x0, 0x1a, 0x48,

    /* U+003C "<" */
    0x6, 0x31, 0x86, 0x7, 0x3, 0x81, 0x80,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0xc0, 0xc0, 0x60, 0x31, 0x8c, 0x20, 0x0,

    /* U+003F "?" */
    0x7a, 0x10, 0x43, 0x18, 0xc2, 0x8, 0x0, 0xc3,
    0x0,

    /* U+0040 "@" */
    0xf, 0x83, 0xc, 0x60, 0x24, 0xf1, 0x89, 0x19,
    0x11, 0x91, 0x19, 0x11, 0x93, 0x2c, 0xdc, 0x40,
    0x3, 0x0, 0x1f, 0x80,

    /* U+0041 "A" */
    0xc, 0x3, 0x1, 0xe0, 0x58, 0x12, 0xc, 0xc2,
    0x31, 0xfc, 0x61, 0x90, 0x2c, 0xc,

    /* U+0042 "B" */
    0xfa, 0x38, 0x61, 0x8b, 0xe8, 0xe1, 0x86, 0x3f,
    0x80,

    /* U+0043 "C" */
    0x1f, 0x20, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x40, 0x61, 0x1f,

    /* U+0044 "D" */
    0xf8, 0x86, 0x82, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x82, 0x84, 0xf8,

    /* U+0045 "E" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x84, 0x3e,

    /* U+0046 "F" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x84, 0x20,

    /* U+0047 "G" */
    0x1f, 0x90, 0x50, 0x10, 0x8, 0x4, 0x3e, 0x3,
    0x1, 0x40, 0xb0, 0x47, 0xc0,

    /* U+0048 "H" */
    0x81, 0x81, 0x81, 0x81, 0x81, 0xff, 0x81, 0x81,
    0x81, 0x81, 0x81,

    /* U+0049 "I" */
    0xe9, 0x24, 0x92, 0x4b, 0x80,

    /* U+004A "J" */
    0x11, 0x11, 0x11, 0x11, 0x13, 0xe0,

    /* U+004B "K" */
    0x85, 0x1a, 0x65, 0x8a, 0x1c, 0x28, 0x58, 0x99,
    0x1a, 0x18,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0x84, 0x3e,

    /* U+004D "M" */
    0xc0, 0x78, 0xf, 0x83, 0xd0, 0x5b, 0x1b, 0x22,
    0x66, 0x4c, 0x59, 0x8a, 0x31, 0xc6, 0x10, 0x80,

    /* U+004E "N" */
    0xc1, 0xc1, 0xe1, 0xb1, 0x91, 0x99, 0x89, 0x85,
    0x87, 0x83, 0x83,

    /* U+004F "O" */
    0x1e, 0x18, 0x64, 0xa, 0x1, 0x80, 0x60, 0x18,
    0x6, 0x1, 0x40, 0x98, 0x61, 0xe0,

    /* U+0050 "P" */
    0xfa, 0x38, 0x61, 0x86, 0x2f, 0x20, 0x82, 0x8,
    0x0,

    /* U+0051 "Q" */
    0x1e, 0xc, 0x31, 0x2, 0x40, 0x28, 0x5, 0x0,
    0xa0, 0x14, 0x2, 0x40, 0x8c, 0x30, 0x78, 0x3,
    0x0, 0x30, 0x3, 0xc0,

    /* U+0052 "R" */
    0xf9, 0x1a, 0x14, 0x28, 0xde, 0x26, 0x44, 0x8d,
    0xa, 0x18,

    /* U+0053 "S" */
    0x7f, 0x18, 0x20, 0xe0, 0xc1, 0x81, 0x6, 0x1f,
    0x80,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+0055 "U" */
    0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x42, 0x3c,

    /* U+0056 "V" */
    0xc0, 0xa0, 0xd0, 0x4c, 0x22, 0x31, 0x10, 0xc8,
    0x2c, 0x14, 0xe, 0x3, 0x0,

    /* U+0057 "W" */
    0xc3, 0xd, 0xc, 0x34, 0x38, 0x98, 0xe2, 0x66,
    0x98, 0x92, 0x62, 0x4d, 0xd, 0x14, 0x1c, 0x70,
    0x61, 0xc1, 0x86, 0x0,

    /* U+0058 "X" */
    0x41, 0x31, 0x88, 0x82, 0xc1, 0xc0, 0x60, 0x50,
    0x6c, 0x22, 0x31, 0xb0, 0x60,

    /* U+0059 "Y" */
    0x83, 0x8d, 0x13, 0x62, 0x85, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+005A "Z" */
    0x7f, 0x2, 0x6, 0x4, 0xc, 0x18, 0x10, 0x30,
    0x60, 0x60, 0xff,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x4e,

    /* U+005C "\\" */
    0x83, 0x4, 0x10, 0x20, 0x83, 0x4, 0x10, 0x60,
    0x82, 0x4,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x9e,

    /* U+005E "^" */
    0x0, 0x60, 0xc3, 0x44, 0x98, 0xa1, 0x0,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0x99, 0x0,

    /* U+0061 "a" */
    0x78, 0x10, 0x4f, 0xc6, 0x18, 0xdd,

    /* U+0062 "b" */
    0x81, 0x2, 0x4, 0xb, 0xd8, 0xa0, 0xc1, 0x83,
    0x7, 0x15, 0xc0,

    /* U+0063 "c" */
    0x3d, 0x18, 0x20, 0x82, 0x4, 0x4f,

    /* U+0064 "d" */
    0x2, 0x4, 0x8, 0x13, 0xa8, 0xe0, 0xc1, 0x83,
    0x5, 0x1b, 0xd0,

    /* U+0065 "e" */
    0x3c, 0x8e, 0xf, 0xf8, 0x10, 0x10, 0x9e,

    /* U+0066 "f" */
    0x3a, 0x10, 0x8f, 0x21, 0x8, 0x42, 0x10, 0x80,

    /* U+0067 "g" */
    0x3a, 0x8e, 0xc, 0x18, 0x30, 0x51, 0xbd, 0x2,
    0x6, 0x17, 0xc0,

    /* U+0068 "h" */
    0x82, 0x8, 0x20, 0xbb, 0x38, 0x61, 0x86, 0x18,
    0x61,

    /* U+0069 "i" */
    0x8f, 0xf0,

    /* U+006A "j" */
    0x10, 0x0, 0x11, 0x11, 0x11, 0x11, 0x11, 0x3e,

    /* U+006B "k" */
    0x82, 0x8, 0x20, 0x8a, 0x6b, 0x38, 0xe2, 0xc9,
    0xa3,

    /* U+006C "l" */
    0xff, 0xf0,

    /* U+006D "m" */
    0xb9, 0xd9, 0xce, 0x10, 0xc2, 0x18, 0x43, 0x8,
    0x61, 0xc, 0x21,

    /* U+006E "n" */
    0xbb, 0x38, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x51, 0x1c,

    /* U+0070 "p" */
    0xbd, 0x8a, 0xc, 0x18, 0x30, 0x71, 0x5c, 0x81,
    0x2, 0x4, 0x0,

    /* U+0071 "q" */
    0x3a, 0x8e, 0xc, 0x18, 0x30, 0x51, 0xbd, 0x2,
    0x4, 0x8, 0x10,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88, 0x88,

    /* U+0073 "s" */
    0x7c, 0x21, 0xc3, 0x84, 0x3e,

    /* U+0074 "t" */
    0x42, 0x3e, 0x84, 0x21, 0x8, 0x41, 0xc0,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x1c, 0xdd,

    /* U+0076 "v" */
    0xc2, 0x8d, 0x13, 0x22, 0xc5, 0xe, 0x8,

    /* U+0077 "w" */
    0xc4, 0x68, 0xc9, 0x29, 0x25, 0x26, 0xac, 0x73,
    0xc, 0x61, 0x8c,

    /* U+0078 "x" */
    0x46, 0xc8, 0xa1, 0xc3, 0x85, 0x1b, 0x63,

    /* U+0079 "y" */
    0x87, 0x89, 0x12, 0x66, 0x85, 0xe, 0x18, 0x10,
    0x60, 0x86, 0x0,

    /* U+007A "z" */
    0xfc, 0x21, 0x84, 0x21, 0x84, 0x3f,

    /* U+007B "{" */
    0x29, 0x24, 0xa2, 0x49, 0x22,

    /* U+007C "|" */
    0xff, 0xff,

    /* U+007D "}" */
    0x89, 0x24, 0x8a, 0x49, 0x28,

    /* U+007E "~" */
    0x72, 0x8e,

    /* U+007F "" */
    0x0,

    /* U+4E91 "云" */
    0x7f, 0xf0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x7f, 0xfc, 0x30, 0x1, 0x10, 0x10, 0xc1, 0x3,
    0x1f, 0xfc, 0x0, 0x20,

    /* U+4F7F "使" */
    0x10, 0x80, 0x42, 0x3, 0xff, 0xc8, 0x20, 0x20,
    0x81, 0x9f, 0xee, 0x48, 0x89, 0xfe, 0x20, 0x88,
    0x92, 0x2, 0x30, 0x8, 0x40, 0x22, 0xe0, 0xb0,
    0x60,

    /* U+6B22 "欢" */
    0x1, 0x3, 0xe8, 0xf, 0xbf, 0x82, 0x82, 0xcc,
    0x91, 0x52, 0x43, 0x8, 0xc, 0x20, 0x31, 0xc0,
    0xa5, 0x4, 0xf2, 0x21, 0x8c, 0x8c, 0x18,

    /* U+7528 "用" */
    0x3f, 0xf9, 0x8, 0x48, 0x42, 0x42, 0x13, 0xff,
    0x90, 0x84, 0x84, 0x27, 0xff, 0x21, 0x9, 0x8,
    0x50, 0x42, 0x82, 0x14, 0x17, 0x80,

    /* U+7BB1 "箱" */
    0x20, 0x81, 0x4, 0x1f, 0xbe, 0xa2, 0x88, 0x96,
    0x11, 0xfb, 0xe8, 0x44, 0x7e, 0x72, 0x13, 0x50,
    0xaa, 0xfc, 0x44, 0x22, 0x3f, 0x11, 0x8,

    /* U+8FCE "迎" */
    0x43, 0x80, 0x91, 0xe2, 0x44, 0x81, 0x12, 0xe4,
    0x48, 0x91, 0x22, 0x44, 0x89, 0x52, 0x26, 0x78,
    0x81, 0x6, 0x4, 0x14, 0x0, 0x8f, 0xfc,

    /* U+97F3 "音" */
    0x2, 0x0, 0x10, 0x1f, 0xfc, 0x20, 0x80, 0x88,
    0x7f, 0xfc, 0x0, 0x7, 0xfc, 0x20, 0x21, 0xff,
    0x8, 0x8, 0x40, 0x43, 0xfe, 0x10, 0x10
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 66, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 70, .box_w = 1, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 98, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 5, .adv_w = 143, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 17, .adv_w = 131, .box_w = 6, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 29, .adv_w = 199, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 195, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 57, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 62, .adv_w = 75, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 68, .adv_w = 75, .box_w = 3, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 74, .adv_w = 102, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 78, .adv_w = 166, .box_w = 7, .box_h = 7, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 85, .adv_w = 54, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 86, .adv_w = 97, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 87, .adv_w = 54, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 96, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 98, .adv_w = 131, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 131, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 131, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 131, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 131, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 131, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 131, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 131, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 54, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 54, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 203, .adv_w = 166, .box_w = 7, .box_h = 7, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 210, .adv_w = 166, .box_w = 7, .box_h = 4, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 214, .adv_w = 166, .box_w = 7, .box_h = 7, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 221, .adv_w = 108, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 231, .box_w = 12, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 250, .adv_w = 158, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 141, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 150, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 171, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 123, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 302, .adv_w = 119, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 167, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 173, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 66, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 89, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 115, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 219, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 182, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 388, .adv_w = 183, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 137, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 183, .box_w = 11, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 431, .adv_w = 146, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 129, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 460, .adv_w = 167, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 471, .adv_w = 151, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 228, .box_w = 14, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 504, .adv_w = 144, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 517, .adv_w = 135, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 527, .adv_w = 139, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 538, .adv_w = 75, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 543, .adv_w = 93, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 553, .adv_w = 75, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 558, .adv_w = 166, .box_w = 7, .box_h = 7, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 565, .adv_w = 100, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 566, .adv_w = 66, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 568, .adv_w = 124, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 574, .adv_w = 143, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 585, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 591, .adv_w = 143, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 602, .adv_w = 127, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 609, .adv_w = 78, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 617, .adv_w = 143, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 628, .adv_w = 138, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 637, .adv_w = 60, .box_w = 1, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 639, .adv_w = 60, .box_w = 4, .box_h = 16, .ofs_x = -2, .ofs_y = -4},
    {.bitmap_index = 647, .adv_w = 122, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 656, .adv_w = 60, .box_w = 1, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 210, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 669, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 675, .adv_w = 142, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 682, .adv_w = 143, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 693, .adv_w = 143, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 704, .adv_w = 86, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 104, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 713, .adv_w = 83, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 138, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 726, .adv_w = 118, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 733, .adv_w = 177, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 744, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 751, .adv_w = 119, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 762, .adv_w = 110, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 768, .adv_w = 75, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 773, .adv_w = 60, .box_w = 1, .box_h = 16, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 775, .adv_w = 75, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 780, .adv_w = 166, .box_w = 8, .box_h = 2, .ofs_x = 2, .ofs_y = 3},
    {.bitmap_index = 782, .adv_w = 66, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 783, .adv_w = 224, .box_w = 13, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 803, .adv_w = 224, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 828, .adv_w = 224, .box_w = 14, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 851, .adv_w = 224, .box_w = 13, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 873, .adv_w = 224, .box_w = 13, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 896, .adv_w = 224, .box_w = 14, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 919, .adv_w = 224, .box_w = 13, .box_h = 14, .ofs_x = 0, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0xee, 0x1c91, 0x2697, 0x2d20, 0x413d, 0x4962
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 96, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 20113, .range_length = 18787, .glyph_id_start = 97,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 7, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    3, 83,
    3, 84,
    8, 83,
    8, 84,
    9, 75,
    11, 34,
    11, 43,
    11, 68,
    11, 69,
    11, 70,
    11, 72,
    11, 80,
    11, 82,
    34, 11,
    34, 13,
    34, 28,
    34, 36,
    34, 40,
    34, 43,
    34, 48,
    34, 53,
    34, 54,
    34, 55,
    34, 56,
    34, 58,
    34, 59,
    34, 85,
    34, 87,
    34, 88,
    34, 90,
    35, 53,
    35, 58,
    36, 32,
    36, 36,
    36, 40,
    36, 48,
    36, 50,
    37, 13,
    37, 15,
    37, 34,
    37, 53,
    37, 57,
    37, 59,
    38, 34,
    38, 43,
    38, 53,
    38, 56,
    38, 57,
    39, 13,
    39, 15,
    39, 34,
    39, 43,
    39, 52,
    39, 53,
    39, 66,
    39, 71,
    40, 53,
    40, 55,
    40, 90,
    43, 13,
    43, 15,
    43, 34,
    43, 43,
    43, 66,
    44, 13,
    44, 28,
    44, 36,
    44, 40,
    44, 43,
    44, 48,
    44, 50,
    44, 57,
    44, 59,
    44, 68,
    44, 69,
    44, 70,
    44, 72,
    44, 80,
    44, 82,
    44, 85,
    44, 87,
    44, 88,
    44, 90,
    45, 11,
    45, 32,
    45, 34,
    45, 36,
    45, 40,
    45, 43,
    45, 48,
    45, 50,
    45, 53,
    45, 54,
    45, 55,
    45, 56,
    45, 58,
    45, 59,
    45, 85,
    45, 87,
    45, 88,
    45, 90,
    48, 13,
    48, 15,
    48, 34,
    48, 43,
    48, 53,
    48, 57,
    48, 58,
    48, 59,
    49, 13,
    49, 15,
    49, 34,
    49, 40,
    49, 43,
    49, 56,
    49, 57,
    49, 66,
    49, 68,
    49, 69,
    49, 70,
    49, 72,
    49, 80,
    49, 82,
    50, 13,
    50, 15,
    50, 34,
    50, 53,
    50, 57,
    50, 58,
    50, 59,
    51, 28,
    51, 36,
    51, 40,
    51, 43,
    51, 48,
    51, 50,
    51, 53,
    51, 58,
    51, 68,
    51, 69,
    51, 70,
    51, 72,
    51, 80,
    51, 82,
    52, 85,
    52, 87,
    52, 88,
    52, 90,
    53, 13,
    53, 15,
    53, 27,
    53, 28,
    53, 34,
    53, 36,
    53, 40,
    53, 43,
    53, 48,
    53, 50,
    53, 53,
    53, 55,
    53, 56,
    53, 57,
    53, 58,
    53, 66,
    53, 68,
    53, 69,
    53, 70,
    53, 71,
    53, 72,
    53, 78,
    53, 79,
    53, 80,
    53, 81,
    53, 82,
    53, 83,
    53, 84,
    53, 86,
    53, 87,
    53, 88,
    53, 89,
    53, 90,
    53, 91,
    54, 34,
    55, 13,
    55, 15,
    55, 34,
    55, 36,
    55, 40,
    55, 43,
    55, 48,
    55, 50,
    55, 52,
    55, 53,
    55, 66,
    55, 68,
    55, 69,
    55, 70,
    55, 72,
    55, 78,
    55, 79,
    55, 80,
    55, 81,
    55, 82,
    55, 83,
    55, 84,
    55, 86,
    56, 13,
    56, 15,
    56, 34,
    56, 53,
    56, 66,
    56, 68,
    56, 69,
    56, 70,
    56, 72,
    56, 80,
    56, 82,
    57, 13,
    57, 15,
    57, 28,
    57, 36,
    57, 40,
    57, 43,
    57, 48,
    57, 50,
    57, 53,
    58, 13,
    58, 15,
    58, 34,
    58, 36,
    58, 40,
    58, 43,
    58, 48,
    58, 50,
    58, 52,
    58, 53,
    58, 66,
    58, 68,
    58, 69,
    58, 70,
    58, 71,
    58, 72,
    58, 78,
    58, 79,
    58, 80,
    58, 81,
    58, 82,
    58, 83,
    58, 84,
    58, 86,
    59, 43,
    59, 53,
    59, 90,
    60, 75,
    67, 66,
    67, 71,
    67, 89,
    68, 43,
    68, 53,
    68, 58,
    70, 3,
    70, 8,
    71, 10,
    71, 13,
    71, 14,
    71, 15,
    71, 27,
    71, 28,
    71, 32,
    71, 62,
    71, 67,
    71, 73,
    71, 85,
    71, 87,
    71, 88,
    71, 89,
    71, 90,
    71, 94,
    72, 75,
    75, 75,
    76, 13,
    76, 14,
    76, 15,
    76, 27,
    76, 28,
    76, 68,
    76, 69,
    76, 70,
    76, 72,
    76, 80,
    76, 82,
    76, 85,
    79, 3,
    79, 8,
    80, 3,
    80, 8,
    80, 66,
    80, 71,
    80, 89,
    81, 66,
    81, 71,
    81, 89,
    82, 75,
    83, 13,
    83, 14,
    83, 15,
    83, 27,
    83, 28,
    83, 68,
    83, 69,
    83, 70,
    83, 71,
    83, 72,
    83, 78,
    83, 79,
    83, 80,
    83, 82,
    83, 84,
    83, 85,
    83, 87,
    83, 88,
    83, 89,
    83, 90,
    83, 91,
    85, 14,
    85, 32,
    85, 68,
    85, 69,
    85, 70,
    85, 72,
    85, 80,
    85, 82,
    85, 89,
    86, 3,
    86, 8,
    87, 13,
    87, 15,
    87, 66,
    87, 68,
    87, 69,
    87, 70,
    87, 72,
    87, 80,
    87, 82,
    88, 13,
    88, 15,
    88, 68,
    88, 69,
    88, 70,
    88, 72,
    88, 80,
    88, 82,
    89, 68,
    89, 69,
    89, 70,
    89, 72,
    89, 80,
    89, 82,
    90, 3,
    90, 8,
    90, 13,
    90, 15,
    90, 32,
    90, 68,
    90, 69,
    90, 70,
    90, 71,
    90, 72,
    90, 80,
    90, 82,
    90, 85,
    92, 75
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -6, -8, -6, -8, 28, -20, -18, -12,
    -12, -12, -12, -12, -12, -15, 8, 8,
    -3, -3, 11, -3, -17, -3, -14, -9,
    -18, 7, -3, -5, -3, -4, -11, -8,
    0, -6, -6, -3, -6, -15, -15, -4,
    -11, -6, -6, 1, 8, 0, 3, 1,
    -18, -18, -16, -8, -3, 2, -9, 1,
    -6, -3, -3, -12, -12, -4, -8, -3,
    5, 5, -11, -11, 11, -11, -11, 4,
    5, -3, -3, -3, -3, -3, -3, -6,
    -9, -6, -11, -24, -12, 7, -8, -8,
    12, -8, -8, -13, -3, -14, -6, -15,
    7, -3, -12, -8, -9, -11, -11, -3,
    -1, -11, -4, -3, -6, -38, -38, -19,
    -1, -15, 5, -7, -8, -9, -9, -9,
    -9, -9, -9, -11, -15, -3, -11, -4,
    -1, -6, 10, -3, -3, 7, -2, -2,
    -6, -5, -6, -6, -7, -7, -7, -6,
    -8, -6, -3, -6, -15, -21, -3, -3,
    -18, -11, -11, -13, -11, -11, 5, 5,
    5, -1, 3, -26, -25, -25, -25, -11,
    -25, -21, -21, -25, -21, -25, -21, -18,
    -21, -12, -13, -21, -13, -15, -5, -24,
    -27, -14, -5, -5, -8, -1, -5, -3,
    5, -17, -15, -15, -15, -15, -9, -9,
    -15, -9, -15, -9, -8, -9, -14, -15,
    -9, 5, -9, -6, -6, -6, -6, -6,
    -6, 8, 7, 10, -3, -3, 11, -3,
    -3, 4, -21, -23, -19, -5, -5, -8,
    -5, -5, -3, 5, -24, -21, -21, -21,
    -3, -21, -17, -17, -21, -17, -21, -17,
    -16, -17, 10, 5, -6, 28, -3, -1,
    -3, 8, -12, -9, -12, -12, 17, -15,
    -12, -15, 10, 10, 8, 17, 2, 2,
    4, 5, 5, 2, 4, 10, 6, 4,
    10, -16, 10, 10, 10, -5, -3, -5,
    -5, -5, -3, -2, -12, -12, -17, -17,
    -3, -4, -3, -3, -4, -3, 12, -19,
    -15, -20, 10, 10, -3, -3, -3, 5,
    -3, 0, 0, -3, -3, 2, 7, 10,
    10, 7, 10, 5, -13, -6, -3, -3,
    -2, -2, -2, -2, 3, -8, -8, -14,
    -15, -4, -1, -2, -1, -1, -1, -2,
    -11, -12, -1, -1, -1, -1, -1, -1,
    -2, -2, -2, -2, -2, -2, 3, 3,
    -12, -15, -9, -1, -1, -1, 0, -1,
    -1, -1, 1, 24
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 372,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t font_msyh_14 = {
#else
lv_font_t font_msyh_14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if FONT_MSYH_14*/

