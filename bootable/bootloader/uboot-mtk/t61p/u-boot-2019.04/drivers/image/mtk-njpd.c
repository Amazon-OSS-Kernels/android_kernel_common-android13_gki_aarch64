// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <image-njpd.h>

#include "mtk_api_jpd.h"
#include "mtk-njpd.h"

//#define DEBUG //for open this file debug("xxx") log
#define BUF_PARTITION 2

static JPEG_Buf g_inter_buf;
static JPEG_Buf g_read_buf_high;
static JPEG_Buf g_read_buf_low;
static JPEG_Buf *g_read_buf_0;
static JPEG_Buf *g_read_buf_1;
static JPEG_Alignment g_alignment;
static EN_NJPD_BUF_PART g_read_buf_eof;
static JPEG_FillDataFunc fill_data_cb;

static EN_JPEG_Result _njpd_priv_to_init_param(MTK_NJPD_PRIV *njpd_priv, NJPD_API_InitParam *init_param)
{
    if (!njpd_priv || !init_param)
        return E_JPEG_FAILED;

    init_param->jpd_reg_base = njpd_priv->jpd_reg_base;
    init_param->jpd_ext_reg_base = njpd_priv->jpd_ext_reg_base;
    init_param->clk_njpd = &njpd_priv->clk_njpd;
    init_param->clk_smi2jpd = &njpd_priv->clk_smi2jpd;
    init_param->clk_njpd2jpd = &njpd_priv->clk_njpd2jpd;
    init_param->jpd_irq = -1;
    return E_JPEG_OKAY;
}

static EN_JPEG_Result _jpeg_buf_to_api_buf(const JPEG_Buf *jpeg_buf, NJPD_API_Buf *api_buf)
{
    if (!jpeg_buf || !api_buf)
        return E_JPEG_FAILED;

    api_buf->va = jpeg_buf->va;
    api_buf->pa = jpeg_buf->pa;
    api_buf->offset = 0;
    api_buf->filled_length = jpeg_buf->filled_length;
    api_buf->size = jpeg_buf->size;
    return E_JPEG_OKAY;
}

static EN_JPEG_Result _jpeg_pic_to_api_pic(const JPEG_Pic *jpeg_pic, NJPD_API_Pic *api_pic)
{
    if (!jpeg_pic || !api_pic)
        return E_JPEG_FAILED;

    api_pic->width = jpeg_pic->width;
    api_pic->height = jpeg_pic->height;
    return E_JPEG_OKAY;
}

static NJPD_API_Result _alloc_inter_buf(size_t size, NJPD_API_Buf *buf, void *priv)
{
    if (g_inter_buf.size < size)
        return E_NJPD_API_FAILED;
    return _jpeg_buf_to_api_buf(&g_inter_buf, buf);
}

static NJPD_API_Result _free_inter_buf(NJPD_API_Buf buf, void *priv)
{
    return E_NJPD_API_OKAY;
}

static EN_JPEG_Result _fill_and_feed_data(struct udevice *dev, EN_NJPD_BUF_PART buf_part)
{
    EN_JPEG_Result ret;
    JPEG_Buf *read_buf;
    NJPD_API_Buf api_buf;
    size_t filled_length;
    bool eof;

    // Check if read_buf need to be filled
    if (g_read_buf_eof != E_NJPD_BUF_NONE) {
        dev_err(dev, "[%s] EOF in read_buf\n", __func__);
        return E_JPEG_OKAY;
    }

    // Choose active buffer
    if (buf_part == E_NJPD_BUF_0)
        read_buf = g_read_buf_0;
    else if (buf_part == E_NJPD_BUF_1)
        read_buf = g_read_buf_1;
    else {
        dev_err(dev, "[%s] Invalid buffer part\n", __func__);
        return E_JPEG_FAILED;
    }

    // Fill data callback
    ret = fill_data_cb(*read_buf, &filled_length, &eof);
    debug("[mtk_njpd] %s:%d: fill_data_cb, ret=%d, filled_length=0x%lx eof=%d\n", __FUNCTION__, __LINE__, ret, filled_length, eof);
    if (ret != E_JPEG_OKAY) {
        dev_err(dev, "[%s] fill_data_cb FAIL\n", __func__);
        return ret;
    }
    read_buf->filled_length = filled_length;
    if (eof)
        g_read_buf_eof = buf_part;

    // Decoder feed data
    flush_dcache_range((__u64)read_buf->va, (__u64)(read_buf->va + read_buf->size));

    if (_jpeg_buf_to_api_buf(read_buf, &api_buf) != E_JPEG_OKAY) {
        dev_err(dev, "[%s] _jpeg_buf_to_api_buf FAIL\n", __func__);
        return E_JPEG_FAILED;
    }

    if (MApi_NJPD_Decoder_FeedData(api_buf) != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Decoder_FeedData FAIL\n", __func__);
        return E_JPEG_FAILED;
    }

    // Check EOF to notify no more data
    if (g_read_buf_eof == buf_part) {
        if (MApi_NJPD_Decoder_NoMoreData() != E_NJPD_API_OKAY) {
            dev_err(dev, "[%s] MApi_NJPD_Decoder_NoMoreData FAIL\n", __func__);
            return E_JPEG_FAILED;
        }
    }

    return E_JPEG_OKAY;
}

static EN_JPEG_Result mtk_njpd_start_decode(struct udevice *dev,
                                            JPEG_Buf read_buf,
                                            JPEG_Buf write_buf,
                                            JPEG_Buf inter_buf,
                                            JPEG_Pic max_resolution,
                                            JPEG_FillDataFunc pFillDataFunc)
{
    EN_JPEG_Result ret;
    NJPD_API_Buf api_buf;
    NJPD_API_Pic api_pic;
    int handle;

    debug("[mtk_njpd] %s:%d: IN\n", __FUNCTION__, __LINE__);

    if (!pFillDataFunc || !dev) {
        dev_err(dev, "[%s] param FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    fill_data_cb = pFillDataFunc;
    debug("[mtk_njpd] %s:%d: pFillDataFunc = 0x%p\n", __FUNCTION__, __LINE__, pFillDataFunc);

    // NJPD init
    NJPD_API_InitParam pInitParam;
    MTK_NJPD_PRIV *priv = dev_get_priv(dev);
    ret = _njpd_priv_to_init_param(priv, &pInitParam);
    if (ret != E_JPEG_OKAY) {
        dev_err(dev, "[%s] _njpd_priv_to_init_param FAIL\n", __func__);
        return ret;
    }
    debug("[mtk_njpd] %s:%d: _njpd_priv_to_init_param OK\n", __FUNCTION__, __LINE__);

    if (MApi_NJPD_Init(&pInitParam) != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Init FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    debug("[mtk_njpd] %s:%d: MApi_NJPD_Init OK\n", __FUNCTION__, __LINE__);

    // Parser open
    memcpy(&g_inter_buf, &inter_buf, sizeof(JPEG_Buf));
    NJPD_API_ParserInitParam pParserInitParam;
    memset(&pParserInitParam, 0, sizeof(NJPD_API_ParserInitParam));
    pParserInitParam.pAllocBufFunc = _alloc_inter_buf;
    pParserInitParam.pFreeBufFunc = _free_inter_buf;

    if (MApi_NJPD_Parser_Open(&handle, &pParserInitParam) != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Parser_Open FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    debug("[mtk_njpd] %s:%d: MApi_NJPD_Parser_Open OK\n", __FUNCTION__, __LINE__);

    // Set max resolution
    if (max_resolution.width != 0 && max_resolution.height != 0) {
        ret = _jpeg_pic_to_api_pic(&max_resolution, &api_pic);
        if (ret != E_JPEG_OKAY) {
            dev_err(dev, "[%s] _jpeg_pic_to_api_pic FAIL\n", __func__);
            return ret;
        }

        if (MApi_NJPD_Parser_SetMaxResolution(handle, api_pic) != E_NJPD_API_OKAY) {
            dev_err(dev, "[%s] MApi_NJPD_Parser_SetMaxResolution FAIL\n", __func__);
            return E_JPEG_FAILED;
        }
        debug("[mtk_njpd] %s:%d: MApi_NJPD_Parser_SetMaxResolution OK\n", __FUNCTION__, __LINE__);
    }

    // Split read_buf into g_read_buf_high and g_read_buf_low
    g_read_buf_high.va = read_buf.va;
    g_read_buf_high.pa = read_buf.pa;
    g_read_buf_high.size = read_buf.size/BUF_PARTITION;
    g_read_buf_high.filled_length = (read_buf.filled_length < g_read_buf_high.size)
                                ? read_buf.filled_length : g_read_buf_high.size;
    debug("[mtk_njpd] %s:%d: g_read_buf_high (va, pa, size, filled_length)=(0x%p, 0x%llx, 0x%lx, 0x%lx)\n",
        __FUNCTION__, __LINE__, g_read_buf_high.va, g_read_buf_high.pa, g_read_buf_high.size, g_read_buf_high.filled_length);

    g_read_buf_low.va = read_buf.va + g_read_buf_high.size;
    g_read_buf_low.pa = read_buf.pa + g_read_buf_high.size;
    g_read_buf_low.size = read_buf.size - g_read_buf_high.size;
    g_read_buf_low.filled_length = read_buf.filled_length - g_read_buf_high.filled_length;
    debug("[mtk_njpd] %s:%d: g_read_buf_low (va, pa, size, filled_length)=(0x%p, 0x%llx, 0x%lx, 0x%lx)\n",
        __FUNCTION__, __LINE__, g_read_buf_low.va, g_read_buf_low.pa, g_read_buf_low.size, g_read_buf_low.filled_length);

    // Check buffer if eof
    if (g_read_buf_low.filled_length == 0) {
        g_read_buf_eof = E_NJPD_BUF_HIGH;
    } else if (g_read_buf_low.filled_length < g_read_buf_low.size) {
        g_read_buf_eof = E_NJPD_BUF_LOW;
    } else {
        g_read_buf_eof = E_NJPD_BUF_NONE;
    }
    debug("[mtk_njpd] %s:%d: Check buffer if eof before parse heaser, g_read_buf_eof = %d\n", __FUNCTION__, __LINE__, g_read_buf_eof);

    // Start parse header loop
    EN_NJPD_BUF_PART read_buf_active = E_NJPD_BUF_HIGH;
    NJPD_API_DecCmd dec_cmd;
    while (1) {
        NJPD_API_ParserResult parserRet = E_NJPD_API_HEADER_INVALID;
        switch (read_buf_active) {
            case E_NJPD_BUF_HIGH:
                ret = _jpeg_buf_to_api_buf(&g_read_buf_high, &api_buf);
                if (ret == E_JPEG_OKAY)
                    parserRet = MApi_NJPD_Parser_ParseHeader(handle, api_buf, &dec_cmd);
                break;
            case E_NJPD_BUF_LOW:
                ret = _jpeg_buf_to_api_buf(&g_read_buf_low, &api_buf);
                if (ret == E_JPEG_OKAY)
                    parserRet = MApi_NJPD_Parser_ParseHeader(handle, api_buf, &dec_cmd);
                break;
            default:
                parserRet = E_NJPD_API_HEADER_INVALID;
                dev_err(dev, "[%s] Input buffer is not active.\n", __func__);
                break;
        }
        debug("[mtk_njpd] %s:%d: MApi_NJPD_Parser_ParseHeader parserRet = %d\n", __FUNCTION__, __LINE__, parserRet);

        if (parserRet == E_NJPD_API_HEADER_INVALID) {
            dev_err(dev, "[%s] E_NJPD_API_HEADER_INVALID\n", __func__);
            return E_JPEG_FAILED;
        } else if (parserRet == E_NJPD_API_HEADER_VALID) {
            debug("[mtk_njpd] %s:%d: E_NJPD_API_HEADER_VALID\n", __FUNCTION__, __LINE__);
            break;
        } else {  // E_NJPD_API_HEADER_PARTIAL
            debug("[mtk_njpd] %s:%d: E_NJPD_API_HEADER_PARTIAL, read_buf_active=%d, g_read_buf_eof=%d\n",
                __FUNCTION__, __LINE__, read_buf_active, g_read_buf_eof);
            if (read_buf_active == E_NJPD_BUF_HIGH) {
                // Set read_buf_active to E_NJPD_BUF_NONE if there is no more data to parse.
                if (g_read_buf_eof == E_NJPD_BUF_HIGH)
                    read_buf_active = E_NJPD_BUF_NONE;
                else
                    read_buf_active = E_NJPD_BUF_LOW;
                // Call fill_data_cb to fill read buffer
                if (g_read_buf_eof == E_NJPD_BUF_NONE) {
                    size_t filled_length;
                    bool eof;
                    ret = fill_data_cb(g_read_buf_high, &filled_length, &eof);
                    debug("[mtk_njpd] %s:%d: fill_data_cb, ret=%d, filled_length=0x%lx eof=%d\n", __FUNCTION__, __LINE__, ret, filled_length, eof);
                    if (ret == E_JPEG_OKAY) {
                        g_read_buf_high.filled_length = filled_length;
                        if (eof)
                            g_read_buf_eof = E_NJPD_BUF_HIGH;
                    }
                }
            } else if (read_buf_active == E_NJPD_BUF_LOW) {
                // Set read_buf_active to E_NJPD_BUF_NONE if there is no more data to parse.
                if (g_read_buf_eof == E_NJPD_BUF_LOW)
                    read_buf_active = E_NJPD_BUF_NONE;
                else
                    read_buf_active = E_NJPD_BUF_HIGH;
                // Call fill_data_cb to fill read buffer
                if (g_read_buf_eof == E_NJPD_BUF_NONE) {
                    size_t filled_length;
                    bool eof;
                    ret = fill_data_cb(g_read_buf_low, &filled_length, &eof);
                    debug("[mtk_njpd] %s:%d: fill_data_cb, ret=%d, filled_length=0x%lx eof=%d\n", __FUNCTION__, __LINE__, ret, filled_length, eof);
                    if (ret == E_JPEG_OKAY) {
                        g_read_buf_low.filled_length = filled_length;
                        if (eof)
                            g_read_buf_eof = E_NJPD_BUF_LOW;
                    }
                }
            } else {
                dev_err(dev, "[%s] Unknown error!!!\n", __func__);
                return E_JPEG_FAILED;
            }
        }
    }

    // Get alignment
    NJPD_API_ParserImageInfo imageInfo;
    NJPD_API_ParserBufInfo bufInfo;

    if (MApi_NJPD_Parser_GetImageInfo(handle, &imageInfo) != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Parser_GetImageInfo FAIL\n", __func__);
        return E_JPEG_FAILED;
    }

    if (MApi_NJPD_Parser_GetBufInfo(handle, &bufInfo) != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Parser_GetBufInfo FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    g_alignment.width = imageInfo.width;
    g_alignment.height= imageInfo.height;
    g_alignment.pitch = bufInfo.width;
    debug("[mtk_njpd] %s:%d: Alignment width=%u, height=%u, pitch=%u\n",
        __FUNCTION__, __LINE__, g_alignment.width, g_alignment.height, g_alignment.pitch);

    // Parser close
    if (MApi_NJPD_Parser_Close(handle) != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Parser_Close FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    debug("[mtk_njpd] %s:%d: MApi_NJPD_Parser_Close OK\n", __FUNCTION__, __LINE__);

    // The read_buf with valid header is g_read_buf_0
    if (read_buf_active == E_NJPD_BUF_HIGH) {
        g_read_buf_0 = &g_read_buf_high;
        g_read_buf_1 = &g_read_buf_low;
        if (g_read_buf_eof == E_NJPD_BUF_HIGH)
            g_read_buf_eof = E_NJPD_BUF_0;
        else if (g_read_buf_eof == E_NJPD_BUF_LOW)
            g_read_buf_eof = E_NJPD_BUF_1;
        else
            g_read_buf_eof = E_NJPD_BUF_NONE;
    } else {    // read_buf_active == E_NJPD_BUF_LOW
        g_read_buf_0 = &g_read_buf_low;
        g_read_buf_1 = &g_read_buf_high;
        if (g_read_buf_eof == E_NJPD_BUF_HIGH)
            g_read_buf_eof = E_NJPD_BUF_1;
        else if (g_read_buf_eof == E_NJPD_BUF_LOW)
            g_read_buf_eof = E_NJPD_BUF_0;
        else
            g_read_buf_eof = E_NJPD_BUF_NONE;
    }
    debug("[mtk_njpd] %s:%d: Convert read_buf, g_read_buf_eof=%d\n", __FUNCTION__, __LINE__, g_read_buf_eof);

    // Decoder start
    if (g_read_buf_0 == NULL || g_read_buf_1 == NULL) {
        dev_err(dev, "[%s] Read buffer error!!!\n", __func__);
        return E_JPEG_FAILED;
    }
    flush_dcache_range((__u64)g_read_buf_0->va, (__u64)g_read_buf_0->va + g_read_buf_0->size);
    flush_dcache_range((__u64)g_inter_buf.va, (__u64)g_inter_buf.va + g_inter_buf.size);
    ret = _jpeg_buf_to_api_buf(&write_buf, &api_buf);
    if (ret != E_JPEG_OKAY) {
        dev_err(dev, "[%s] _jpeg_buf_to_api_buf FAIL\n", __func__);
        return ret;
    }
    if (MApi_NJPD_Decoder_Start(&dec_cmd, api_buf) != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Decoder_Start FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    debug("[mtk_njpd] %s:%d: MApi_NJPD_Decoder_Start OK\n", __FUNCTION__, __LINE__);

    // Check EOF of g_read_buf_0 in dec_cmd
    if (g_read_buf_eof == E_NJPD_BUF_0) {
        if (MApi_NJPD_Decoder_NoMoreData() != E_NJPD_API_OKAY) {
            dev_err(dev, "[%s] MApi_NJPD_Decoder_NoMoreData FAIL\n", __func__);
            return E_JPEG_FAILED;
        }
        debug("[mtk_njpd] %s:%d: MApi_NJPD_Decoder_NoMoreData OK\n", __FUNCTION__, __LINE__);
    } else {
        // Decoder feed data
        flush_dcache_range((__u64)g_read_buf_1->va, (__u64)g_read_buf_1->va + g_read_buf_1->size);
        ret = _jpeg_buf_to_api_buf(g_read_buf_1, &api_buf);
        if (ret != E_JPEG_OKAY) {
            dev_err(dev, "[%s] _jpeg_buf_to_api_buf FAIL\n", __func__);
            return ret;
        }

        if (MApi_NJPD_Decoder_FeedData(api_buf) != E_NJPD_API_OKAY) {
            dev_err(dev, "[%s] MApi_NJPD_Decoder_FeedData FAIL\n", __func__);
            return E_JPEG_FAILED;
        }
        debug("[mtk_njpd] %s:%d: MApi_NJPD_Decoder_FeedData OK\n", __FUNCTION__, __LINE__);

        // Check EOF in g_read_buf_1
        if (g_read_buf_eof == E_NJPD_BUF_1) {
            if (MApi_NJPD_Decoder_NoMoreData() != E_NJPD_API_OKAY) {
                dev_err(dev, "[%s] MApi_NJPD_Decoder_NoMoreData FAIL\n", __func__);
                return E_JPEG_FAILED;
            }
            debug("[mtk_njpd] %s:%d: MApi_NJPD_Decoder_NoMoreData OK\n", __FUNCTION__, __LINE__);
        }
    }

    debug("[mtk_njpd] %s:%d: OUT\n", __FUNCTION__, __LINE__);
    return ret;
}

static EN_JPEG_DecodeState mtk_njpd_wait_decode_done(struct udevice *dev)
{
    EN_JPEG_Result ret;
    EN_JPEG_DecodeState decodeRet;

    NJPD_API_DecodeStatus decode_status = MApi_NJPD_Decoder_WaitDone();
    switch (decode_status) {
        case E_NJPD_API_DEC_FAILED:
            debug("[mtk_njpd] %s:%d: decode_status = E_NJPD_API_DEC_FAILED\n", __FUNCTION__, __LINE__);
            decodeRet = E_JPEG_DEC_FAILED;
            break;
        case E_NJPD_API_DEC_DONE:
            debug("[mtk_njpd] %s:%d: decode_status = E_NJPD_API_DEC_DONE\n", __FUNCTION__, __LINE__);
            decodeRet = E_JPEG_DEC_DONE;
            break;
        case E_NJPD_API_DEC_DECODING:
            debug("[mtk_njpd] %s:%d: decode_status = E_NJPD_API_DEC_DECODING\n", __FUNCTION__, __LINE__);
            decodeRet = E_JPEG_DEC_DECODING;
            break;
        case E_NJPD_API_DEC_BUF_EMPTY_0:
            debug("[mtk_njpd] %s:%d: decode_status = E_NJPD_API_DEC_BUF_EMPTY_0\n", __FUNCTION__, __LINE__);
            ret = _fill_and_feed_data(dev, E_NJPD_BUF_0);
            if (ret != E_JPEG_OKAY) {
                decodeRet = E_JPEG_DEC_FAILED;
                break;
            }
            decodeRet = E_JPEG_DEC_PARTIAL_DONE;
            break;
        case E_NJPD_API_DEC_BUF_EMPTY_1:
            debug("[mtk_njpd] %s:%d: decode_status = E_NJPD_API_DEC_BUF_EMPTY_1\n", __FUNCTION__, __LINE__);
            ret = _fill_and_feed_data(dev, E_NJPD_BUF_1);
            if (ret != E_JPEG_OKAY) {
                decodeRet = E_JPEG_DEC_FAILED;
                break;
            }
            decodeRet = E_JPEG_DEC_PARTIAL_DONE;
            break;
        case E_NJPD_API_DEC_BUF_EMPTY_01:
            debug("[mtk_njpd] %s:%d: decode_status = E_NJPD_API_DEC_BUF_EMPTY_01\n", __FUNCTION__, __LINE__);
            ret = _fill_and_feed_data(dev, E_NJPD_BUF_0);
            if (ret != E_JPEG_OKAY) {
                decodeRet = E_JPEG_DEC_FAILED;
                break;
            }
            ret = _fill_and_feed_data(dev, E_NJPD_BUF_1);
            if (ret != E_JPEG_OKAY) {
                decodeRet = E_JPEG_DEC_FAILED;
                break;
            }
            decodeRet = E_JPEG_DEC_PARTIAL_DONE;
            break;
        case E_NJPD_API_DEC_BUF_EMPTY_10:
            debug("[mtk_njpd] %s:%d: decode_status = E_NJPD_API_DEC_BUF_EMPTY_10\n", __FUNCTION__, __LINE__);
            ret = _fill_and_feed_data(dev, E_NJPD_BUF_1);
            if (ret != E_JPEG_OKAY) {
                decodeRet = E_JPEG_DEC_FAILED;
                break;
            }
            ret = _fill_and_feed_data(dev, E_NJPD_BUF_0);
            if (ret != E_JPEG_OKAY) {
                decodeRet = E_JPEG_DEC_FAILED;
                break;
            }
            decodeRet = E_JPEG_DEC_PARTIAL_DONE;
            break;
        default:
            debug("[mtk_njpd] %s:%d: Unknown decode state\n", __FUNCTION__, __LINE__);
            decodeRet = E_JPEG_DEC_FAILED;
            break;
    }

    return decodeRet;
}

static EN_JPEG_Result mtk_njpd_stop_decode(struct udevice *dev)
{
    EN_JPEG_Result ret = E_JPEG_OKAY;

    debug("[mtk_njpd] %s:%d: IN\n", __FUNCTION__, __LINE__);

    if (MApi_NJPD_Decoder_Stop() != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Decoder_Stop FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    debug("[mtk_njpd] %s:%d: MApi_NJPD_Decoder_Stop OK\n", __FUNCTION__, __LINE__);

    if (MApi_NJPD_Deinit() != E_NJPD_API_OKAY) {
        dev_err(dev, "[%s] MApi_NJPD_Deinit FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    debug("[mtk_njpd] %s:%d: MApi_NJPD_Deinit OK\n", __FUNCTION__, __LINE__);

    debug("[mtk_njpd] %s:%d: OUT\n", __FUNCTION__, __LINE__);
    return ret;
}

static EN_JPEG_Result mtk_njpd_get_alignment(struct udevice *dev, JPEG_Alignment *alignment)
{
    EN_JPEG_Result ret = E_JPEG_OKAY;

    debug("[mtk_njpd] %s:%d: IN\n", __FUNCTION__, __LINE__);
    if (!alignment || !dev) {
        dev_err(dev, "[%s] param FAIL\n", __func__);
        return E_JPEG_FAILED;
    }
    *alignment = g_alignment;
    debug("[mtk_njpd] %s:%d: Get alignment width=%u, height=%u, pitch=%u\n",
        __FUNCTION__, __LINE__, alignment->width, alignment->height, alignment->pitch);

    debug("[mtk_njpd] %s:%d: OUT\n", __FUNCTION__, __LINE__);
    return ret;
}

static int mtk_njpd_ofdata_to_platdata(struct udevice *dev)
{
    MTK_NJPD_PRIV *priv;

    debug("[mtk_njpd] %s:%d: device name=%s\n", __FUNCTION__, __LINE__, dev->name);
    if (!dev) {
        dev_err(dev, "[%s] No device\n", __func__);
        return -ENODEV;
    }

    priv = dev_get_priv(dev);
    if (!priv) {
        dev_err(dev, "[%s] NULL private data\n", __func__);
        return -ENOMEM;
    }

    priv->jpd_reg_base = (void *)dev_read_addr_index(dev, 0);
    debug("[mtk_njpd] %s:%d: jpd_reg_base=0x%p\n", __FUNCTION__, __LINE__, priv->jpd_reg_base);
    priv->jpd_ext_reg_base = (void *)dev_read_addr_index(dev, 1);
    debug("[mtk_njpd] %s:%d: jpd_ext_reg_base=0x%p\n", __FUNCTION__, __LINE__, priv->jpd_ext_reg_base);
/*
    // No use below

    strcpy(priv->name, dev_read_string(dev, "file-name"));
    debug("[mtk_njpd] %s:%d: file name=%s\n", __FUNCTION__, __LINE__, priv->name);

    ret = clk_get_by_name(dev, "clk_njpd", &priv->clk_njpd);
    if (ret < 0) {
        return ret;
    }
    ret = clk_get_by_name(dev, "clk_smi2jpd", &priv->clk_smi2jpd);
    if (ret < 0) {
        return ret;
    }
    ret = clk_get_by_name(dev, "clk_njpd2jpd", &priv->clk_njpd2jpd);
    if (ret < 0) {
        return ret;
    }

    priv->read_buffer = dev_read_u32_default(dev, "read-buf-start", 0);
    priv->read_size = dev_read_u32_default(dev, "read-buf-size", 0);
    debug("[mtk_njpd] %s:%d: read_buf_addr=0x%x, read_buf_size=0x%x\n", __FUNCTION__, __LINE__, priv->read_buffer, priv->read_size);

    priv->inter_buffer = dev_read_u32_default(dev, "inter-buf-start", 0);
    priv->inter_size = dev_read_u32_default(dev, "inter-buf-size", 0);
    debug("[mtk_njpd] %s:%d: inter_buf_addr=0x%x, inter_buf_size=0x%x\n", __FUNCTION__, __LINE__, priv->inter_buffer, priv->inter_size);

    priv->output_buffer = dev_read_u32_default(dev, "out-buf-start", 0);
    priv->output_size = dev_read_u32_default(dev, "out-buf-size", 0);
    debug("[mtk_njpd] %s:%d: write_buf_addr=0x%x, write_buf_size=0x%x\n", __FUNCTION__, __LINE__, priv->output_buffer, priv->output_size);
*/
    return 0;
}

static int mtk_njpd_probe(struct udevice *dev)
{
    debug("[mtk_njpd] %s:%d: INIT\n", __FUNCTION__, __LINE__);
    return 0;
}

static const JPEG_Ops mtk_njpd_ops = {
    .start_decode       = mtk_njpd_start_decode,
    .wait_decode_done   = mtk_njpd_wait_decode_done,
    .stop_decode        = mtk_njpd_stop_decode,
    .get_alignment      = mtk_njpd_get_alignment,
};

static const struct udevice_id mtk_njpd_ids[] = {
    { .compatible = "mediatek,mtk_njpd" },
    { },
};

U_BOOT_DRIVER(mediatek_njpd) = {
    .name = "mtk-njpd",
    .id = UCLASS_NJPD,
    .of_match = mtk_njpd_ids,
    .ops = &mtk_njpd_ops,
    .ofdata_to_platdata = mtk_njpd_ofdata_to_platdata,
    .probe = mtk_njpd_probe,
    .priv_auto_alloc_size = sizeof(MTK_NJPD_PRIV),
};
