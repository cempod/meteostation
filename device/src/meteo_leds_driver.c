/****************************************************************************
 * boards/meteostation/src/meteo_leds_driver.c
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

#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/kmalloc.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include "stm32_gpio.h"
#include "meteostation.h"
#include "arch/board/board.h"

#ifndef CONFIG_ARCH_LEDS

typedef FAR struct file file_t;

static ssize_t meteo_leds_read(file_t *filep, FAR char *buffer, size_t buflen);
//static int meteo_leds_ioctl(FAR struct file *filep, int cmd, unsigned long arg);
struct meteo_leds_driver_s
{
	uint8_t pwm_leds_mask;
	uint8_t leds_statuses[BOARD_NLEDS];
};
static const struct file_operations leds_ops = {
	NULL,		            /* open */
	NULL,		            /* close */
	meteo_leds_read,		/* read */
	NULL,		            /* write */
	NULL,			        /* seek */
	NULL,		/* ioctl */
};

static ssize_t meteo_leds_read(file_t *filep, FAR char *buf, size_t buflen)
{
	register uint8_t reg;

	if(buf == NULL || buflen < 1)
		/* Well... nothing to do */
		return -EINVAL;

	/* These LEDs are actived by low signal (common anode), then invert signal we read*/
	reg = stm32_gpioread(GPIO_LD1);
	reg = reg & 0x0F;

	*buf = (char) reg;

	return 1;
}

int init_meteo_leds(void)
{
	stm32_configgpio(GPIO_LD1);

	FAR struct meteo_leds_driver_s *priv;
	priv = kmm_zalloc(sizeof(struct meteo_leds_driver_s));
	if (priv == NULL)
    {
      return -ENOMEM;
    }

	int ret = register_driver("/dev/leds", &leds_ops, 0444, priv);
	
  	stm32_gpiowrite(GPIO_LD1, true);
	if (ret < 0)
    {
      /* Free the device structure if we failed to create the character
       * device.
       */

      kmm_free(priv);
    }
	return ret;
}
#endif /* CONFIG_ARCH_LEDS */
