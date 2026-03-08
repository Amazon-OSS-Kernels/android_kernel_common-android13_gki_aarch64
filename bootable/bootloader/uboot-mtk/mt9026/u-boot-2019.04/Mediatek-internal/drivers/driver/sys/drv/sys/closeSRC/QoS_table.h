#ifndef _QOS_TABLE_H_
#define _QOS_TABLE_H_
#include "drvPQ.h" //PQ_BWD_TYPE
#include "apiXC.h" //EN_XC_FRC_BW_MODE
#include "apiPNL.h"//EN_PNL_OVERDRIVE_MODE
#include "apiVDEC_EX.h" //VDEC_EX_VdecPlusDropRatio
#include "apiDMS.h" //EN_DMS_BW_MODE
#include "halQOS.h"
#define QOS_MAX_SCENARIO    20
#define MAX_CUSTOMIZE       10
#define QOS_TABLE_IGNORE_FLAG    -1

// for reference
/*
typedef enum
{
    E_XC_FRC_BW_REPEAT_MODE = 0,       //FRC BW Level 1
    E_XC_FRC_BW_LOW_BANDWIDTH_MODE,    //FRC BW Level 2
    E_XC_FRC_BW_NORMAL_MODE,           //FRC BW Level 3
    E_XC_FRC_BW_MAX,
}EN_XC_FRC_BW_MODE;

typedef enum
{
    /// off
    PQ_BWD_OFF,
    /// Low
    PQ_BWD_LOW,
    /// Middle
    PQ_BWD_MID,
    /// High
    PQ_BWD_HIGH,
    PQ_BWD_NUM,
}PQ_BWD_TYPE;

typedef enum
{
    E_PNL_OVERDRIVE_OFF = 0,
    E_PNL_OVERDRIVE_ON,
    E_PNL_OVERDRIVE_MAX,
}EN_PNL_OVERDRIVE_MODE;

typedef enum
{
    E_VDEC_EX_VDEC_PLUS_DROP_RATIO_1 = 1, //no skip data
    E_VDEC_EX_VDEC_PLUS_DROP_RATIO_2 = 2, //skip 1/2 data
    E_VDEC_EX_VDEC_PLUS_DROP_RATIO_4 = 4, //skip 3/4 data
} VDEC_EX_VdecPlusDropRatio;
*/

#ifdef QOS_OSD_DETECT_ENABLE
typedef struct {
    int bEnable;  // if equal to E_QOS_OSD_IGNORE then ignore OSD event, otherwise do osd on/off event
    int NonTransCntThreshold;  // if current total pixels of non-transparent greater than it then osd on, otherwise osd off.
} ST_QOS_OSD_EVENT;
#endif

typedef enum {
    E_QOS_MM = 0,
    E_QOS_DTV,
    E_QOS_ATV,
    E_QOS_YOUTUBE,
    E_QOS_NETFLIX,
    E_QOS_HDMI,
    E_QOS_CUSTOMER1,
    E_QOS_CUSTOMER2,
    E_QOS_CUSTOMER3,
    E_QOS_SOURCE_NONE,
    E_QOS_SOURCE_IGNORE,
} EN_QOS_SOURCE;

typedef enum {
    E_QOS_SD = 0,
    E_QOS_HD,
    E_QOS_FHD,
    E_QOS_UHD,
    E_QOS_8K,
    E_QOS_RESOLUTION_NONE,
    E_QOS_RESOLUTION_IGNORE,
} EN_QOS_RESOLUTION;

typedef enum {
    E_QOS_60P = 0,
    E_QOS_50P,
    E_QOS_30P,
    E_QOS_25P,
    E_QOS_24P,
    E_QOS_FPS_NONE,
    E_QOS_FPS_IGNORE,
} EN_QOS_FPS;

typedef enum {
    E_QOS_H264 = 0,
    E_QOS_H265,
    E_QOS_VP9,
    E_QOS_AV1,
    E_QOS_CODEC_TYPE_NONE,
    E_QOS_CODEC_TYPE_IGNORE,
} EN_QOS_CODEC_TYPE;

typedef struct {
    EN_QOS_RESOLUTION in_resolution;
    EN_QOS_RESOLUTION out_resolution;
    EN_QOS_FPS fps;
    EN_QOS_CODEC_TYPE codec;
} ST_QOS_SOURCE_INFO;

typedef struct {

    // use case
    EN_QOS_SOURCE source;
    ST_QOS_SOURCE_INFO main;
    ST_QOS_SOURCE_INFO sub;

    // scenario
    int bEnablePVR;
    int bEnablePIP;
    int bEnableAIPQ;
    int bEnableGA;
    int bEnableRotate;
    int bEnableVR360;
    int bEnableDIP;

    // customized feature
    int bEnableCustomized[MAX_CUSTOMIZE];

    int default_setting;

#ifdef QOS_OSD_DETECT_ENABLE
    ST_QOS_OSD_EVENT  osd_event;
#endif
} ST_QOS_INPUT;


typedef enum {
    E_QOS_PQ_4K60_52 = 0,
    E_QOS_PQ_4K60_26,
    E_QOS_PQ_NONE,
} EN_QOS_PQ_BASE;

typedef enum {
    E_QOS_EVD_4K60P_10B = 0,
    E_QOS_EVD_4K60P_8B,
    E_QOS_EVD_FHD60P_10B,
    E_QOS_EVD_FHD60P_8B,
    E_QOS_HVD_4K60P,
    E_QOS_HVD_4K30P,
    E_QOS_HVD_FHD60P,
    E_QOS_HVD_FHD30P,
    E_QOS_CODEC_NONE,
} EN_QOS_VDEC_BASE;

typedef enum {
    E_QOS_OD_4K2K60_RGB_15 = 0,
    E_QOS_OD_4K2K60_RGB_23,
    E_QOS_OD_4K2K60_RGB_31,
    E_QOS_OD_FHDK60_RGB_15,
    E_QOS_OD_FHD60_RGB_23,
    E_QOS_OD_FHD60_RGB_31,
    E_QOS_OD_4K2K60_RGBW_35,
    E_QOS_OD_4K2K60_RGBW_36,
    E_QOS_OD_4K2K60_RGBW_37,
    E_QOS_OD_4K2K60_RGBW_38,
    E_QOS_OD_NONE,
} EN_QOS_OD_BASE;


typedef enum {
    E_QOS_FRC_ON = 0,
    E_QOS_FRC_NONE,
} EN_QOS_FRC_BASE;

typedef enum {
    E_QOS_AIPQ_ON = 0,
    E_QOS_AIPQ_NONE,
} EN_QOS_AIPQ_BASE;


typedef struct {
    EN_QOS_PQ_BASE base; // not used
    PQ_BWD_TYPE type;
    int value; // not used
} ST_PQ_LEVEL;

typedef struct {
    EN_QOS_FRC_BASE base; // not used
    EN_XC_FRC_BW_MODE type;
    int value; // not used
} ST_FRC_LEVEL;

typedef struct {
    EN_QOS_OD_BASE base; // not used
    EN_PNL_OVERDRIVE_MODE type;
    int value; // not used
} ST_OD_LEVEL;

typedef struct {
    EN_QOS_VDEC_BASE base; // not used
    VDEC_EX_VdecPlusDropRatio type;
    int value; // not used
} ST_VDEC_LEVEL;

#ifdef QOS_OUTPUT_AIPQ_ENABLE
typedef struct {
    EN_QOS_AIPQ_BASE base; // not used
    EN_DMS_BW_MODE type;
    int value; // not used
} ST_AIPQ_LEVEL;
#endif

typedef struct {
    ST_PQ_LEVEL pq[LEVEL_MAX];
    ST_FRC_LEVEL frc[LEVEL_MAX];
    ST_OD_LEVEL od[LEVEL_MAX];
    ST_VDEC_LEVEL vdec[LEVEL_MAX];
#ifdef QOS_OUTPUT_AIPQ_ENABLE
    ST_AIPQ_LEVEL aipq[LEVEL_MAX];
#endif
} ST_QOS_OUTPUT;

typedef struct {
    ST_QOS_INPUT  input;
    ST_QOS_OUTPUT output;
} ST_QOS_SCENARIO;

typedef struct {
    int version;
    int cases;
    ST_QOS_SCENARIO scenario[QOS_MAX_SCENARIO];
} ST_QOS_TABLE;

#endif
