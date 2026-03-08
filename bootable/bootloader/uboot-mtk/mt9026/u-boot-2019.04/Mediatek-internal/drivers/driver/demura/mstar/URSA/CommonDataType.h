// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef CommonDataTypeH
#define CommonDataTypeH
#include <demura.h>
//------------------------------------------------------------------

typedef enum
{
  E_COMP_IC,  // ignore case
  E_COMP_CS   // case sensitive
} E_COMP_OPTION;

//------------------------------------------------------------------
// array element count
#define COUNT_OF(array)   ((int) (sizeof(array) / sizeof(array[0])))

// count of a variable-length array and array's pointer
#define ARRAY_INFO(array)   COUNT_OF(array), array

// Bit values operations
// NOTE: only for 32 bits or less. NOT for 64 bits!
#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif
//#define BITS(_bits_, val)           ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (val<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

#define SET_BIT_ON(_bit_, val)      ( (val) |= BIT(_bit_) )
#define SET_BIT_OFF(_bit_, val)     ( (val) &= ~BIT(_bit_) )

#define IS_BIT_ON(_bit_, val)       ((val) & BIT(_bit_)) ? true : false
#define IS_BIT_OFF(_bit_, val)      ((val) & BIT(_bit_)) ? false : true

#define SET_BIT_ON_OFF(_bit_, val, on_off)      \
  if (on_off != 0) { SET_BIT_ON(_bit_, val); }  \
  else { SET_BIT_OFF(_bit_, val); }

#define GET_BIT_ON_OFF(_bit_, val)         (IS_BIT_ON(_bit_, val) ? 1:0)

#define BIT_POS_TO_MASK(bitPos)     ( (1<<((bitPos)+1)) - 1 )

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


// MAX and MIN
#ifndef MAX
#define MAX(a, b)        (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a ,b)        (((a) < (b)) ? (a) : (b))
#endif

//------------------------------------------------------------------

typedef unsigned char       BYTE;

//------------------------------------------------------------------
typedef enum
{
  VDT_U8,
  VDT_U16,
  VDT_U32,
  VDT_U64,
  VDT_S8,
  VDT_S16,
  VDT_S32,
  VDT_S64,
  VDT_DOUBLE,
  VDT_STRING,
} VAR_DATA_TYPE;

//------------------------------------------------------------------
typedef enum
{
  TEXT_TYPE_NONE,
  TEXT_TYPE_DEC,
  TEXT_TYPE_HEX,
  TEXT_TYPE_FLOAT,
  TEXT_TYPE_ANY
} TEXT_TYPE;

//------------------------------------------------------------------

//---------------------------------------------------------------------------
typedef struct
{
  union
  {
    MS_U8 u8Value;
    struct
    {
      MS_U8  bit_1_0 : 2;
      MS_U8  bit_3_2 : 2;
      MS_U8  bit_5_4 : 2;
      MS_U8  bit_7_6 : 2;
    };
  };
} Pack2222;


typedef enum
{
    TOOL_PACKING = 0,
    IC_DRAM = 1,
    COMPRESSED_IC_DRAM = 2,
} LAYER_DATA_TYPE;


typedef enum
{
    EN_DEMURA_ID_INVLID  = 0x00,
    EN_DEMURA_ID_MSTAR   = 0x01,
    EN_DEMURA_ID_AUO     = 0x02,
    EN_DEMURA_ID_CSOT    = 0x03,
    EN_DEMURA_ID_INX     = 0x04,
    EN_DEMURA_ID_LGD     = 0x05,
    EN_DEMURA_ID_MAX     = 0x06,
}EN_DEMURA_ID;

/* typedef enum
{
    EN_DEMURA_MULTI_NOT  = 0,
    EN_DEMURA_MULTI_AUO,
    EN_DEMURA_MULTI_LGD_START,
    EN_DEMURA_MULTI_LGD_V18,
    EN_DEMURA_MULTI_LGD_SQ18_19_MONO,
    EN_DEMURA_MULTI_LGD_SQ18_19_COLOR,
    EN_DEMURA_MULTI_LGD_V19_MONO,
    EN_DEMURA_MULTI_LGD_V19_COLOR,
    EN_DEMURA_MULTI_LGD_END,
    EN_DEMURA_MULTI_NOVA,
    EN_DEMURA_MULTI_CSOT_HI_SILICON,
    EN_DEMURA_MULTI_CSOT_HIMAX,
    EN_DEMURA_MULTI_SDC,
    EN_DEMURA_MULTI_INX,
    EN_DEMURA_MULTI_CSOT_CSOT,
    EN_DEMURA_MULTI_HKC_NOVA,
    EN_DEMURA_MULTI_MTK,
    EN_DEMURA_MULTI_MAX,
}EN_DEMURA_MULTI_VENDOR;
 */
typedef struct
{
    MS_U32           project_id;
    MS_U32           reg_base_addr;
    LAYER_DATA_TYPE  data_type;
    MS_U32           HNode;
    MS_U32           VNode;
    MS_U32           Blk_h_size;
    MS_U32           Blk_v_size;
    MS_U32           Gain_type;
    MS_U32           Sep_type;   // 1 : R/G/B sperate ;  0 : R/G/B share.
    MS_U32           LevelCount;
    MS_U32           Build_Date;
    EN_DEMURA_ID     Vendor_ID;
    MS_U32           Vendor_ARG1;
    MS_U32           Vendor_ARG2;

    #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
    MS_BOOL          channel_enable[4];   // R/G/B/W channel
    #else
    MS_BOOL          channel_enable[3];   // R/G/B channel
    #endif
    MS_U8            *bin_buf;            // pointer to malloc_buffer
    MS_U32           bin_size;
#if defined(CONFIG_DEMURA_URSA11)
    interface_info   Info[3];
#elif defined(CONFIG_DEMURA_URSA13)
    interface_info   Info;
#elif defined(CONFIG_DEMURA_M7622)
    interface_info   Info;
#elif defined(CONFIG_DEMURA_M7632)
    interface_info   Info;
#elif defined(CONFIG_DEMURA_MT5896)
    interface_info   Info;
#endif
}BinOutputInfo;


#endif
