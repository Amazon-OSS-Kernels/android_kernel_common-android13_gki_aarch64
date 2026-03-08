// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
#include <common.h>
#include <dm.h>
#include <image-njpd.h>

EN_JPEG_Result njpd_start_decode(struct udevice *dev,
                                 JPEG_Buf read_buf,
                                 JPEG_Buf write_buf,
                                 JPEG_Buf inter_buf,
                                 JPEG_Pic max_resolution,
                                 JPEG_FillDataFunc pFillDataFunc)
{
    if (!dev)
        return E_JPEG_FAILED;

    JPEG_Ops *ops = njpd_get_ops(dev);
    if (!ops || !ops->start_decode)
        return E_JPEG_FAILED;
    return ops->start_decode(dev, read_buf, write_buf, inter_buf, max_resolution, pFillDataFunc);
}

EN_JPEG_DecodeState njpd_wait_decode_done(struct udevice *dev)
{
    if (!dev)
        return E_JPEG_FAILED;

    JPEG_Ops *ops = njpd_get_ops(dev);
    if (!ops || !ops->wait_decode_done)
        return E_JPEG_FAILED;
    return ops->wait_decode_done(dev);
}

EN_JPEG_Result njpd_stop_decode(struct udevice *dev)
{
    if (!dev)
        return E_JPEG_FAILED;

    JPEG_Ops *ops = njpd_get_ops(dev);
    if (!ops || !ops->stop_decode)
        return E_JPEG_FAILED;
    return ops->stop_decode(dev);
}

EN_JPEG_Result njpd_get_alignment(struct udevice *dev, JPEG_Alignment *alignment)
{
    if (!dev)
        return E_JPEG_FAILED;

    JPEG_Ops *ops = njpd_get_ops(dev);
    if (!ops || !ops->get_alignment)
        return E_JPEG_FAILED;
    return ops->get_alignment(dev, alignment);
}

UCLASS_DRIVER(mtk_njpd) = {
	.id		= UCLASS_NJPD,
	.name	= "mtk-njpd",
};
