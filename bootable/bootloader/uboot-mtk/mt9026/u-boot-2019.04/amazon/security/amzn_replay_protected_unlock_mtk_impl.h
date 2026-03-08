/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */

#include <amzn_replay_protected_unlock.h>

#ifndef _AMZN_REPLAY_PROTECTED_UNLOCK_MTK_IMPL_H_
#define _AMZN_REPLAY_PROTECTED_UNLOCK_MTK_IMPL_H_

#define TA_AMZN_UNLOCK_UUID { 0x676054fe, 0xc976, 0x4b14, \
				{ 0xa1, 0xa5, 0x52, 0x86, 0x8f, 0x8f, 0x1e, 0x0b } }

// CMD for Yubikey public key tag
#define CMD_AMZN_UNLOCK_READ_PUB_KEY_TAG         1
#define CMD_AMZN_UNLOCK_WRITE_PUB_KEY_TAG        2
// CMD for Replay Protected Unlock
#define CMD_AMZN_UNLOCK_READ_RPU_NONCE           3
#define CMD_AMZN_UNLOCK_WRITE_RPU_NONCE          4

#endif    //_AMZN_REPLAY_PROTECTED_UNLOCK_MTK_IMPL_H_
