#include "hwreg_frc_map.h" 
#include "M7322_2D_4K2K.h"

// 2D_4K2K
// 2D_FHD_RGB_BYPASS
void MFC_3D_2D_4K2K_2D_FHD_RGB_BYPASS(void)
{
// FSC_TOP
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x01); // reg_hvsp_bypass
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x02); // reg_hvsp_buffer_md
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x04); // reg_fsc_lb_bypass
// VSU
    MDrv_WriteByteMask( REG_SC_BK4F_12, 0x00, 0xff); // vsp_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_13, 0x00, 0xff); // vsp_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_14, 0x08, 0xff); // vsp_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_15, 0x01, 0x01); // vsp_scl_en
    MDrv_WriteByteMask( REG_SC_BK4F_15, 0x02, 0x02); // vsp_shift_mode_en
    MDrv_WriteByteMask( REG_SC_BK4F_06, 0x00, 0xff); // vsp_ini_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_07, 0x00, 0xff); // vsp_ini_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_08, 0x0C, 0xff); // vsp_ini_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_52, 0x38, 0xff); // vsp_vsize_in0
    MDrv_WriteByteMask( REG_SC_BK4F_53, 0x04, 0xff); // vsp_vsize_in1
    MDrv_WriteByteMask( REG_SC_BK4F_56, 0x70, 0xff); // vsp_vsize_out0
    MDrv_WriteByteMask( REG_SC_BK4F_57, 0x08, 0xff); // vsp_vsize_out1
// HSU
    MDrv_WriteByteMask( REG_SC_BK4F_0E, 0x00, 0xff); // hsp_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_0F, 0x00, 0xff); // hsp_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_10, 0x08, 0xff); // hsp_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_11, 0x01, 0x01); // hsp_scl_en
    MDrv_WriteByteMask( REG_SC_BK4F_11, 0x02, 0x02); // hsp_shift_mode_en
    MDrv_WriteByteMask( REG_SC_BK4F_02, 0x00, 0xff); // hsp_ini_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_03, 0x00, 0xff); // hsp_ini_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_04, 0x0C, 0xff); // hsp_ini_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_50, 0x80, 0xff); // hsp_hsize_in0
    MDrv_WriteByteMask( REG_SC_BK4F_51, 0x07, 0xff); // hsp_hsize_in1
    MDrv_WriteByteMask( REG_SC_BK4F_54, 0x00, 0xff); // hsp_hsize_out0
    MDrv_WriteByteMask( REG_SC_BK4F_55, 0x0f, 0xff); // hsp_hsize_out1
// SPTF_D2LR
// FSC_3D
    MDrv_WriteByteMask( REG_SC_BK4F_21, 0x00, 0x40); // reg_3d_top_bot_en
    MDrv_WriteByteMask( REG_SC_BK4F_21, 0x00, 0x80); // reg_3d_sbs_en
}



/********************************************/
// 2D_FHD_YUV
void MFC_3D_2D_4K2K_2D_FHD_YUV(void)
{
// FSC_TOP
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x01); // reg_hvsp_bypass
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x02); // reg_hvsp_buffer_md
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x04); // reg_fsc_lb_bypass
// VSU
    MDrv_WriteByteMask( REG_SC_BK4F_12, 0x00, 0xff); // vsp_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_13, 0x00, 0xff); // vsp_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_14, 0x08, 0xff); // vsp_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_15, 0x01, 0x01); // vsp_scl_en
    MDrv_WriteByteMask( REG_SC_BK4F_15, 0x02, 0x02); // vsp_shift_mode_en
    MDrv_WriteByteMask( REG_SC_BK4F_06, 0x00, 0xff); // vsp_ini_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_07, 0x00, 0xff); // vsp_ini_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_08, 0x0C, 0xff); // vsp_ini_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_52, 0x38, 0xff); // vsp_vsize_in0
    MDrv_WriteByteMask( REG_SC_BK4F_53, 0x04, 0xff); // vsp_vsize_in1
    MDrv_WriteByteMask( REG_SC_BK4F_56, 0x70, 0xff); // vsp_vsize_out0
    MDrv_WriteByteMask( REG_SC_BK4F_57, 0x08, 0xff); // vsp_vsize_out1
// HSU
    MDrv_WriteByteMask( REG_SC_BK4F_0E, 0x00, 0xff); // hsp_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_0F, 0x00, 0xff); // hsp_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_10, 0x08, 0xff); // hsp_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_11, 0x01, 0x01); // hsp_scl_en
    MDrv_WriteByteMask( REG_SC_BK4F_11, 0x02, 0x02); // hsp_shift_mode_en
    MDrv_WriteByteMask( REG_SC_BK4F_02, 0x00, 0xff); // hsp_ini_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_03, 0x00, 0xff); // hsp_ini_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_04, 0x0C, 0xff); // hsp_ini_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_50, 0x80, 0xff); // hsp_hsize_in0
    MDrv_WriteByteMask( REG_SC_BK4F_51, 0x07, 0xff); // hsp_hsize_in1
    MDrv_WriteByteMask( REG_SC_BK4F_54, 0x00, 0xff); // hsp_hsize_out0
    MDrv_WriteByteMask( REG_SC_BK4F_55, 0x0f, 0xff); // hsp_hsize_out1
// SPTF_D2LR
// FSC_3D
    MDrv_WriteByteMask( REG_SC_BK4F_21, 0x00, 0x40); // reg_3d_top_bot_en
    MDrv_WriteByteMask( REG_SC_BK4F_21, 0x00, 0x80); // reg_3d_sbs_en
}



/********************************************/
// 2D_4K2K_RGB_BYPASS
void MFC_3D_2D_4K2K_2D_4K2K_RGB_BYPASS(void)
{
// FSC_TOP
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x01); // reg_hvsp_bypass
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x02, 0x02); // reg_hvsp_buffer_md
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x04); // reg_fsc_lb_bypass
// VSU
    MDrv_WriteByteMask( REG_SC_BK4F_12, 0x00, 0xff); // vsp_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_13, 0x00, 0xff); // vsp_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_14, 0x10, 0xff); // vsp_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_15, 0x01, 0x01); // vsp_scl_en
    MDrv_WriteByteMask( REG_SC_BK4F_15, 0x00, 0x02); // vsp_shift_mode_en
    MDrv_WriteByteMask( REG_SC_BK4F_06, 0x00, 0xff); // vsp_ini_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_07, 0x00, 0xff); // vsp_ini_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_08, 0x00, 0xff); // vsp_ini_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_52, 0x70, 0xff); // vsp_vsize_in0
    MDrv_WriteByteMask( REG_SC_BK4F_53, 0x08, 0xff); // vsp_vsize_in1
    MDrv_WriteByteMask( REG_SC_BK4F_56, 0x70, 0xff); // vsp_vsize_out0
    MDrv_WriteByteMask( REG_SC_BK4F_57, 0x08, 0xff); // vsp_vsize_out1
// HSU
    MDrv_WriteByteMask( REG_SC_BK4F_0E, 0x00, 0xff); // hsp_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_0F, 0x00, 0xff); // hsp_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_10, 0x10, 0xff); // hsp_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_11, 0x01, 0x01); // hsp_scl_en
    MDrv_WriteByteMask( REG_SC_BK4F_11, 0x00, 0x02); // hsp_shift_mode_en
    MDrv_WriteByteMask( REG_SC_BK4F_02, 0x00, 0xff); // hsp_ini_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_03, 0x00, 0xff); // hsp_ini_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_04, 0x00, 0xff); // hsp_ini_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_50, 0x00, 0xff); // hsp_hsize_in0
    MDrv_WriteByteMask( REG_SC_BK4F_51, 0x0f, 0xff); // hsp_hsize_in1
    MDrv_WriteByteMask( REG_SC_BK4F_54, 0x00, 0xff); // hsp_hsize_out0
    MDrv_WriteByteMask( REG_SC_BK4F_55, 0x0f, 0xff); // hsp_hsize_out1
// SPTF_D2LR
// FSC_3D
    MDrv_WriteByteMask( REG_SC_BK4F_21, 0x00, 0x40); // reg_3d_top_bot_en
    MDrv_WriteByteMask( REG_SC_BK4F_21, 0x00, 0x80); // reg_3d_sbs_en
}



/********************************************/
// 2D_4K2K_YUV
void MFC_3D_2D_4K2K_2D_4K2K_YUV(void)
{
// FSC_TOP
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x01); // reg_hvsp_bypass
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x02, 0x02); // reg_hvsp_buffer_md
    MDrv_WriteByteMask( REG_SC_BK4F_C0, 0x00, 0x04); // reg_fsc_lb_bypass
// VSU
    MDrv_WriteByteMask( REG_SC_BK4F_12, 0x00, 0xff); // vsp_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_13, 0x00, 0xff); // vsp_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_14, 0x10, 0xff); // vsp_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_15, 0x01, 0x01); // vsp_scl_en
    MDrv_WriteByteMask( REG_SC_BK4F_15, 0x00, 0x02); // vsp_shift_mode_en
    MDrv_WriteByteMask( REG_SC_BK4F_06, 0x00, 0xff); // vsp_ini_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_07, 0x00, 0xff); // vsp_ini_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_08, 0x00, 0xff); // vsp_ini_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_52, 0x70, 0xff); // vsp_vsize_in0
    MDrv_WriteByteMask( REG_SC_BK4F_53, 0x08, 0xff); // vsp_vsize_in1
    MDrv_WriteByteMask( REG_SC_BK4F_56, 0x70, 0xff); // vsp_vsize_out0
    MDrv_WriteByteMask( REG_SC_BK4F_57, 0x08, 0xff); // vsp_vsize_out1
// HSU
    MDrv_WriteByteMask( REG_SC_BK4F_0E, 0x00, 0xff); // hsp_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_0F, 0x00, 0xff); // hsp_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_10, 0x10, 0xff); // hsp_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_11, 0x01, 0x01); // hsp_scl_en
    MDrv_WriteByteMask( REG_SC_BK4F_11, 0x00, 0x02); // hsp_shift_mode_en
    MDrv_WriteByteMask( REG_SC_BK4F_02, 0x00, 0xff); // hsp_ini_scl_fac0
    MDrv_WriteByteMask( REG_SC_BK4F_03, 0x00, 0xff); // hsp_ini_scl_fac1
    MDrv_WriteByteMask( REG_SC_BK4F_04, 0x00, 0xff); // hsp_ini_scl_fac2
    MDrv_WriteByteMask( REG_SC_BK4F_50, 0x00, 0xff); // hsp_hsize_in0
    MDrv_WriteByteMask( REG_SC_BK4F_51, 0x0f, 0xff); // hsp_hsize_in1
    MDrv_WriteByteMask( REG_SC_BK4F_54, 0x00, 0xff); // hsp_hsize_out0
    MDrv_WriteByteMask( REG_SC_BK4F_55, 0x0f, 0xff); // hsp_hsize_out1
// SPTF_D2LR
// FSC_3D
    MDrv_WriteByteMask( REG_SC_BK4F_21, 0x00, 0x40); // reg_3d_top_bot_en
    MDrv_WriteByteMask( REG_SC_BK4F_21, 0x00, 0x80); // reg_3d_sbs_en
}



/********************************************/
