/****************************************************************************
 * boards/arm/stm32h7/openh743i/src/stm32_sdmmc.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/sdio.h>
#include <nuttx/mmcsd.h>
#include <syslog.h>

#include "stm32_gpio.h"
#include "stm32_sdmmc.h"

#include "meteostation.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#ifndef CONFIG_STM32H7_SDMMC1
#  error SDMMC1 supported only
#endif

/* If IDMA is enabled, internal SRAM must be excluded from heap */

#if CONFIG_MM_REGIONS > 1 && defined(CONFIG_STM32H7_SDMMC_IDMA)
#  error SDMMC1 with IDMA does not work CONFIG_MM_REGIONS > 1
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct sdio_dev_s *g_sdio_dev;
static bool g_sd_inserted = 0xff; /* Impossible value */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_ncd_interrupt
 *
 * Description:
 *   Card detect interrupt handler.
 *
 ****************************************************************************/

static int stm32_ncd_interrupt(int irq, void *context, void *param)
{
  bool present;

  present = stm32_gpioread(GPIO_SDIO_NCD);
  if (present != g_sd_inserted)
    {
      sdio_mediachange(g_sdio_dev, present);
      syslog(LOG_INFO, "Card detect : %d\n", present);
      g_sd_inserted = present;
    }

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_sdio_initialize
 *
 * Description:
 *   Initialize SDIO-based MMC/SD card support
 *
 ****************************************************************************/

int stm32_sdio_initialize(void)
{
  bool cd_status;
  int  ret;

  /* Configure the card detect GPIO */

  stm32_configgpio(GPIO_SDIO_NCD);

  /* Register an interrupt handler for the card detect pin */

  stm32_gpiosetevent(GPIO_SDIO_NCD, true, true, true,
                     stm32_ncd_interrupt, NULL);

  /* Mount the SDIO-based MMC/SD block driver */

  /* First, get an instance of the SDIO interface */

  finfo("Initializing SDIO slot %d\n", SDIO_SLOTNO);

  g_sdio_dev = sdio_initialize(SDIO_SLOTNO);
  if (!g_sdio_dev)
    {
      ferr("ERROR: Failed to initialize SDIO slot %d\n", SDIO_SLOTNO);
      return -ENODEV;
    }

  /* Now bind the SDIO interface to the MMC/SD driver */

  finfo("Bind SDIO to the MMC/SD driver, minor=%d\n", SDIO_MINOR);

  ret = mmcsd_slotinitialize(SDIO_MINOR, g_sdio_dev);
  if (ret != OK)
    {
      ferr("ERROR: Failed to bind SDIO to the MMC/SD driver: %d\n", ret);
      return ret;
    }

  finfo("Successfully bound SDIO to the MMC/SD driver\n");

  /* Use SD card detect pin to check if a card is g_sd_inserted */

  cd_status = stm32_gpioread(GPIO_SDIO_NCD);
  finfo("Card detect : %d\n", cd_status);

  sdio_mediachange(g_sdio_dev, cd_status);

  return OK;
}
