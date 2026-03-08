
U8 MST_DisableUC_rule_IP_Index_Main_Ex[PQ_DisableUC_rule_IP_NUM_Main_Ex]=
{
    PQ_IP_MADi_Motion_Main_Ex,
    PQ_IP_MADi_DFK_Main_Ex,
    PQ_IP_MADi_SST_Main_Ex,
    PQ_IP_MADi_EODiW_Main_Ex,
    PQ_IP_UCDi_Main_Ex,
    PQ_IP_UCDi_UI_VR_Main_Ex,
    PQ_IP_UC_CTL_Main_Ex,
    PQ_IP_UC_CTL_UI_VR_Main_Ex,
    PQ_IP_SwDriver_Main_Ex,
};


U8 MST_DisableUC_rule_Array_Main_Ex[PQ_DisableUC_rule_NUM_Main_Ex][PQ_DisableUC_rule_IP_NUM_Main_Ex]=
{
    {//DisableUC, 0
        PQ_IP_NULL, PQ_IP_MADi_DFK_OFF_Main_Ex, PQ_IP_MADi_SST_OFF_Main_Ex, PQ_IP_MADi_EODiW_OFF_Main_Ex, PQ_IP_UCDi_OFF_Main_Ex, 
        PQ_IP_UCDi_UI_VR_OFF_Main_Ex, PQ_IP_UC_CTL_OFF_Main_Ex, PQ_IP_UC_CTL_UI_VR_OFF_Main_Ex, PQ_IP_NULL, 
    },
};
