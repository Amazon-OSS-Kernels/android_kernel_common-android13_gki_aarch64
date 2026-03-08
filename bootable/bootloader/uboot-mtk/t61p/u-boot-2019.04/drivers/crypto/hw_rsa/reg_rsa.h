/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _REG_RSA_H_
#define _REG_RSA_H_

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
#define REG_SHARNGCTRL_BASE         (0x1C538800UL) // 538400UL -> 1C538800UL // 29C2 -> 29C4

#define RSA_E_BASE_ADDR             (0x00UL)
#define RSA_N_BASE_ADDR             (0x40UL)
#define RSA_A_BASE_ADDR             (0x80UL)
#define RSA_Z_BASE_ADDR             (0xC0UL)

typedef struct _REG32
{
    volatile unsigned short         L;
    volatile unsigned short         empty_L;
    volatile unsigned short         H;
    volatile unsigned short         empty_H;
} REG32;

typedef struct _REG_CipherKey
{
    REG32                           Key_L;
    REG32                           Key_H;
} REG_CipherKey;

typedef struct _REG_InitVector
{
    REG32                           IV_L;
    REG32                           IV_H;
} REG_InitVector;

typedef struct _REG_SHARNGCtrl
{
    REG32               Rng_Ctrl;                                   //0x00
        #define SHARNG_CTRL_RNG_SW_RST              0x00000080UL
        #define MOBF_IN_MIU_READ_EN                 0x00010000UL
        #define MOBF_IN_MIU_WRITE_EN                0x00020000UL
        #define AES_FILE_IN_MIU_NS                  0x00100000UL
        #define AES_FILE_OUT_MIU_NS                 0x00200000UL
        #define MOBF_ONEWAY_EN                      0x01000000UL

        #define SECRET_KEY_IN_NORMAL_BANK           0x08000000UL

    REG32               Rng_Out;                                    //0x02
    REG32               MOBF_KeyR;                                  //0x04
    REG32               MOBF_KeyW;                                  //0x06
    REG32               Sha_Ctrl;                                   //0x08
        #define SHARNG_CTRL_SHA_FIRE_ONCE           0x00000001UL
        #define SHARNG_CTRL_SHA_CLR                 0x00000040UL
        #define SHARNG_CTRL_SHA_RST                 0x00000080UL
        #define SHARNG_CTRL_SHA_INT                 0x00000100UL
        #define SHARNG_CTRL_SHA_SEL_SHA256          0x00000200UL
        #define SHARNG_CTRL_SHA_BYPASS_TABLE_EN     0x00000800UL
        #define SHARNG_CTRL_SHA_INITIAL_HASH_EN     0x00002000UL
        #define SHARNG_CTRL_SHA_WORKMODE_MANUAL_EN  0x00004000UL
        #define SHARNG_CTRL_SHA_MSG_INVERSE         0x00040000UL
        #define SHARNG_CTRL_SHA_MSG_BLOCK_NUM       0x00010000UL /* Always be 1 */
        #define SHARNG_CTRL_SPEED_MODE_N		    0x00000010UL
        #define SHARNG_CTRL_REMOVE_SCATTER_GATHER	0x00000800UL

    REG32               Sha_Start;                                  //0x0a
    REG32               Sha_Length;                                 //0x0c
    REG32               Sha_Status;                                 //0x0e
        #define SHARNG_CTRL_SHA_BUSY                0x00020000UL
        #define SHARNG_CTRL_SHA_READY               0x00010000UL
        #define SHARNG_CTRL_SHA_SEL_SHA384          0x00000400UL        //jerry
        #define SHARNG_CTRL_SHA_SEL_SHA512          0x00000800UL        //jerry

    REG32               Sha_Out[8];                                 //0x10~0x1E
    REG32               Rsa_Ind32_Start;                            //0x20
        #define RSA_INDIRECT_START                  0x00000001UL
        #define RSA_IND32_CTRL_DIRECTION_WRITE      0x00020000UL
        #define RSA_IND32_CTRL_ADDR_AUTO_INC        0x00040000UL
        #define RSA_IND32_CTRL_ACCESS_AUTO_START    0x00080000UL
    REG32               Rsa_Ind32_Addr;                             //0x22
        #define RSA_ADDRESS_MASK                    0x0000FFFFUL
        #define RSA_WDATA_MASK_L                    0xFFFF0000UL
    REG32               Rsa_Ind32_WData;                            //0x24
        #define RSA_WDATA_MASK_H                    0x0000FFFFUL
        #define RSA_RDATA_MASK_L                    0xFFFF0000UL
    REG32               Rsa_Ind32_RData;                            //0x26
        #define RSA_RDATA_MASK_H                    0x0000FFFFUL
        #define RSA_EXP_START                       0x00010000UL
        #define RSA_INT_CLR                         0x00020000UL
        #define SHARNG_CTRL_SHA_SEL_SHA384          0x00000400UL
        #define SHARNG_CTRL_SHA_SEL_SHA512          0x00000800UL
    REG32               Rsa_Ctrl;                                   //0x28
        #define RSA_CTRL_RSA_RST                    0x00000001UL
        #define RSA_CTRL_SEL_HW_KEY                 0x00000002UL
        #define RSA_CTRL_SEL_PUBLIC_KEY             0x00000004UL
        #define RSA_CTRL_KEY_LENGTH_MASK            0x00003F00UL
        #define RSA_STATUS_RSA_BUSY                 0x00010000UL
        #define RSA_STATUS_MASK                     0x00FF0000UL
}REG_SHARNGCtrl;

#endif // #ifndef _REG_RSA_H_
