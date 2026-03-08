#define disp_misc 0x00484200

//Page DISP_MISC
#define REG_0100_DISP_MISC (disp_misc + 0x100)
    #define REG_0100_DISP_MISC_REG_ODCLK_ROOT_EN Fld(1,0,AC_MSKB0)//[0:0]
#define REG_0104_DISP_MISC (disp_misc + 0x104)
    #define REG_0104_DISP_MISC_REG_CLK_SHIFT Fld(1,0,AC_MSKB0)//[0:0]
#define REG_01E0_DISP_MISC (disp_misc + 0x1E0)
    #define REG_01E0_DISP_MISC_REG_TESTBUS_MUX Fld(4,0,AC_MSKB0)//[3:0]
#define REG_01FC_DISP_MISC (disp_misc + 0x1FC)
    #define REG_01FC_DISP_MISC_REG_DISP_MISC_DUMMY Fld(16,0,AC_FULLW10)//[15:0]

