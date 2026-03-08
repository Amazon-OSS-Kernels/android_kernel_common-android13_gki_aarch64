ifeq ($(CONFIG_MSTAR_M7221),y)
    URSA_TYPE   := 13
    FCIC_IP     := true
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_M3822),y)
    URSA_TYPE   := 11
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_M7622),y)
    URSA_TYPE   := 13
    FCIC_IP     := true
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_MOONEY),y)
    URSA_TYPE   := 13
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_MAXIM),y)
    URSA_TYPE   := 11
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_MACAN),y)
    URSA_TYPE   := 11
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny   := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_MAZDA),y)
    URSA_TYPE   := 11
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny   := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_MASERATI),y)
    URSA_TYPE   := 11
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny    := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_M7322),y)
    URSA_TYPE   := 7622
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_M7332),y)
    URSA_TYPE   := 13
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_M7632),y)
    URSA_TYPE   := 7632
    FCIC_IP     := true
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MSTAR_M7642),y)
    URSA_TYPE   := 7632
    FCIC_IP     := true
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_MTK_MT5889),y)
    URSA_TYPE   := 13
    FCIC_IP     := false
    SIW_IP      := true
    SDCtiny     := true
    SIW_IP      := false
else ifeq ($(CONFIG_MTK_MT5872),y)
    URSA_TYPE   := 13
    FCIC_IP     := false
    SIW_IP      := true
    SDCtiny     := false
    SIW_IP      := false
else ifeq ($(CONFIG_TARGET_MT5896),y)
    URSA_TYPE   := MT5896
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := true

else
    URSA_TYPE   := MT5896
    FCIC_IP     := false
    SIW_IP      := false
    SDCtiny     := false
    SIW_IP      := false
endif


# Check rule
#ifeq ($(URSA_TYPE),invalid_num)
#    $(info Please set URSA_TYPE value)
#    $(info Exit Building ...)
#    $(info )
#    exit -1
#endif

ifeq ($(FCIC_IP),false)
    ifeq ($(CONFIG_DEMURA_FCIC),y)
        $(info $(CONFIG_MSTAR_CHIP_NAME) do not support FCIC, Please unset it ! )
        $(info Exit Building ...)
        $(info )
        exit -1
    endif
endif

ifeq ($(SIW_IP),false)
    ifeq ($(CONFIG_DEMURA_SIW),y)
        $(info $(CONFIG_MSTAR_CHIP_NAME) do not support SIW, Please unset it ! )
        $(info Exit Building ...)
        $(info )
        exit -1
    endif
endif

ifeq ($(SDCtiny_IP),false)
    ifeq ($(CONFIG_DEMURA_SDCtiny),y)
        $(info $(CONFIG_MSTAR_CHIP_NAME) do not support SDCtiny, Please unset it ! )
        $(info Exit Building ...)
        $(info )
        exit -1
    endif
endif
