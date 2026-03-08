#define PQ_GRULE_NR_ENABLE 1
#define PQ_GRULE_HDR_ENABLE 1
#define PQ_GRULE_DEFINE_AUTO_GEN 1
#define PQ_GRule_NR_OFF_MAIN_COLOR 1
#define PQ_GRule_NR_LOW_MAIN_COLOR 1
#define PQ_GRule_NR_LOW_FRC_MAIN_COLOR 1
#define PQ_GRule_NR_MIDDLE_MAIN_COLOR 1
#define PQ_GRule_NR_MIDDLE_FRC_MAIN_COLOR 1
#define PQ_GRule_NR_HIGH_MAIN_COLOR 1
#define PQ_GRule_NR_HIGH_FRC_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_LOW_L_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_LOW_M_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_LOW_H_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_MIDDLE_L_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_MIDDLE_L_FRC_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_MIDDLE_M_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_MIDDLE_M_FRC_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_MIDDLE_H_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_MIDDLE_H_FRC_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_HIGH_L_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_HIGH_L_FRC_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_HIGH_M_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_HIGH_M_FRC_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_HIGH_H_MAIN_COLOR 1
#define PQ_GRule_NR_AUTO_HIGH_H_FRC_MAIN_COLOR 1
#define PQ_GRule_HDR_ON_MAIN_COLOR 1
#define PQ_GRule_HDR_REF_MAIN_COLOR 1
#define PQ_GRule_HDR_SDR_1920_MAIN_COLOR 1
#define PQ_GRule_HDR_SDR_1366_MAIN_COLOR 1
#define PQ_GRule_HDR_SDR_4K_MAIN_COLOR 1
#define PQ_GRule_HDR_HDR_OFF_1920_MAIN_COLOR 1
#define PQ_GRule_HDR_HDR_OFF_1366_MAIN_COLOR 1
#define PQ_GRule_HDR_HDR_OFF_4K_MAIN_COLOR 1
#define PQ_GRule_HDR_DOLBY_ON_MAIN_COLOR 1
#define PQ_GRule_HDR_DOLBY_USER_MAIN_COLOR 1
#if (PQ_GRULE_NR_ENABLE) || (PQ_GRULE_HDR_ENABLE)
typedef enum
{
#if PQ_GRULE_NR_ENABLE
    PQ_GRule_NR_Main_Color,
#endif

#if PQ_GRULE_HDR_ENABLE
    PQ_GRule_HDR_Main_Color,
#endif

}
MST_GRule_Index_Main_Color;
#endif

#if PQ_GRULE_NR_ENABLE
typedef enum
{
    PQ_GRule_NR_Off_Main_Color,
    PQ_GRule_NR_Low_Main_Color,
    PQ_GRule_NR_Low_FRC_Main_Color,
    PQ_GRule_NR_Middle_Main_Color,
    PQ_GRule_NR_Middle_FRC_Main_Color,
    PQ_GRule_NR_High_Main_Color,
    PQ_GRule_NR_High_FRC_Main_Color,
    PQ_GRule_NR_Auto_Low_L_Main_Color,
    PQ_GRule_NR_Auto_Low_M_Main_Color,
    PQ_GRule_NR_Auto_Low_H_Main_Color,
    PQ_GRule_NR_Auto_Middle_L_Main_Color,
    PQ_GRule_NR_Auto_Middle_L_FRC_Main_Color,
    PQ_GRule_NR_Auto_Middle_M_Main_Color,
    PQ_GRule_NR_Auto_Middle_M_FRC_Main_Color,
    PQ_GRule_NR_Auto_Middle_H_Main_Color,
    PQ_GRule_NR_Auto_Middle_H_FRC_Main_Color,
    PQ_GRule_NR_Auto_High_L_Main_Color,
    PQ_GRule_NR_Auto_High_L_FRC_Main_Color,
    PQ_GRule_NR_Auto_High_M_Main_Color,
    PQ_GRule_NR_Auto_High_M_FRC_Main_Color,
    PQ_GRule_NR_Auto_High_H_Main_Color,
    PQ_GRule_NR_Auto_High_H_FRC_Main_Color,
}
MST_GRule_NR_Index_Main_Color;
#endif

#if PQ_GRULE_HDR_ENABLE
typedef enum
{
    PQ_GRule_HDR_On_Main_Color,
    PQ_GRule_HDR_Ref_Main_Color,
    PQ_GRule_HDR_SDR_1920_Main_Color,
    PQ_GRule_HDR_SDR_1366_Main_Color,
    PQ_GRule_HDR_SDR_4K_Main_Color,
    PQ_GRule_HDR_HDR_OFF_1920_Main_Color,
    PQ_GRule_HDR_HDR_OFF_1366_Main_Color,
    PQ_GRule_HDR_HDR_OFF_4K_Main_Color,
    PQ_GRule_HDR_Dolby_ON_Main_Color,
    PQ_GRule_HDR_Dolby_User_Main_Color,
}
MST_GRule_HDR_Index_Main_Color;
#endif

#if PQ_GRULE_NR_ENABLE
typedef enum
{
    PQ_GRule_Lvl_NR_Off_Main_Color,
    PQ_GRule_Lvl_NR_Low_Main_Color,
    PQ_GRule_Lvl_NR_Middle_Main_Color,
    PQ_GRule_Lvl_NR_High_Main_Color,
    PQ_GRule_Lvl_NR_Auto_Low_L_Main_Color,
    PQ_GRule_Lvl_NR_Auto_Low_M_Main_Color,
    PQ_GRule_Lvl_NR_Auto_Low_H_Main_Color,
    PQ_GRule_Lvl_NR_Auto_Middle_L_Main_Color,
    PQ_GRule_Lvl_NR_Auto_Middle_M_Main_Color,
    PQ_GRule_Lvl_NR_Auto_Middle_H_Main_Color,
    PQ_GRule_Lvl_NR_Auto_High_L_Main_Color,
    PQ_GRule_Lvl_NR_Auto_High_M_Main_Color,
    PQ_GRule_Lvl_NR_Auto_High_H_Main_Color,
}
MST_GRule_NR_LvL_Index_Main_Color;
#endif

#if PQ_GRULE_HDR_ENABLE
typedef enum
{
    PQ_GRule_Lvl_HDR_Off_Main_Color,
    PQ_GRule_Lvl_HDR_Open_Auto_Main_Color,
    PQ_GRule_Lvl_HDR_Open_High_Main_Color,
    PQ_GRule_Lvl_HDR_Open_Mid_Main_Color,
    PQ_GRule_Lvl_HDR_Open_Low_Main_Color,
    PQ_GRule_Lvl_HDR_Open_Ref_Main_Color,
    PQ_GRule_Lvl_HDR_Dolby_Vivid_Main_Color,
    PQ_GRule_Lvl_HDR_Dolby_User_Main_Color,
    PQ_GRule_Lvl_HDR_Dolby_Brightness_Main_Color,
    PQ_GRule_Lvl_HDR_Dolby_Dark_Main_Color,
    PQ_GRule_Lvl_HDR_HLG_Auto_Main_Color,
    PQ_GRule_Lvl_HDR_HLG_High_Main_Color,
    PQ_GRule_Lvl_HDR_HLG_Mid_Main_Color,
    PQ_GRule_Lvl_HDR_HLG_Low_Main_Color,
    PQ_GRule_Lvl_HDR_SDR_Main_Color,
}
MST_GRule_HDR_LvL_Index_Main_Color;
#endif


#define PQ_GRULE_RULE_NUM_Main_Color 2

#if PQ_GRULE_NR_ENABLE
#define PQ_GRULE_NR_IP_NUM_Main_Color 1
#define PQ_GRULE_NR_NUM_Main_Color 22
#define PQ_GRULE_NR_LVL_NUM_Main_Color 13
#endif

#if PQ_GRULE_HDR_ENABLE
#define PQ_GRULE_HDR_IP_NUM_Main_Color 1
#define PQ_GRULE_HDR_NUM_Main_Color 10
#define PQ_GRULE_HDR_LVL_NUM_Main_Color 15
#endif

#if PQ_GRULE_NR_ENABLE
extern U8 MST_GRule_NR_IP_Index_Main_Color[PQ_GRULE_NR_IP_NUM_Main_Color];
extern U8 MST_GRule_NR_Main_Color[QM_INPUTTYPE_NUM_Main_Color][PQ_GRULE_NR_NUM_Main_Color][PQ_GRULE_NR_IP_NUM_Main_Color];
#endif

#if PQ_GRULE_HDR_ENABLE
extern U8 MST_GRule_HDR_IP_Index_Main_Color[PQ_GRULE_HDR_IP_NUM_Main_Color];
extern U8 MST_GRule_HDR_Main_Color[QM_INPUTTYPE_NUM_Main_Color][PQ_GRULE_HDR_NUM_Main_Color][PQ_GRULE_HDR_IP_NUM_Main_Color];
#endif

#if PQ_GRULE_NR_ENABLE
extern U8 MST_GRule_1920_NR_Main_Color[PQ_GRULE_NR_LVL_NUM_Main_Color];
#endif
#if PQ_GRULE_HDR_ENABLE
extern U8 MST_GRule_1920_HDR_Main_Color[PQ_GRULE_HDR_LVL_NUM_Main_Color];
#endif

