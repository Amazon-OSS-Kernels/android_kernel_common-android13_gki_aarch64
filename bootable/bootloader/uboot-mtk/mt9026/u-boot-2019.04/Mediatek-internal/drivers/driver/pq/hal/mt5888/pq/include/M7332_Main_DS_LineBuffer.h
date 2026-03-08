#define PQ_DS_LineBuffer_ID_Main 5
#define PQ_DS_LineBuffer_IP_NUM_Main 3
#define PQ_DS_LineBuffer_NUM_Main 4

#define PQ_DS_LINEBUFFER_4K_MAIN 1
#define PQ_DS_LINEBUFFER_DOLBY_4K_MAIN 1
#define PQ_DS_LINEBUFFER_BELOW4K_MAIN 1
#define PQ_DS_LINEBUFFER_DOLBY_BELOW4K_MAIN 1

typedef enum
{
PQ_DS_LineBuffer_4K_Main,
PQ_DS_LineBuffer_Dolby_4K_Main,
PQ_DS_LineBuffer_Below4K_Main,
PQ_DS_LineBuffer_Dolby_Below4K_Main,
}
MST_DS_LineBuffer_Index_Main;
extern U8 MST_DS_LineBuffer_IP_Index_Main[PQ_DS_LineBuffer_IP_NUM_Main];
extern U8 MST_DS_LineBuffer_Array_Main[PQ_DS_LineBuffer_NUM_Main][PQ_DS_LineBuffer_IP_NUM_Main];

