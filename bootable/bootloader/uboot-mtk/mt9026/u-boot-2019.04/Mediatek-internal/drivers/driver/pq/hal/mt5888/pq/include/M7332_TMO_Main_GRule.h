#define PQ_GRULE_TMO_MODE_ENABLE 1
#define PQ_GRULE_SWDR_MODE_ENABLE 1
#define PQ_GRULE_UCD_MODE_ENABLE 1
#define PQ_GRULE_DLC_Setting_MODE_ENABLE 1
#define PQ_GRULE_DEFINE_AUTO_GEN 1
#define PQ_GRule_TMO_MODE_LOW_MAIN 1
#define PQ_GRule_TMO_MODE_MID_MAIN 1
#define PQ_GRule_TMO_MODE_HIGH_MAIN 1
#define PQ_GRule_TMO_MODE_LOW_1920_MAIN 1
#define PQ_GRule_TMO_MODE_MID_1920_MAIN 1
#define PQ_GRule_TMO_MODE_HIGH_1920_MAIN 1
#define PQ_GRule_TMO_MODE_LOW_NETSTREAMING_MAIN 1
#define PQ_GRule_TMO_MODE_MID_NETSTREAMING_MAIN 1
#define PQ_GRule_TMO_MODE_HIGH_NETSTREAMING_MAIN 1
#define PQ_GRule_SWDR_MODE_OFF_MAIN 1
#define PQ_GRule_SWDR_MODE_LOW_MAIN 1
#define PQ_GRule_SWDR_MODE_MID_MAIN 1
#define PQ_GRule_SWDR_MODE_HIGH_MAIN 1
#define PQ_GRule_SWDR_MODE_DLC_OFF_MAIN 1
#define PQ_GRule_SWDR_MODE_OFF_FRC_MAIN 1
#define PQ_GRule_SWDR_MODE_LOW_FRC_MAIN 1
#define PQ_GRule_SWDR_MODE_MID_FRC_MAIN 1
#define PQ_GRule_SWDR_MODE_HIGH_FRC_MAIN 1
#define PQ_GRule_SWDR_MODE_DLC_OFF_FRC_MAIN 1
#define PQ_GRule_SWDR_MODE_AI_PQ_MAIN 1
#define PQ_GRule_SWDR_MODE_LOW_NETSTREAMING_MAIN 1
#define PQ_GRule_SWDR_MODE_MID_NETSTREAMING_MAIN 1
#define PQ_GRule_SWDR_MODE_HIGH_NETSTREAMING_MAIN 1
#define PQ_GRule_UCD_MODE_OFF_MAIN 1
#define PQ_GRule_UCD_MODE_LOW_MAIN 1
#define PQ_GRule_UCD_MODE_MID_MAIN 1
#define PQ_GRule_UCD_MODE_HIGH_MAIN 1
#define PQ_GRule_UCD_MODE_LOW_FRC_MAIN 1
#define PQ_GRule_UCD_MODE_MID_FRC_MAIN 1
#define PQ_GRule_UCD_MODE_HIGH_FRC_MAIN 1
#define PQ_GRule_UCD_MODE_LOW_NETSTREAMING_MAIN 1
#define PQ_GRule_UCD_MODE_MID_NETSTREAMING_MAIN 1
#define PQ_GRule_UCD_MODE_HIGH_NETSTREAMING_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_LOW_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_MID_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_HIGH_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_LOW_FRC_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_MID_FRC_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_HIGH_FRC_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_LOW_NETSTREAMING_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_MID_NETSTREAMING_MAIN 1
#define PQ_GRule_DLC_SETTING_MODE_HIGH_NETSTREAMING_MAIN 1
#if (PQ_GRULE_TMO_MODE_ENABLE) || (PQ_GRULE_SWDR_MODE_ENABLE) || (PQ_GRULE_UCD_MODE_ENABLE) || (PQ_GRULE_DLC_Setting_MODE_ENABLE)
typedef enum
{
#if PQ_GRULE_TMO_MODE_ENABLE
    PQ_GRule_TMO_MODE_Main,
#endif

#if PQ_GRULE_SWDR_MODE_ENABLE
    PQ_GRule_SWDR_MODE_Main,
#endif

#if PQ_GRULE_UCD_MODE_ENABLE
    PQ_GRule_UCD_MODE_Main,
#endif

#if PQ_GRULE_DLC_Setting_MODE_ENABLE
    PQ_GRule_DLC_Setting_MODE_Main,
#endif

}
MST_GRule_TMO_Index_Main;
#endif

#if PQ_GRULE_TMO_MODE_ENABLE
typedef enum
{
    PQ_GRule_TMO_MODE_Low_Main,
    PQ_GRule_TMO_MODE_Mid_Main,
    PQ_GRule_TMO_MODE_High_Main,
    PQ_GRule_TMO_MODE_Low_1920_Main,
    PQ_GRule_TMO_MODE_Mid_1920_Main,
    PQ_GRule_TMO_MODE_High_1920_Main,
    PQ_GRule_TMO_MODE_Low_NetStreaming_Main,
    PQ_GRule_TMO_MODE_Mid_NetStreaming_Main,
    PQ_GRule_TMO_MODE_High_NetStreaming_Main,
}
MST_GRule_TMO_MODE_Index_Main;
#endif

#if PQ_GRULE_SWDR_MODE_ENABLE
typedef enum
{
    PQ_GRule_SWDR_MODE_Off_Main,
    PQ_GRule_SWDR_MODE_Low_Main,
    PQ_GRule_SWDR_MODE_Mid_Main,
    PQ_GRule_SWDR_MODE_High_Main,
    PQ_GRule_SWDR_MODE_DLC_OFF_Main,
    PQ_GRule_SWDR_MODE_Off_FRC_Main,
    PQ_GRule_SWDR_MODE_Low_FRC_Main,
    PQ_GRule_SWDR_MODE_Mid_FRC_Main,
    PQ_GRule_SWDR_MODE_High_FRC_Main,
    PQ_GRule_SWDR_MODE_DLC_OFF_FRC_Main,
    PQ_GRule_SWDR_MODE_AI_PQ_Main,
    PQ_GRule_SWDR_MODE_Low_NetStreaming_Main,
    PQ_GRule_SWDR_MODE_Mid_NetStreaming_Main,
    PQ_GRule_SWDR_MODE_High_NetStreaming_Main,
}
MST_GRule_SWDR_MODE_Index_Main;
#endif

#if PQ_GRULE_UCD_MODE_ENABLE
typedef enum
{
    PQ_GRule_UCD_MODE_OFF_Main,
    PQ_GRule_UCD_MODE_LOW_Main,
    PQ_GRule_UCD_MODE_MID_Main,
    PQ_GRule_UCD_MODE_HIGH_Main,
    PQ_GRule_UCD_MODE_LOW_FRC_Main,
    PQ_GRule_UCD_MODE_MID_FRC_Main,
    PQ_GRule_UCD_MODE_HIGH_FRC_Main,
    PQ_GRule_UCD_MODE_LOW_NetStreaming_Main,
    PQ_GRule_UCD_MODE_MID_NetStreaming_Main,
    PQ_GRule_UCD_MODE_HIGH_NetStreaming_Main,
}
MST_GRule_UCD_MODE_Index_Main;
#endif

#if PQ_GRULE_DLC_Setting_MODE_ENABLE
typedef enum
{
    PQ_GRule_DLC_Setting_MODE_Low_Main,
    PQ_GRule_DLC_Setting_MODE_Mid_Main,
    PQ_GRule_DLC_Setting_MODE_High_Main,
    PQ_GRule_DLC_Setting_MODE_Low_FRC_Main,
    PQ_GRule_DLC_Setting_MODE_Mid_FRC_Main,
    PQ_GRule_DLC_Setting_MODE_High_FRC_Main,
    PQ_GRule_DLC_Setting_MODE_Low_NetStreaming_Main,
    PQ_GRule_DLC_Setting_MODE_Mid_NetStreaming_Main,
    PQ_GRule_DLC_Setting_MODE_High_NetStreaming_Main,
}
MST_GRule_DLC_Setting_MODE_Index_Main;
#endif

#if PQ_GRULE_TMO_MODE_ENABLE
typedef enum
{
    PQ_GRule_Lvl_TMO_MODE_Low_Main,
    PQ_GRule_Lvl_TMO_MODE_Mid_Main,
    PQ_GRule_Lvl_TMO_MODE_High_Main,
    PQ_GRule_Lvl_TMO_MODE_Low_NetStreaming_Main,
    PQ_GRule_Lvl_TMO_MODE_Mid_NetStreaming_Main,
    PQ_GRule_Lvl_TMO_MODE_High_NetStreaming_Main,
}
MST_GRule_TMO_MODE_LvL_Index_Main;
#endif

#if PQ_GRULE_SWDR_MODE_ENABLE
typedef enum
{
    PQ_GRule_Lvl_SWDR_MODE_Off_Main,
    PQ_GRule_Lvl_SWDR_MODE_Low_Main,
    PQ_GRule_Lvl_SWDR_MODE_Mid_Main,
    PQ_GRule_Lvl_SWDR_MODE_High_Main,
    PQ_GRule_Lvl_SWDR_MODE_DLC_Off_Main,
    PQ_GRule_Lvl_SWDR_MODE_AI_PQ_Main,
    PQ_GRule_Lvl_SWDR_MODE_Low_NetStreaming_Main,
    PQ_GRule_Lvl_SWDR_MODE_Mid_NetStreaming_Main,
    PQ_GRule_Lvl_SWDR_MODE_High_NetStreaming_Main,
}
MST_GRule_SWDR_MODE_LvL_Index_Main;
#endif

#if PQ_GRULE_UCD_MODE_ENABLE
typedef enum
{
    PQ_GRule_Lvl_UCD_MODE_OFF_Main,
    PQ_GRule_Lvl_UCD_MODE_LOW_Main,
    PQ_GRule_Lvl_UCD_MODE_MID_Main,
    PQ_GRule_Lvl_UCD_MODE_HIGH_Main,
    PQ_GRule_Lvl_UCD_MODE_Low_NetStreaming_Main,
    PQ_GRule_Lvl_UCD_MODE_Mid_NetStreaming_Main,
    PQ_GRule_Lvl_UCD_MODE_High_NetStreaming_Main,
}
MST_GRule_UCD_MODE_LvL_Index_Main;
#endif

#if PQ_GRULE_DLC_Setting_MODE_ENABLE
typedef enum
{
    PQ_GRule_Lvl_DLC_Setting_MODE_Low_Main,
    PQ_GRule_Lvl_DLC_Setting_MODE_Mid_Main,
    PQ_GRule_Lvl_DLC_Setting_MODE_High_Main,
    PQ_GRule_Lvl_DLC_Setting_MODE_Low_NetStreaming_Main,
    PQ_GRule_Lvl_DLC_Setting_MODE_Mid_NetStreaming_Main,
    PQ_GRule_Lvl_DLC_Setting_MODE_High_NetStreaming_Main,
}
MST_GRule_DLC_Setting_MODE_LvL_Index_Main;
#endif


#define PQ_GRULE_TMO_RULE_NUM_Main 4

#if PQ_GRULE_TMO_MODE_ENABLE
#define PQ_GRULE_TMO_MODE_IP_NUM_Main 3
#define PQ_GRULE_TMO_MODE_NUM_Main 9
#define PQ_GRULE_TMO_MODE_LVL_NUM_Main 6
#endif

#if PQ_GRULE_SWDR_MODE_ENABLE
#define PQ_GRULE_SWDR_MODE_IP_NUM_Main 1
#define PQ_GRULE_SWDR_MODE_NUM_Main 14
#define PQ_GRULE_SWDR_MODE_LVL_NUM_Main 9
#endif

#if PQ_GRULE_UCD_MODE_ENABLE
#define PQ_GRULE_UCD_MODE_IP_NUM_Main 1
#define PQ_GRULE_UCD_MODE_NUM_Main 10
#define PQ_GRULE_UCD_MODE_LVL_NUM_Main 7
#endif

#if PQ_GRULE_DLC_Setting_MODE_ENABLE
#define PQ_GRULE_DLC_Setting_MODE_IP_NUM_Main 1
#define PQ_GRULE_DLC_Setting_MODE_NUM_Main 9
#define PQ_GRULE_DLC_Setting_MODE_LVL_NUM_Main 6
#endif

#if PQ_GRULE_TMO_MODE_ENABLE
extern U8 MST_GRule_TMO_MODE_IP_Index_Main[PQ_GRULE_TMO_MODE_IP_NUM_Main];
extern U8 MST_GRule_TMO_MODE_Main[QM_TMO_INPUTTYPE_NUM_Main][PQ_GRULE_TMO_MODE_NUM_Main][PQ_GRULE_TMO_MODE_IP_NUM_Main];
#endif

#if PQ_GRULE_SWDR_MODE_ENABLE
extern U8 MST_GRule_SWDR_MODE_IP_Index_Main[PQ_GRULE_SWDR_MODE_IP_NUM_Main];
extern U8 MST_GRule_SWDR_MODE_Main[QM_TMO_INPUTTYPE_NUM_Main][PQ_GRULE_SWDR_MODE_NUM_Main][PQ_GRULE_SWDR_MODE_IP_NUM_Main];
#endif

#if PQ_GRULE_UCD_MODE_ENABLE
extern U8 MST_GRule_UCD_MODE_IP_Index_Main[PQ_GRULE_UCD_MODE_IP_NUM_Main];
extern U8 MST_GRule_UCD_MODE_Main[QM_TMO_INPUTTYPE_NUM_Main][PQ_GRULE_UCD_MODE_NUM_Main][PQ_GRULE_UCD_MODE_IP_NUM_Main];
#endif

#if PQ_GRULE_DLC_Setting_MODE_ENABLE
extern U8 MST_GRule_DLC_Setting_MODE_IP_Index_Main[PQ_GRULE_DLC_Setting_MODE_IP_NUM_Main];
extern U8 MST_GRule_DLC_Setting_MODE_Main[QM_TMO_INPUTTYPE_NUM_Main][PQ_GRULE_DLC_Setting_MODE_NUM_Main][PQ_GRULE_DLC_Setting_MODE_IP_NUM_Main];
#endif

#if PQ_GRULE_TMO_MODE_ENABLE
extern U8 MST_GRule_1920_TMO_MODE_Main[PQ_GRULE_TMO_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_SWDR_MODE_ENABLE
extern U8 MST_GRule_1920_SWDR_MODE_Main[PQ_GRULE_SWDR_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_UCD_MODE_ENABLE
extern U8 MST_GRule_1920_UCD_MODE_Main[PQ_GRULE_UCD_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_DLC_Setting_MODE_ENABLE
extern U8 MST_GRule_1920_DLC_Setting_MODE_Main[PQ_GRULE_DLC_Setting_MODE_LVL_NUM_Main];
#endif

#if PQ_GRULE_TMO_MODE_ENABLE
extern U8 MST_GRule_1366_TMO_MODE_Main[PQ_GRULE_TMO_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_SWDR_MODE_ENABLE
extern U8 MST_GRule_1366_SWDR_MODE_Main[PQ_GRULE_SWDR_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_UCD_MODE_ENABLE
extern U8 MST_GRule_1366_UCD_MODE_Main[PQ_GRULE_UCD_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_DLC_Setting_MODE_ENABLE
extern U8 MST_GRule_1366_DLC_Setting_MODE_Main[PQ_GRULE_DLC_Setting_MODE_LVL_NUM_Main];
#endif

#if PQ_GRULE_TMO_MODE_ENABLE
extern U8 MST_GRule_4K2K_TMO_MODE_Main[PQ_GRULE_TMO_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_SWDR_MODE_ENABLE
extern U8 MST_GRule_4K2K_SWDR_MODE_Main[PQ_GRULE_SWDR_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_UCD_MODE_ENABLE
extern U8 MST_GRule_4K2K_UCD_MODE_Main[PQ_GRULE_UCD_MODE_LVL_NUM_Main];
#endif
#if PQ_GRULE_DLC_Setting_MODE_ENABLE
extern U8 MST_GRule_4K2K_DLC_Setting_MODE_Main[PQ_GRULE_DLC_Setting_MODE_LVL_NUM_Main];
#endif

