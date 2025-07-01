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
#include <debug.h>
#include "stm32_gpio.h"
#include "meteostation.h"
#include "arch/board/board.h"
#include <nuttx/timers/pwm.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>

#ifndef CONFIG_ARCH_LEDS



struct meteo_leds_driver_s
{
	struct pwm_info_s info;
	int pwm_fd;
	uint8_t pwm_leds_mask;
	uint8_t leds_statuses[METEO_NLEDS];
};

typedef FAR struct file file_t;

static int     meteo_leds_open(file_t *filep);
static int     meteo_leds_close(file_t *filep);
static ssize_t meteo_leds_read(file_t *filep, FAR char *buffer, size_t buflen);
static int meteo_leds_ioctl(FAR struct file *filep, int cmd, unsigned long arg);
static const struct file_operations leds_ops = {
	meteo_leds_open,		/* open */
	meteo_leds_close,		/* close */
	meteo_leds_read,		/* read */
	NULL,		            /* write */
	NULL,			        /* seek */
	meteo_leds_ioctl,		/* ioctl */
};

static int meteo_leds_open(file_t *filep)
{
	/* Nothing to do here, maybe I should increase a counter like for Linux driver? */

	return OK;
}

static int meteo_leds_close(file_t *filep)
{
	/* Nothing to do here, maybe I should decrease a counter like for Linux driver?*/

	return OK;
}

static ssize_t meteo_leds_read(file_t *filep, FAR char *buf, size_t buflen)
{
	FAR struct inode *inode = filep->f_inode;
	FAR struct meteo_leds_driver_s *priv = inode->i_private;
	FAR struct meteo_leds_driver_s *p = (FAR struct meteo_leds_driver_s *)buf;

	if (buflen < sizeof(struct meteo_leds_driver_s))
    {
      syslog(LOG_ERR, "Expected buffer size is %zu\n", sizeof(struct meteo_leds_driver_s));
      return 0;
    }

	for (int i = 0; i< METEO_NLEDS; i++) {
		p->leds_statuses[i] = priv->leds_statuses[i];
	}
	p->pwm_leds_mask = priv->pwm_leds_mask;
	
	return buflen;
}

static int meteo_leds_ioctl(FAR struct file *filep, int cmd, unsigned long arg)
{
	FAR struct inode        *inode = filep->f_inode;
	FAR struct meteo_leds_driver_s *priv  = inode->i_private;
	int ret = OK;
	
	if (priv->pwm_leds_mask & (1 << cmd)) {
		struct pwm_info_s info = priv->info;
		info.duty = (uint8_t)arg ? b16divi(uitoub16((uint8_t)arg) - 1, 100) : 0;
		ret = ioctl(priv->pwm_fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)&info));
		if (ret < 0)
		{
		  syslog(LOG_ERR, "meteo_leds_driver: ioctl(PWMIOC_SETCHARACTERISTICS) failed: %d\n", errno);
		}
	} else {
		stm32_gpiowrite(GPIO_LD1, arg > 0 ? true : false);
	}
	priv->leds_statuses[cmd] = (uint8_t)arg;
	
	return ret;
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

	struct pwm_info_s info;
	info.frequency = 100;
	info.duty = 0;
	
	int fd = open("/dev/pwm0", O_RDONLY);
  	if (fd < 0)
    {
      syslog(LOG_ERR, "meteo_leds_driver: open dev/pwm0 failed: %d\n", errno);
    } else {
		priv->pwm_fd = fd;
		priv->info = info;
		priv->pwm_leds_mask |= 1<<MDISPLAY_BACK_LED;
	}
  	int ret = ioctl(fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)&info));
  	if (ret < 0)
    {
      syslog(LOG_ERR, "meteo_leds_driver: ioctl(PWMIOC_SETCHARACTERISTICS) failed: %d\n", errno);
    }
	ret = ioctl(fd, PWMIOC_START, 0);
	if (ret < 0)
    {
      syslog(LOG_ERR, "meteo_leds_driver: ioctl(PWMIOC_START) failed: %d\n", errno);
    }
	
	ret = register_driver("/dev/leds", &leds_ops, 0444, priv);
	
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
