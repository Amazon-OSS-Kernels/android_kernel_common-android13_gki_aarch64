# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2001
# Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.

#ccflags-y += -DDEBUG

obj-$(CONFIG_DM_PWM)		+= pwm-uclass.o

obj-$(CONFIG_PWM_EXYNOS)	+= exynos_pwm.o
obj-$(CONFIG_PWM_IMX)		+= pwm-imx.o pwm-imx-util.o
obj-$(CONFIG_PWM_ROCKCHIP)	+= rk_pwm.o
obj-$(CONFIG_PWM_SANDBOX)	+= sandbox_pwm.o
obj-$(CONFIG_PWM_TEGRA)		+= tegra_pwm.o
obj-$(CONFIG_PWM_SUNXI)		+= sunxi_pwm.o
obj-$(CONFIG_PWM_MEDIATEK_DTV)	+= mtk_dtv_pwm.o
obj-$(CONFIG_PWM_MEDIATEK_DTV_DAC)	+= mtk_dtv_pwm_dac.o
obj-$(CONFIG_PWM_MEDIATEK_DTV_PM)	+= mtk_dtv_pwm_pm.o
obj-$(CONFIG_PWM_MT5896)	+= mt5896_pwm.o
obj-$(CONFIG_PWM_MT5896_PM)	+= mt5896_pm_pwm.o
obj-$(CONFIG_PWM_MT5896)	+= mtk_dtv_segpwm.o

