
U8 MST_DS_LineBuffer_IP_Index_Main[PQ_DS_LineBuffer_IP_NUM_Main]=
{
    PQ_IP_LineBufferShareControl_IP_Main,
    PQ_IP_LineBufferShareControl_OP_Main,
    PQ_IP_LineBufferShareControl_SC_Main,
};


U8 MST_DS_LineBuffer_Array_Main[PQ_DS_LineBuffer_NUM_Main][PQ_DS_LineBuffer_IP_NUM_Main]=
{
    {//4K, 0
        PQ_IP_LineBufferShareControl_IP_OFF_Main, PQ_IP_LineBufferShareControl_OP_SPFv11tap_VIPv5tap_Main, PQ_IP_LineBufferShareControl_SC_OFF_Main, 
    },
    {//Dolby_4K, 1
        PQ_IP_LineBufferShareControl_IP_SPFv5tap_VIPv5tap_Main, PQ_IP_LineBufferShareControl_OP_OFF_Main, PQ_IP_LineBufferShareControl_SC_OFF_Main, 
    },
    {//Below4K, 2
        PQ_IP_LineBufferShareControl_IP_OFF_Main, PQ_IP_LineBufferShareControl_OP_SPFv11tap_VIPv13tap_Main, PQ_IP_LineBufferShareControl_SC_ON_Main, 
    },
    {//Dolby_Below4K, 3
        PQ_IP_LineBufferShareControl_IP_SPFv5tap_VIPv5tap_Main, PQ_IP_LineBufferShareControl_OP_OFF_Main, PQ_IP_LineBufferShareControl_SC_ON_Main, 
    },
};
