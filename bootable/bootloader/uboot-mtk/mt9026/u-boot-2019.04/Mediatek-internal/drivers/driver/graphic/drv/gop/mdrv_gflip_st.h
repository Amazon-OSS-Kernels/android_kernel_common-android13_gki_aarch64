/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef _MDRV_GFLIP_ST_H
#define _MDRV_GFLIP_ST_H

//=============================================================================
// Includs
//=============================================================================

//=============================================================================
// Type and Structure Declaration
//=============================================================================
#define MULTIINFO_NUM               8UL

#define MS_HDR_VERSION  4

#define GOP_OSDBLENDING_DEFAULT_MAX (5)
#define ST_GFLIP_GOP_CSC_PARAM_VERSION (1)

//IO Ctrl struct defines:
typedef enum
{
    /// Color format RGB555 and Blink.
    E_GFLIP_COLOR_RGB555_BLINK    =0,
    /// Color format RGB565.
    E_GFLIP_COLOR_RGB565          =1,
    /// Color format ARGB4444.
    E_GFLIP_COLOR_ARGB4444        =2,
    /// Color format alpha blink.
    E_GFLIP_COLOR_2266      =3,
    /// Color format I8 (256-entry palette).
    E_GFLIP_COLOR_I8              =4,
    /// Color format ARGB8888.
    E_GFLIP_COLOR_ARGB8888        =5,
    /// Color format ARGB1555.
    E_GFLIP_COLOR_ARGB1555        =6,
    /// Color format ARGB8888.  - Andriod format
    E_GFLIP_COLOR_ABGR8888        =7,
    /// Color format RGB555/YUV422.
    E_GFLIP_COLOR_RGB555YUV422    =8,
    /// Color format YUV422.
    E_GFLIP_COLOR_YUV422          =9,
    /// Color format ARGB8888.  - Andriod format
    E_GFLIP_COLOR_RGBA5551        =10,
    /// Color format ARGB8888.  - Andriod format
    E_GFLIP_COLOR_RGBA4444        =11,

    /// Invalid color format.
    E_GFLIP_COLOR_INVALID
} GFLIP_COLOR_TYPE;

typedef union
{
    MS_U8 u8DWinIntInfo;
    struct
    {
        MS_U8 bDWinIntWADR:1;
        MS_U8 bDWinIntPROG:1;
        MS_U8 bDWinIntTF:1;
        MS_U8 bDWinIntBF:1;
        MS_U8 bDWinIntVS:1;
        MS_U8 reserved:3;
    }sDwinIntInfo;
}GFLIP_DWININT_INFO, *PGFLIP_DWININT_INFO;

typedef struct
{
    MS_U32 u32GopIdx; //in
    MS_U32 u32GwinIdx; //in
    MS_PHY u64Addr; //in
    MS_U32 u32TagId; //in
    MS_U32 u32QEntry; //InOut
    MS_U32 u32Result; //Out
}MS_GFLIP_INFO, *PMS_GFLIP_INFO;

typedef struct
{
    MS_U32 u32GopIdx; //in
    MS_U32 u32GwinIdx; //in
    MS_PHY u64Addr; //in
    MS_U32 u32TagId; //in
    MS_U32 u32QEntry; //InOut
    MS_U32 u32Result; //Out
    MS_BOOL bTLBEnable;
    MS_PHY u64TLBAddr;
}MS_TLB_GFLIP_INFO, *PMS_TLB_GFLIP_INFO;

typedef struct
{
    MS_U32 u32GopIdx; //in
    MS_U32 u32GwinIdx; //in
    MS_PHY u64MainAddr; //in
    MS_U32 u32TagId; //in
    MS_U32 u32QEntry; //InOut
    MS_U32 u32Result; //Out
    MS_PHY u64SubAddr; //in
}MS_GFLIP_3DINFO, *PMS_GFLIP_3DINFO;

typedef struct
{
    MS_U32 u32GopIdx; //in
    MS_U32 u32GwinIdx; //in
    MS_PHY u64MainAddr; //in
    MS_U32 u32TagId; //in
    MS_U32 u32QEntry; //InOut
    MS_U32 u32Result; //Out
    MS_PHY u64SubAddr; //in
    MS_BOOL bTLBEnable;
    MS_PHY u64TLBAddr;
}MS_TLB_GFLIP_3DINFO, *PMS_TLB_GFLIP_3DINFO;

typedef struct
{
    MS_U8 u8GopInfoCnt;
    MS_GFLIP_3DINFO astGopInfo[MULTIINFO_NUM];
}MS_GFLIP_MULTIINFO, *PMS_GFLIP_MULTIINFO;

typedef struct
{
    MS_U8 u8GopInfoCnt;
    MS_TLB_GFLIP_3DINFO astTLBGopInfo[MULTIINFO_NUM];
}MS_TLB_GFLIP_MULTIINFO, *PMS_TLB_GFLIP_MULTIINFO;

typedef struct
{
    GFLIP_DWININT_INFO gflipDWinIntInfo;
    MS_BOOL bResetDWinIntInfo;

}MS_GFLIP_DWININT_INFO, *PMS_GFLIP_DWININT_INFO;

typedef struct
{
    GFLIP_DWININT_INFO gflipDWinIntInfo;
    MS_U32  u32Timeout;
    MS_BOOL bResetDWinIntInfo;
}MS_GFLIP_DWININT_INFO2, *PMS_GFLIP_DWININT_INFO2;

typedef struct
{
    MS_BOOL bEnable; //InOut
    MS_U8   u8FrameCount; //Out, value range: 0~3
    MS_U8   u8Result; //Out
}MS_GFLIP_VECAPTURESTATE, *PMS_GFLIP_VECAPTURESTATE;

typedef struct
{
    MS_U32 u32GopIdx; //in
    MS_U32 u32GwinIdx; //in
    MS_U32 u32Result; //Out
}MS_GFLIP_GOPGWINIDX,*PMS_GFLIP_GOPGWINIDX;

typedef struct
{
    MS_PHY u64Addr;
    MS_U16 u16X;
    MS_U16 u16Y;
    MS_U16 u16W;
    MS_U16 u16H;
    MS_U8  u8GopIdx;
    MS_U8  u8GwinIdx;
    GFLIP_COLOR_TYPE clrType;
}MS_GWIN_INFO;

typedef struct
{
    MS_BOOL bCurveChange;  // Curve Change
    MS_U8  u8LumaCurve[16];  // Default luma curve from Supernova DLC.ini
    MS_U8  u8LumaCurve2_a[16];  // Default luma curve 2a  from Supernova DLC.ini
    MS_U8  u8LumaCurve2_b[16];  // Default luma curve 2b  from Supernova DLC.ini
    MS_U8  u8Result; //Out
}MS_DLC_INFO;

typedef struct
{
    MS_BOOL bBLEPointChange;  // BLE Point Change
    MS_U16  u16BLESlopPoint[6];  // Default BLE Slop Point from Supernova DLC.ini
    MS_U8  u8Result; //Out
}MS_BLE_INFO;

typedef struct
{
    MS_U16 u16OrgOPmuxInfo;
    MS_U16 u16OrgOSDBInfo[GOP_OSDBLENDING_DEFAULT_MAX];
}MS_SDR2HDR_INFO;

typedef struct
{
    MS_BOOL bCentralCtrl;
    MS_U8 u8GOPNum;
}ST_HDR_CENTRAL_CTRL_INFO;

typedef struct
{
    MS_BOOL bInitInfoChange;  // Init Info Change

    // Default luma curve
    MS_U8 ucLumaCurve[16];
    MS_U8 ucLumaCurve2_a[16];
    MS_U8 ucLumaCurve2_b[16];
    MS_U8 ucDlcHistogramLimitCurve[17];

    MS_U8 ucDlcPureImageMode; // Compare difference of max and min bright
    MS_U8 ucDlcLevelLimit; // n = 0 ~ 4 => Limit n levels => ex. n=2, limit 2 level 0xF7, 0xE7
    MS_U8 ucDlcAvgDelta; // n = 0 ~ 50, default value: 12
    MS_U8 ucDlcAvgDeltaStill; // n = 0 ~ 15 => 0: disable still curve, 1 ~ 15 => 0.1 ~ 1.5 enable still curve
    MS_U8 ucDlcFastAlphaBlending; // min 17 ~ max 32
    MS_U8 ucDlcYAvgThresholdL; // default value: 0
    MS_U8 ucDlcYAvgThresholdH; // default value: 128

    MS_U8 ucDlcBLEPoint; // n = 24 ~ 64, default value: 48
    MS_U8 ucDlcWLEPoint; // n = 24 ~ 64, default value: 48
    MS_U8 bEnableBLE : 1; // 1: enable; 0: disable
    MS_U8 bEnableWLE : 1; // 1: enable; 0: disable
    MS_U8 ucDlcYAvgThresholdM;
    MS_U8 ucDlcCurveMode;
    MS_U8 ucDlcCurveModeMixAlpha;
    MS_U8 ucDlcAlgorithmMode;
    MS_U8 ucDlcSepPointH;
    MS_U8 ucDlcSepPointL;
    MS_U16 uwDlcBleStartPointTH;
    MS_U16 uwDlcBleEndPointTH;
    MS_U8 ucDlcCurveDiff_L_TH;
    MS_U8 ucDlcCurveDiff_H_TH;
    MS_U16 uwDlcBLESlopPoint_1;
    MS_U16 uwDlcBLESlopPoint_2;
    MS_U16 uwDlcBLESlopPoint_3;
    MS_U16 uwDlcBLESlopPoint_4;
    MS_U16 uwDlcBLESlopPoint_5;
    MS_U16 uwDlcDark_BLE_Slop_Min;
    MS_U8 ucDlcCurveDiffCoringTH;
    MS_U8 ucDlcAlphaBlendingMin;
    MS_U8 ucDlcAlphaBlendingMax;
    MS_U8 ucDlcFlicker_alpha;
    MS_U8 ucDlcYAVG_L_TH;
    MS_U8 ucDlcYAVG_H_TH;
    MS_U8 ucDlcDiffBase_L;
    MS_U8 ucDlcDiffBase_M;
    MS_U8 ucDlcDiffBase_H;
    // NEW DLC
    MS_U8 ucLMaxThreshold;
    MS_U8 ucLMinThreshold;
    MS_U8 ucLMaxCorrection;
    MS_U8 ucLMinCorrection;
    MS_U8 ucRMaxThreshold;
    MS_U8 ucRMinThreshold;
    MS_U8 ucRMaxCorrection;
    MS_U8 ucRMinCorrection;
    MS_U8 ucAllowLoseContrast;
    MS_U8 ucKernelDlcAlgorithm;
    MS_U8 ucDlcHistogramSource;
}MS_DLC_INIT_INFO;

typedef struct __attribute__((packed))
{
    MS_U8      u8ColorPrimaries;
    MS_U8      u8TransferCharacteristics;
    MS_U8      u8MatrixCoefficients;
} MS_DLC_HDR_METADATA_MPEG_VUI;

// Data format follows HDR_Metadata_MpegSEI_MasteringColorVolume
typedef struct __attribute__((packed))
{
    MS_U16     display_primaries_x[3]; // x = data*0.00002    [709] {15000, 7500, 32000}
    MS_U16     display_primaries_y[3]; // y = data*0.00002    [709] {30000, 3000, 16500}
    MS_U16     white_point_x; // x = data*0.00002    [709] 15635
    MS_U16     white_point_y; // y = data*0.00002    [709] 16450
    MS_U32     max_display_mastering_luminance; // data*0.0001 nits    [600nits] 6000000
    MS_U32     min_display_mastering_luminance; // data*0.0001 nits    [0.3nits] 3000
} MS_DLC_HDR_METADATA_MPEG_SEI_MASTERING_COLOR_VOLUME;

typedef struct __attribute__((packed))
{
    MS_U16 u16Smin; // 0.10
    MS_U16 u16Smed; // 0.10
    MS_U16 u16Smax; // 0.10
    MS_U16 u16Tmin; // 0.10
    MS_U16 u16Tmed; // 0.10
    MS_U16 u16Tmax; // 0.10
    MS_U16 u16MidSourceOffset;
    MS_U16 u16MidTargetOffset;
    MS_U16 u16MidSourceRatio;
    MS_U16 u16MidTargetRatio;
} MS_DLC_HDRToneMappingData;

typedef struct __attribute__((packed))
{
    MS_U16 u16tRx; // target Rx
    MS_U16 u16tRy; // target Ry
    MS_U16 u16tGx; // target Gx
    MS_U16 u16tGy; // target Gy
    MS_U16 u16tBx; // target Bx
    MS_U16 u16tBy; // target By
    MS_U16 u16tWx; // target Wx
    MS_U16 u16tWy; // target Wy
} MS_DLC_HDRGamutMappingData;

typedef struct __attribute__((packed))
{
    MS_U8      u8EOTF; // 0:SDR gamma, 1:HDR gamma, 2:SMPTE ST2084, 3:Future EOTF, 4-7:Reserved
    MS_U16     u16Rx; // display primaries Rx
    MS_U16     u16Ry; // display primaries Ry
    MS_U16     u16Gx; // display primaries Gx
    MS_U16     u16Gy; // display primaries Gy
    MS_U16     u16Bx; // display primaries Bx
    MS_U16     u16By; // display primaries By
    MS_U16     u16Wx; // display primaries Wx
    MS_U16     u16Wy; // display primaries Wy
    MS_U16     u16Lmax; // max display mastering luminance
    MS_U16     u16Lmin; // min display mastering luminance
    MS_U16     u16MaxCLL; // maximum content light level
    MS_U16     u16MaxFALL; // maximum frame-average light level
} MS_DLC_HDRMetadataHdmiTxInfoFrame;

typedef struct __attribute__((packed))
{
    MS_U8 PixelFormat;              // Pixel Format
    MS_U8 Colorimetry;              // Color imetry
    MS_U8 ExtendedColorimetry;      // Extended Color imetry
    MS_U8 RgbQuantizationRange;     // Rgb Quantization Range
    MS_U8 YccQuantizationRange;     // Ycc Quantization Range
    MS_U8 StaticMetadataDescriptorID; //Static Metadata Descriptor ID
} MS_DLC_HDRHdmiTxAviInfoFrame;

typedef struct __attribute__((packed))
{
    /// HDR Version Info
    MS_U16 u16HDRVerInfo;
    /// HDR init Length
    MS_U16 u16HDRInitLength;
    /// HDR Enable
    MS_BOOL bHDREnable;
    /// HDR Function Select
    MS_U16 u16HDRFunctionSelect;
    /// HDR Metadata Mpeg VUI
    MS_DLC_HDR_METADATA_MPEG_VUI DLC_HDRMetadataMpegVUI;
    /// HDR Tone Mapping Data
    MS_DLC_HDRToneMappingData DLC_HDRToneMappingData;
    /// HDR Gamut Mapping Data
    MS_DLC_HDRGamutMappingData DLC_HDRGamutMappingData;
    //HDR Metadata Hdmi Tx Info Frame
    MS_DLC_HDRMetadataHdmiTxInfoFrame DLC_HDRMetadataHdmiTxInfoFrame;
    //HDR Hdmi Tx Avi Info Frame
    MS_DLC_HDRHdmiTxAviInfoFrame DLC_HDRHdmiTxInfoAviFrame;
} MS_HDR_INFO;

// HDR use customer DLC curve.
typedef struct __attribute__((packed))
{
    MS_BOOL bFixHdrCurve;
    MS_U16 u16DlcCurveSize;
    MS_U8 *pucDlcCurve;
#if !defined (__aarch64__)
    void    *pDummy;
#endif
} MS_DLC_HDRCustomerDlcCurve;

// HDR use customer color primaries.
typedef struct __attribute__((packed))
{
    MS_BOOL bCustomerEnable;
    MS_U16 u16sWx;
    MS_U16 u16sWy;
} MS_DLC_HDRCustomerColorPrimaries;

// HDR new tone mapping parameters.
typedef struct __attribute__((packed))
{
    // TMO
    MS_U16 u16SrcMinRatio;			//default 10
    MS_U16 u16SrcMedRatio;                   //default 512
    MS_U16 u16SrcMaxRatio;			//default 990

    MS_U8 u8TgtMinFlag;                         //default 1
    MS_U16 u16TgtMin;					//default 500
    MS_U8 u8TgtMaxFlag;                        //default 0
    MS_U16 u16TgtMax;				//default 300

    MS_U16 u16FrontSlopeMin;			//default 256
    MS_U16 u16FrontSlopeMax;			//default 512
    MS_U16 u16BackSlopeMin;			//default 128
    MS_U16 u16BackSlopeMax;			//default 256

    MS_U16 u16SceneChangeThrd;		//default 1024
    MS_U16 u16SceneChangeRatioMax;	//default 1024

    MS_U8 u8IIRRatio;					//default 31
    MS_U8 u8TMO_TargetMode;      // default 0. 0 : keeps the value in initial function  1 : from output source
    MS_U8 u8TMO_Algorithm;                // default 0.  0: 18 level TMO algorithm, 1: 512 level TMO algorithm.

} MS_DLC_HDRNewToneMapping;

typedef struct __attribute__((packed))
{
    /// HDR Version Info
    MS_U16 u16HDRVerInfo;
    /// HDR init Length
    MS_U16 u16HDRInitLength;
    /// HDR Enable
    MS_BOOL bHDREnable;
    /// HDR Function Select
    MS_U16 u16HDRFunctionSelect;
    /// HDR Metadata Mpeg VUI
    MS_DLC_HDR_METADATA_MPEG_VUI DLC_HDRMetadataMpegVUI;
    /// HDR Tone Mapping Data
    MS_DLC_HDRToneMappingData DLC_HDRToneMappingData;
    /// HDR Gamut Mapping Data
    MS_DLC_HDRGamutMappingData DLC_HDRGamutMappingData;
    //HDR Metadata Hdmi Tx Info Frame
    MS_DLC_HDRMetadataHdmiTxInfoFrame DLC_HDRMetadataHdmiTxInfoFrame;
    // Customer DLC Curve
    MS_DLC_HDRCustomerDlcCurve DLC_HDRCustomerDlcCurve;
    // Customer color primarie.
    MS_DLC_HDRCustomerColorPrimaries DLC_HDRCustomerColorPrimaries;
    //HDR Hdmi Tx Avi Info Frame
    MS_DLC_HDRHdmiTxAviInfoFrame DLC_HDRHdmiTxInfoAviFrame;
    /// HDR metadata MPEG SEI mastering color volume/
    MS_DLC_HDR_METADATA_MPEG_SEI_MASTERING_COLOR_VOLUME DLC_HDRMetadataMpegSEIMasteringColorVolume;
    /// New tone mapping parameters.
    MS_DLC_HDRNewToneMapping DLC_HDRNewToneMapping;
} MS_HDR_INFO_EXTEND;

//follow Tomato's table
typedef enum _E_GFLIP_CFD_CFIO
{
    E_GFLIP_CFD_CFIO_RGB_NOTSPECIFIED      = 0x0, //means RGB, but no specific colorspace
    E_GFLIP_CFD_CFIO_RGB_BT601_625         = 0x1,
    E_GFLIP_CFD_CFIO_RGB_BT601_525         = 0x2,
    E_GFLIP_CFD_CFIO_RGB_BT709             = 0x3,
    E_GFLIP_CFD_CFIO_RGB_BT2020            = 0x4,
    E_GFLIP_CFD_CFIO_SRGB                  = 0x5,
    E_GFLIP_CFD_CFIO_ADOBE_RGB             = 0x6,
    E_GFLIP_CFD_CFIO_YUV_NOTSPECIFIED      = 0x7, //means RGB, but no specific colorspace
    E_GFLIP_CFD_CFIO_YUV_BT601_625         = 0x8,
    E_GFLIP_CFD_CFIO_YUV_BT601_525         = 0x9,
    E_GFLIP_CFD_CFIO_YUV_BT709             = 0xA,
    E_GFLIP_CFD_CFIO_YUV_BT2020_NCL        = 0xB,
    E_GFLIP_CFD_CFIO_YUV_BT2020_CL         = 0xC,
    E_GFLIP_CFD_CFIO_XVYCC_601             = 0xD,
    E_GFLIP_CFD_CFIO_XVYCC_709             = 0xE,
    E_GFLIP_CFD_CFIO_SYCC601               = 0xF,
    E_GFLIP_CFD_CFIO_ADOBE_YCC601          = 0x10,
    E_GFLIP_CFD_CFIO_DOLBY_HDR_TEMP        = 0x11,
    E_GFLIP_CFD_CFIO_SYCC709               = 0x12,
    E_GFLIP_CFD_CFIO_DCIP3_THEATER         = 0x13, /// HDR10+
    E_GFLIP_CFD_CFIO_DCIP3_D65             = 0x14, /// HDR10+
    E_GFLIP_CFD_CFIO_RESERVED_START

} E_GFLIP_CFD_CFIO;

typedef enum _E_GFLIP_CFD_MC_FORMAT
{

    E_GFLIP_CFD_MC_FORMAT_RGB     = 0x00,
    E_GFLIP_CFD_MC_FORMAT_YUV422  = 0x01,
    E_GFLIP_CFD_MC_FORMAT_YUV444  = 0x02,
    E_GFLIP_CFD_MC_FORMAT_YUV420  = 0x03,
    E_GFLIP_CFD_MC_FORMAT_RESERVED_START

} E_GFLIP_CFD_MC_FORMAT;

typedef enum _E_GFLIP_CFD_CFIO_RANGE//u8MM_Codec
{
    E_GFLIP_CFD_CFIO_RANGE_LIMIT   = 0x0,
    E_GFLIP_CFD_CFIO_RANGE_FULL    = 0x1,
    E_GFLIP_CFD_CFIO_RANGE_RESERVED_START

} E_GFLIP_CFD_CFIO_RANGE;

typedef struct __attribute__((packed))
{
    MS_U32 u32Version;
    MS_U32 u32Length;
    MS_U32 u32GOPNum;
    //STU_CFDAPI_UI_CONTROL
    MS_U16 u16Hue;
    MS_U16 u16Saturation;
    MS_U16 u16Contrast;
    MS_U16 u16Brightness;
    MS_U16 u16RGBGGain[3];
    //STU_CFDAPI_MAIN_CONTROL_LITE
    E_GFLIP_CFD_CFIO enInputFormat;
    E_GFLIP_CFD_MC_FORMAT enInputDataFormat;
    E_GFLIP_CFD_CFIO_RANGE enInputRange;
    E_GFLIP_CFD_CFIO enOutputFormat;
    E_GFLIP_CFD_MC_FORMAT enOutputDataFormat;
    E_GFLIP_CFD_CFIO_RANGE enOutputRange;
    //CFD Output CSC
    MS_U32 u32CSCAddr[10];
    MS_U16 u16CSCValue[10];
    MS_U16 u16CSCMask[10];
    MS_U16 u16CSCClient[10];
    MS_U8 u8CSCAdlData[10];
    //CFD Output Brightness
    MS_U32 u32BriAddr[3];
    MS_U16 u16BriValue[3];
    MS_U16 u16BriMask[3];
    MS_U16 u16BriClient[3];
    MS_U8 u8BriAdlData[3];
}ST_GFLIP_GOP_CSC_PARAM;

#endif //_MDRV_GFLIP_ST_H
