/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#ifndef __UFBL_DSTSBLT_H
#define __UFBL_DSTSBLT_H

#ifdef AMZN_DSTSBLT
typedef enum {
    /* Range from 0x00 to 0x7F is reserved for lk */

    DSTSBLT_CP_UNLOCK_START = 0x80,
    DSTSBLT_CP_UNLOCK_CODEPRESENT = 0x81,
    DSTSBLT_CP_UNLOCK0_END = 0x82,
    DSTSBLT_CP_UNLOCK1_END = 0x83,
    DSTSBLT_CP_RELOCK_DONE = 0x84,
    DSTSBLT_CP_TUCERT_PRESENT = 0x85,
    DSTSBLT_CP_TUCODE_PRESENT = 0x86,
    DSTSBLT_CP_TU_CHECK_DONE = 0x87,
    DSTSBLT_CP_UNLOCKFLASH_START = 0x88,
    DSTSBLT_CP_UNLOCKFLASH_END = 0x89,

    /* Range from 0xF0 to 0xF7 is reserved for lk errors */

    DSTSBLT_CP_MAX = 0xff
} dstsblt_lk_checkpoint_t;

typedef enum {
    /* Range from 0x0000 to 0x7FFF is reserved for lk */

    DSTSBLT_SECTRIG_FLASHUNLOCK_FAILLEN = 0x8001,
    DSTSBLT_SECTRIG_FLASHUNLOCK_FAILAUTH = 0x8002,
    DSTSBLT_SEC_TRIG_UNLOCK_INVALIDPRESENT = 0x8004,

    DSTSBLT_SECTRIG_TUNLOCK_FAILAUTH = 0x8003,

    DSTSBLT_SECTRIG_NONE = 0xFFFF
} dstsblt_sectrig_id_t;

extern void dstsblt_set_checkpoint(dstsblt_lk_checkpoint_t checkpoint);
extern void dstsblt_sectrig_addentry(dstsblt_sectrig_id_t id, void * data, uint8_t len);
extern void dstsblt_set_unlockstate(int unlocked);

/* Use DSTSBLT_CHECKPOINT to log a valid processing step
   Use DSTSBLT_SECTRIG to notify about a (prevented) security incident*/

#define DSTSBLT_CHECKPOINT(x) dstsblt_set_checkpoint(x)
#define DSTSBLT_SECTRIG(x, y, z) dstsblt_sectrig_addentry(x, y, z)
#else
#define DSTSBLT_CHECKPOINT(x) do {} while(0)
#define DSTSBLT_SECTRIG(x, y, z) do {} while(0)
#endif

#endif
