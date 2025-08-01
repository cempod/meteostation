############################################################################
# boards/meteostation/src/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(TOPDIR)/Make.defs

CSRCS = stm32_boot.c stm32_bringup.c

ifeq ($(CONFIG_ARCH_LEDS),y)
CSRCS += stm32_autoleds.c
endif

ifeq ($(CONFIG_BOARDCTL),y)
CSRCS += stm32_appinitialize.c
endif

ifeq ($(CONFIG_USBDEV),y)
CSRCS += stm32_usb.c
endif

ifeq ($(CONFIG_PWM),y)
CSRCS += stm32_pwm.c
endif

ifeq ($(CONFIG_METEO_LEDS),y)
CSRCS += meteo_leds_driver.c
endif

ifeq ($(CONFIG_STM32H7_SPI),y)
CSRCS += stm32_spi.c

ifeq ($(CONFIG_LCD),y)
CSRCS += stm32_st7796.c
CSRCS += st7796.c
endif

endif

ifeq ($(CONFIG_INPUT_FT5X06),y)
CSRCS += stm32_ft5x06.c
endif

ifeq ($(CONFIG_STM32H7_SDMMC1),y)
CSRCS += stm32_sdmmc.c
endif

ifeq ($(CONFIG_FAT_DMAMEMORY),y)
CSRCS += stm32_dma_alloc.c
endif

ifeq ($(CONFIG_ETC_ROMFS),y)
RCSRCS = etc/init.d/rc.sysinit etc/init.d/rcS	
endif

include $(TOPDIR)/boards/Board.mk
