#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

ccflags-y  += -I$(THERMAL_CHIP_DRIVER_DIR)/inc
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/thermal/mt8695/inc
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/base/power/mt8695
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/base/power/$(MTK_PLATFORM)
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/gpu/hal/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/thermal/fakeHeader/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/video/include/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/gpu/ged/include/
ccflags-y  += -I$(srctree)/drivers/thermal/
ccflags-y  += -I$(srctree)/drivers/misc/amazon/
ccflags-y  += -I$(srctree)/drivers/gpu/img-rogue/services/system/rgx_mt8695/

obj-  := dummy.o

ifeq ($(CONFIG_THERMAL), y)
else
    obj-y += mtk_thermal_dummy.o
endif

obj-$(CONFIG_THERMAL) += mtk_thermal_platform.o
obj-$(CONFIG_THERMAL) += mtk_thermal_timer.o

ifeq ($(CONFIG_MTK_GPU_SUPPORT), y)
obj-$(CONFIG_THERMAL) += mtk_cooler_fps.o
endif

ifneq ($(CONFIG_MTK_COMBO_WIFI),)
    #obj-$(CONFIG_THERMAL) += mtk_ts_wmt.o
endif

# Only included in non-EVB projects
ifneq ($(CONFIG_POWER_EXT), y)
    #obj-$(CONFIG_THERMAL) += mtk_cooler_bcct.o
    #obj-$(CONFIG_THERMAL) += mtk_ts_battery.o
endif

ifeq ($(CONFIG_BTS_THERMISTOR),)
	obj-$(CONFIG_THERMAL) += mtk_ts_bts.o
else
	obj-$(CONFIG_BTS_THERMISTOR) += lab126_ts_bts.o
endif
obj-$(CONFIG_THERMAL) += mtk_ts_cpu.o
obj-$(CONFIG_AMZN_THERMAL_COOLING) += amzn_thermal_cooling.o

ifeq ($(CONFIG_MTK_PMIC), y)
obj-$(CONFIG_THERMAL) += mtk_ts_pmic6392.o
endif
