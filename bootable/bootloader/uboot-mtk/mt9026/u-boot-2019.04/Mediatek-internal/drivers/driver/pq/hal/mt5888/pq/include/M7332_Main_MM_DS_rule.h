#define PQ_MM_DS_rule_ID_Main 4
#define PQ_MM_DS_rule_IP_NUM_Main 10
#define PQ_MM_DS_rule_NUM_Main 14

#define PQ_MM_DS_RULE_L_SD_I_MAIN 1
#define PQ_MM_DS_RULE_SD_I_MAIN 1
#define PQ_MM_DS_RULE_H_SD_I_MAIN 1
#define PQ_MM_DS_RULE_FHD_I_MAIN 1
#define PQ_MM_DS_RULE_H_FHD_I_MAIN 1
#define PQ_MM_DS_RULE_4K_1K_I_MAIN 1
#define PQ_MM_DS_RULE_4K_I_MAIN 1
#define PQ_MM_DS_RULE_L_SD_P_MAIN 1
#define PQ_MM_DS_RULE_SD_P_MAIN 1
#define PQ_MM_DS_RULE_H_SD_P_MAIN 1
#define PQ_MM_DS_RULE_FHD_P_MAIN 1
#define PQ_MM_DS_RULE_H_FHD_P_MAIN 1
#define PQ_MM_DS_RULE_4K_1K_P_MAIN 1
#define PQ_MM_DS_RULE_4K_P_MAIN 1

typedef enum
{
PQ_MM_DS_rule_L_SD_I_Main,
PQ_MM_DS_rule_SD_I_Main,
PQ_MM_DS_rule_H_SD_I_Main,
PQ_MM_DS_rule_FHD_I_Main,
PQ_MM_DS_rule_H_FHD_I_Main,
PQ_MM_DS_rule_4K_1K_I_Main,
PQ_MM_DS_rule_4K_I_Main,
PQ_MM_DS_rule_L_SD_P_Main,
PQ_MM_DS_rule_SD_P_Main,
PQ_MM_DS_rule_H_SD_P_Main,
PQ_MM_DS_rule_FHD_P_Main,
PQ_MM_DS_rule_H_FHD_P_Main,
PQ_MM_DS_rule_4K_1K_P_Main,
PQ_MM_DS_rule_4K_P_Main,
}
MST_MM_DS_rule_Index_Main;
extern U8 MST_MM_DS_rule_IP_Index_Main[PQ_MM_DS_rule_IP_NUM_Main];
extern U8 MST_MM_DS_rule_Array_Main[PQ_MM_DS_rule_NUM_Main][PQ_MM_DS_rule_IP_NUM_Main];

