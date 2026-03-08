
U8 MST_DS_HVSP_Settings_IP_Index_Main[PQ_DS_HVSP_Settings_IP_NUM_Main]=
{
    PQ_IP_VSP_Y_Main,
    PQ_IP_VSP_C_Main,
    PQ_IP_HSP_Y_Main,
    PQ_IP_HSP_C_Main,
    PQ_IP_SRAM1_Main,
    PQ_IP_SRAM2_Main,
    PQ_IP_C_SRAM1_Main,
    PQ_IP_C_SRAM2_Main,
};


U8 MST_DS_HVSP_Settings_Array_Main[PQ_DS_HVSP_Settings_NUM_Main][PQ_DS_HVSP_Settings_IP_NUM_Main]=
{
    {//H1920_ScDown_V_ScDown, 0
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_SRAM_2_6Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc75Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920_ScDown_V_1X, 1
        PQ_IP_VSP_Y_Bypass_Main, PQ_IP_VSP_C_Bypass_Main, PQ_IP_HSP_Y_SRAM_2_6Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920_ScDown_V_ScUp, 2
        PQ_IP_VSP_Y_SRAM_1_10Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_SRAM_2_6Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_ScDown_V_ScDown, 3
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_SRAM_2_6Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc75Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_ScDown_V_1X, 4
        PQ_IP_VSP_Y_Bypass_Main, PQ_IP_VSP_C_Bypass_Main, PQ_IP_HSP_Y_SRAM_2_6Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_ScDown_V_ScUp, 5
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_SRAM_2_6Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920_1X_V_ScDown, 6
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_Bypass_Main, PQ_IP_HSP_C_Bypass_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc75Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920_1X_V_1X, 7
        PQ_IP_VSP_Y_Bypass_Main, PQ_IP_VSP_C_Bypass_Main, PQ_IP_HSP_Y_Bypass_Main, PQ_IP_HSP_C_Bypass_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920_1X_V_ScUp, 8
        PQ_IP_VSP_Y_SRAM_1_10Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_Bypass_Main, PQ_IP_HSP_C_Bypass_Main, PQ_IP_SRAM1_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_1X_V_ScDown, 9
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_Bypass_Main, PQ_IP_HSP_C_Bypass_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc75Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc6Tc4p4Fc57Apass01Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_1X_V_1X, 10
        PQ_IP_VSP_Y_Bypass_Main, PQ_IP_VSP_C_Bypass_Main, PQ_IP_HSP_Y_Bypass_Main, PQ_IP_HSP_C_Bypass_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_1X_V_ScUp, 11
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_Bypass_Main, PQ_IP_HSP_C_Bypass_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920_ScUp_V_ScDown, 12
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_SRAM_2_10Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc75Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920_ScUp_V_1X, 13
        PQ_IP_VSP_Y_Bypass_Main, PQ_IP_VSP_C_Bypass_Main, PQ_IP_HSP_Y_SRAM_2_10Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920_ScUp_V_ScUp, 14
        PQ_IP_VSP_Y_SRAM_1_10Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_SRAM_2_10Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_ScUp_V_ScDown, 15
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_SRAM_2_10Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc75Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_ScUp_V_1X, 16
        PQ_IP_VSP_Y_Bypass_Main, PQ_IP_VSP_C_Bypass_Main, PQ_IP_HSP_Y_SRAM_2_10Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
    {//H1920Up_ScUp_V_ScUp, 17
        PQ_IP_VSP_Y_SRAM_1_4Tap_Main, PQ_IP_VSP_C_C_SRAM_1_Main, PQ_IP_HSP_Y_SRAM_2_10Tap_Main, PQ_IP_HSP_C_C_SRAM_1_Main, PQ_IP_SRAM1_InvSinc4Tc4p4Fc90Apass0001Astop40_Main, 
        PQ_IP_SRAM2_InvSinc10Tc4p4Fc95Apass0001Astop60_Main, PQ_IP_C_SRAM1_C121_Main, PQ_IP_C_SRAM2_C121_Main, 
    },
};
