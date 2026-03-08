#ifndef __IMAGE_NJPD_H
#define __IMAGE_NJPD_H

typedef enum {
    E_JPEG_FAILED   = 0,
    E_JPEG_OKAY     = 1,
} EN_JPEG_Result;

typedef enum {
    E_JPEG_DEC_FAILED       = 0,
    E_JPEG_DEC_DONE         = 1,
    E_JPEG_DEC_DECODING     = 2,
    E_JPEG_DEC_PARTIAL_DONE = 3,
} EN_JPEG_DecodeState;

typedef struct {
    void *va;
    dma_addr_t pa;
    size_t filled_length;
    size_t size;
} JPEG_Buf;

typedef struct {
    u16 width;
    u16 height;
} JPEG_Pic;

typedef struct {
    u32 width;
    u32 height;
    u32 pitch;
} JPEG_Alignment;

typedef EN_JPEG_Result (*JPEG_FillDataFunc) (JPEG_Buf buf, size_t *filled_length, bool *eof);

typedef struct {
    EN_JPEG_Result (*start_decode)(struct udevice *dev,
                                   JPEG_Buf read_buf,
                                   JPEG_Buf write_buf,
                                   JPEG_Buf inter_buf,
                                   JPEG_Pic max_resolution,
                                   JPEG_FillDataFunc pFillDataFunc);
    EN_JPEG_DecodeState (*wait_decode_done)(struct udevice *dev);
    EN_JPEG_Result (*stop_decode)(struct udevice *dev);
    EN_JPEG_Result (*get_alignment)(struct udevice *dev, JPEG_Alignment *alignment);
} JPEG_Ops;

EN_JPEG_Result njpd_start_decode(struct udevice *dev,
                                 JPEG_Buf read_buf,
                                 JPEG_Buf write_buf,
                                 JPEG_Buf inter_buf,
                                 JPEG_Pic max_resolution,
                                 JPEG_FillDataFunc pFillDataFunc);
EN_JPEG_DecodeState njpd_wait_decode_done(struct udevice *dev);
EN_JPEG_Result njpd_stop_decode(struct udevice *dev);
EN_JPEG_Result njpd_get_alignment(struct udevice *dev, JPEG_Alignment *alignment);

#define njpd_get_ops(dev)   ((JPEG_Ops *)(dev)->driver->ops)

#endif /* __IMAGE_NJPD_H */
