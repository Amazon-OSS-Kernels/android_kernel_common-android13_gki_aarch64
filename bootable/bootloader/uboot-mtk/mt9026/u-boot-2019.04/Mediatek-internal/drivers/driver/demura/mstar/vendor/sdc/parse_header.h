#ifndef _PARSE_HEADER_H_
#define _PARSE_HEADER_H_

#define _BIT0  0x0001
#define _BIT1  0x0002
#define _BIT2  0x0004
#define _BIT3  0x0008
#define _BIT4  0x0010
#define _BIT5  0x0020
#define _BIT6  0x0040
#define _BIT7  0x0080
#define _BIT8  0x0100
#define _BIT9  0x0200
#define _BIT10 0x0400
#define _BIT11 0x0800
#define _BIT12 0x1000
#define _BIT13 0x2000
#define _BIT14 0x4000
#define _BIT15 0x8000

typedef struct
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned high     : 4;
    unsigned reserve  : 4;
#else /* __BIG_ENDIAN */
    unsigned reserve  : 4;
    unsigned high     : 4;
#endif
    unsigned char low;

} __attribute__((packed)) AUO_BIN_Panel_Level;


typedef struct
{
    MS_U8           PARAMETER_CRC_H;
    MS_U8           PARAMETER_CRC_L;
    MS_U8           dummy0;
    MS_U8           DEMURA_PLANE_NUM;
    MS_U8           DEMURA_TBL_H_H;
    MS_U8           DEMURA_TBL_H_L;
    MS_U8           DEMURA_TBL_V_H;
    MS_U8           DEMURA_TBL_V_L;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U8           dummy1[10];
    MS_U8           LOWER_BOUND_H;
    MS_U8           LOWER_BOUND_L;
    MS_U8           UPPER_BOUND_H;
    MS_U8           UPPER_BOUND_L;
    MS_U8           dummy2[12];
    MS_U8           PLANE00_LV_H;
    MS_U8           PLANE00_LV_L;
    MS_U8           PLANE01_LV_H;
    MS_U8           PLANE01_LV_L;
    MS_U8           PLANE02_LV_H;
    MS_U8           PLANE02_LV_L;
    MS_U8           PLANE03_LV_H;
    MS_U8           PLANE03_LV_L;
    MS_U8           PLANE04_LV_H;
    MS_U8           PLANE04_LV_L;
    MS_U8           dummy3[14];
} __attribute__((packed)) CHOT_BIN_Header;

#endif  // _PARSE_HEADER_H_
