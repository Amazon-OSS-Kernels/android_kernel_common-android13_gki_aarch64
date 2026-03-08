#define PQ_GRULE_DYNAMIC_SCBW_ENABLE 1
#define PQ_GRULE_OSD_BW_ENABLE 1
#define PQ_GRULE_MPEG_NR_ENABLE 1
#define PQ_GRULE_ULTRAT_CLEAR_ENABLE 1
#define PQ_GRULE_GAME_MODE_ENABLE 1
#define PQ_GRULE_DEFINE_AUTO_GEN 1
#define PQ_GRule_OSD_BW_ON_1920_MAIN_EX 1
#define PQ_GRule_OSD_BW_ON_1366_MAIN_EX 1
#define PQ_GRule_OSD_BW_ON_4K2K_MAIN_EX 1
#define PQ_GRule_MPEG_NR_OFF_MAIN_EX 1
#define PQ_GRule_MPEG_NR_LOW_MAIN_EX 1
#define PQ_GRule_MPEG_NR_MIDDLE_MAIN_EX 1
#define PQ_GRule_MPEG_NR_HIGH_MAIN_EX 1
#define PQ_GRule_ULTRAT_CLEAR_OFF_MAIN_EX 1
#define PQ_GRule_ULTRAT_CLEAR_ON_MAIN_EX 1
#define PQ_GRule_ULTRAT_CLEAR_ON_4K2K_MAIN_EX 1
#define PQ_GRule_GAME_MODE_ON_MIRROR_MAIN_EX 1
#define PQ_GRule_GAME_MODE_ON_NONMIRROR_MAIN_EX 1
#define PQ_GRule_GAME_MODE_ON_MIRROR_1920_MAIN_EX 1
#define PQ_GRule_GAME_MODE_ON_NONMIRROR_1920_MAIN_EX 1
#define PQ_GRule_DYNAMIC_SCBW_OFF_MAIN_EX 1
#define PQ_GRule_DYNAMIC_SCBW_LOW_MAIN_EX 1
#define PQ_GRule_DYNAMIC_SCBW_MIDDLE_MAIN_EX 1
#define PQ_GRule_DYNAMIC_SCBW_HIGH_MAIN_EX 1
#if (PQ_GRULE_DYNAMIC_SCBW_ENABLE) || (PQ_GRULE_OSD_BW_ENABLE) || (PQ_GRULE_MPEG_NR_ENABLE) || (PQ_GRULE_ULTRAT_CLEAR_ENABLE) || (PQ_GRULE_GAME_MODE_ENABLE)
typedef enum
{
#if PQ_GRULE_DYNAMIC_SCBW_ENABLE
    PQ_GRule_OSD_BW_Main_Ex,
#endif

#if PQ_GRULE_OSD_BW_ENABLE
    PQ_GRule_MPEG_NR_Main_Ex,
#endif

#if PQ_GRULE_MPEG_NR_ENABLE
    PQ_GRule_ULTRAT_CLEAR_Main_Ex,
#endif

#if PQ_GRULE_ULTRAT_CLEAR_ENABLE
    PQ_GRule_GAME_MODE_Main_Ex,
#endif

#if PQ_GRULE_GAME_MODE_ENABLE
    PQ_GRule_DYNAMIC_SCBW_Main_Ex,
#endif

}
MST_GRule_Index_Main_Ex;
#endif

#if PQ_GRULE_DYNAMIC_SCBW_ENABLE
typedef enum
{
    PQ_GRule_OSD_BW_On_1920_Main_Ex,
    PQ_GRule_OSD_BW_On_1366_Main_Ex,
    PQ_GRule_OSD_BW_On_4K2K_Main_Ex,
}
MST_GRule_OSD_BW_Index_Main_Ex;
#endif

#if PQ_GRULE_OSD_BW_ENABLE
typedef enum
{
    PQ_GRule_MPEG_NR_Off_Main_Ex,
    PQ_GRule_MPEG_NR_Low_Main_Ex,
    PQ_GRule_MPEG_NR_Middle_Main_Ex,
    PQ_GRule_MPEG_NR_High_Main_Ex,
}
MST_GRule_MPEG_NR_Index_Main_Ex;
#endif

#if PQ_GRULE_MPEG_NR_ENABLE
typedef enum
{
    PQ_GRule_ULTRAT_CLEAR_Off_Main_Ex,
    PQ_GRule_ULTRAT_CLEAR_On_Main_Ex,
    PQ_GRule_ULTRAT_CLEAR_On_4K2K_Main_Ex,
}
MST_GRule_ULTRAT_CLEAR_Index_Main_Ex;
#endif

#if PQ_GRULE_ULTRAT_CLEAR_ENABLE
typedef enum
{
    PQ_GRule_GAME_MODE_On_Mirror_Main_Ex,
    PQ_GRule_GAME_MODE_On_NonMirror_Main_Ex,
    PQ_GRule_GAME_MODE_On_Mirror_1920_Main_Ex,
    PQ_GRule_GAME_MODE_On_NonMirror_1920_Main_Ex,
}
MST_GRule_GAME_MODE_Index_Main_Ex;
#endif

#if PQ_GRULE_GAME_MODE_ENABLE
typedef enum
{
    PQ_GRule_DYNAMIC_SCBW_Off_Main_Ex,
    PQ_GRule_DYNAMIC_SCBW_Low_Main_Ex,
    PQ_GRule_DYNAMIC_SCBW_Middle_Main_Ex,
    PQ_GRule_DYNAMIC_SCBW_High_Main_Ex,
}
MST_GRule_DYNAMIC_SCBW_Index_Main_Ex;
#endif

#if PQ_GRULE_DYNAMIC_SCBW_ENABLE
typedef enum
{
    PQ_GRule_Lvl_OSD_BW_On_Main_Ex,
}
MST_GRule_OSD_BW_LvL_Index_Main_Ex;
#endif

#if PQ_GRULE_OSD_BW_ENABLE
typedef enum
{
    PQ_GRule_Lvl_MPEG_NR_Off_Main_Ex,
    PQ_GRule_Lvl_MPEG_NR_Low_Main_Ex,
    PQ_GRule_Lvl_MPEG_NR_Middle_Main_Ex,
    PQ_GRule_Lvl_MPEG_NR_High_Main_Ex,
}
MST_GRule_MPEG_NR_LvL_Index_Main_Ex;
#endif

#if PQ_GRULE_MPEG_NR_ENABLE
typedef enum
{
    PQ_GRule_Lvl_ULTRAT_CLEAR_Off_Main_Ex,
    PQ_GRule_Lvl_ULTRAT_CLEAR_On_Main_Ex,
}
MST_GRule_ULTRAT_CLEAR_LvL_Index_Main_Ex;
#endif

#if PQ_GRULE_ULTRAT_CLEAR_ENABLE
typedef enum
{
    PQ_GRule_Lvl_GAME_MODE_On_Mirror_Main_Ex,
    PQ_GRule_Lvl_GAME_MODE_On_NonMirror_Main_Ex,
}
MST_GRule_GAME_MODE_LvL_Index_Main_Ex;
#endif

#if PQ_GRULE_GAME_MODE_ENABLE
typedef enum
{
    PQ_GRule_Lvl_DYNAMIC_SCBW_Off_Main_Ex,
    PQ_GRule_Lvl_DYNAMIC_SCBW_Low_Main_Ex,
    PQ_GRule_Lvl_DYNAMIC_SCBW_Middle_Main_Ex,
    PQ_GRule_Lvl_DYNAMIC_SCBW_High_Main_Ex,
}
MST_GRule_DYNAMIC_SCBW_LvL_Index_Main_Ex;
#endif


#define PQ_GRULE_RULE_NUM_Main_Ex 5

#if PQ_GRULE_DYNAMIC_SCBW_ENABLE
#define PQ_GRULE_OSD_BW_IP_NUM_Main_Ex 5
#define PQ_GRULE_OSD_BW_NUM_Main_Ex 3
#define PQ_GRULE_OSD_BW_LVL_NUM_Main_Ex 1
#endif

#if PQ_GRULE_OSD_BW_ENABLE
#define PQ_GRULE_MPEG_NR_IP_NUM_Main_Ex 5
#define PQ_GRULE_MPEG_NR_NUM_Main_Ex 4
#define PQ_GRULE_MPEG_NR_LVL_NUM_Main_Ex 4
#endif

#if PQ_GRULE_MPEG_NR_ENABLE
#define PQ_GRULE_ULTRAT_CLEAR_IP_NUM_Main_Ex 3
#define PQ_GRULE_ULTRAT_CLEAR_NUM_Main_Ex 3
#define PQ_GRULE_ULTRAT_CLEAR_LVL_NUM_Main_Ex 2
#endif

#if PQ_GRULE_ULTRAT_CLEAR_ENABLE
#define PQ_GRULE_GAME_MODE_IP_NUM_Main_Ex 12
#define PQ_GRULE_GAME_MODE_NUM_Main_Ex 4
#define PQ_GRULE_GAME_MODE_LVL_NUM_Main_Ex 2
#endif

#if PQ_GRULE_GAME_MODE_ENABLE
#define PQ_GRULE_DYNAMIC_SCBW_IP_NUM_Main_Ex 1
#define PQ_GRULE_DYNAMIC_SCBW_NUM_Main_Ex 4
#define PQ_GRULE_DYNAMIC_SCBW_LVL_NUM_Main_Ex 4
#endif

#if PQ_GRULE_DYNAMIC_SCBW_ENABLE
extern U8 MST_GRule_OSD_BW_IP_Index_Main_Ex[PQ_GRULE_OSD_BW_IP_NUM_Main_Ex];
extern U8 MST_GRule_OSD_BW_Main_Ex[QM_INPUTTYPE_NUM_Main_Ex][PQ_GRULE_OSD_BW_NUM_Main_Ex][PQ_GRULE_OSD_BW_IP_NUM_Main_Ex];
#endif

#if PQ_GRULE_OSD_BW_ENABLE
extern U8 MST_GRule_MPEG_NR_IP_Index_Main_Ex[PQ_GRULE_MPEG_NR_IP_NUM_Main_Ex];
extern U8 MST_GRule_MPEG_NR_Main_Ex[QM_INPUTTYPE_NUM_Main_Ex][PQ_GRULE_MPEG_NR_NUM_Main_Ex][PQ_GRULE_MPEG_NR_IP_NUM_Main_Ex];
#endif

#if PQ_GRULE_MPEG_NR_ENABLE
extern U8 MST_GRule_ULTRAT_CLEAR_IP_Index_Main_Ex[PQ_GRULE_ULTRAT_CLEAR_IP_NUM_Main_Ex];
extern U8 MST_GRule_ULTRAT_CLEAR_Main_Ex[QM_INPUTTYPE_NUM_Main_Ex][PQ_GRULE_ULTRAT_CLEAR_NUM_Main_Ex][PQ_GRULE_ULTRAT_CLEAR_IP_NUM_Main_Ex];
#endif

#if PQ_GRULE_ULTRAT_CLEAR_ENABLE
extern U8 MST_GRule_GAME_MODE_IP_Index_Main_Ex[PQ_GRULE_GAME_MODE_IP_NUM_Main_Ex];
extern U8 MST_GRule_GAME_MODE_Main_Ex[QM_INPUTTYPE_NUM_Main_Ex][PQ_GRULE_GAME_MODE_NUM_Main_Ex][PQ_GRULE_GAME_MODE_IP_NUM_Main_Ex];
#endif

#if PQ_GRULE_GAME_MODE_ENABLE
extern U8 MST_GRule_DYNAMIC_SCBW_IP_Index_Main_Ex[PQ_GRULE_DYNAMIC_SCBW_IP_NUM_Main_Ex];
extern U8 MST_GRule_DYNAMIC_SCBW_Main_Ex[QM_INPUTTYPE_NUM_Main_Ex][PQ_GRULE_DYNAMIC_SCBW_NUM_Main_Ex][PQ_GRULE_DYNAMIC_SCBW_IP_NUM_Main_Ex];
#endif

#if PQ_GRULE_DYNAMIC_SCBW_ENABLE
extern U8 MST_GRule_1920_OSD_BW_Main_Ex[PQ_GRULE_OSD_BW_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_OSD_BW_ENABLE
extern U8 MST_GRule_1920_MPEG_NR_Main_Ex[PQ_GRULE_MPEG_NR_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_MPEG_NR_ENABLE
extern U8 MST_GRule_1920_ULTRAT_CLEAR_Main_Ex[PQ_GRULE_ULTRAT_CLEAR_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_ULTRAT_CLEAR_ENABLE
extern U8 MST_GRule_1920_GAME_MODE_Main_Ex[PQ_GRULE_GAME_MODE_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_GAME_MODE_ENABLE
extern U8 MST_GRule_1920_DYNAMIC_SCBW_Main_Ex[PQ_GRULE_DYNAMIC_SCBW_LVL_NUM_Main_Ex];
#endif

#if PQ_GRULE_DYNAMIC_SCBW_ENABLE
extern U8 MST_GRule_1366_OSD_BW_Main_Ex[PQ_GRULE_OSD_BW_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_OSD_BW_ENABLE
extern U8 MST_GRule_1366_MPEG_NR_Main_Ex[PQ_GRULE_MPEG_NR_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_MPEG_NR_ENABLE
extern U8 MST_GRule_1366_ULTRAT_CLEAR_Main_Ex[PQ_GRULE_ULTRAT_CLEAR_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_ULTRAT_CLEAR_ENABLE
extern U8 MST_GRule_1366_GAME_MODE_Main_Ex[PQ_GRULE_GAME_MODE_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_GAME_MODE_ENABLE
extern U8 MST_GRule_1366_DYNAMIC_SCBW_Main_Ex[PQ_GRULE_DYNAMIC_SCBW_LVL_NUM_Main_Ex];
#endif

#if PQ_GRULE_DYNAMIC_SCBW_ENABLE
extern U8 MST_GRule_4K2K_OSD_BW_Main_Ex[PQ_GRULE_OSD_BW_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_OSD_BW_ENABLE
extern U8 MST_GRule_4K2K_MPEG_NR_Main_Ex[PQ_GRULE_MPEG_NR_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_MPEG_NR_ENABLE
extern U8 MST_GRule_4K2K_ULTRAT_CLEAR_Main_Ex[PQ_GRULE_ULTRAT_CLEAR_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_ULTRAT_CLEAR_ENABLE
extern U8 MST_GRule_4K2K_GAME_MODE_Main_Ex[PQ_GRULE_GAME_MODE_LVL_NUM_Main_Ex];
#endif
#if PQ_GRULE_GAME_MODE_ENABLE
extern U8 MST_GRule_4K2K_DYNAMIC_SCBW_Main_Ex[PQ_GRULE_DYNAMIC_SCBW_LVL_NUM_Main_Ex];
#endif

