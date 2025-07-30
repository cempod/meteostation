/****************************************************************************
 * drivers/lcd/st7796.h
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

#ifndef __DRIVERS_LCD_ST7796_H
#define __DRIVERS_LCD_ST7796_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* System Function Command Table 1 */

#define ST7796_NOP        0x00   /* No Operation */
#define ST7796_SWRESET    0x01   /* Software Reset */
#define ST7796_RDDID      0x04   /* Read Display ID */
#define ST7796_RDDST      0x09   /* Read Display Status */
#define ST7796_RDDPM      0x0a   /* Read Display Power */
#define ST7796_RDDMADCTL  0x0b   /* Read Display MADCTL */
#define ST7796_RDDCOLMOD  0x0c   /* Read Display Pixel Format */
#define ST7796_RDDIM      0x0d   /* Read Display Image Mode */
#define ST7796_RDDSDR     0x0f   /* Read Display Self-Diagnostic Result */
#define ST7796_SLPIN      0x10   /* Sleep In & Booster Off */
#define ST7796_SLPOUT     0x11   /* Sleep Out & Booster On */
#define ST7796_PTLON      0x12   /* Partial Mode On */
#define ST7796_NORON      0x13   /* Partial Mode Off */
#define ST7796_INVOFF     0x20   /* Display Inversion Off */
#define ST7796_INVON      0x21   /* Display Inversion On */
#define ST7796_GAMSET     0x26   /* Gamma Set */
#define ST7796_DISPOFF    0x28   /* Display Off */
#define ST7796_DISPON     0x29   /* Display On */
#define ST7796_CASET      0x2a   /* Column Address Set */
#define ST7796_RASET      0x2b   /* Row Address Set */
#define ST7796_RAMWR      0x2c   /* Memory Write */
#define ST7796_RAMRD      0x2e   /* Memory Read */
#define ST7796_PTLAR      0x30   /* Partial Area */
#define ST7796_VSCRDEF    0x33   /* Vertical Scrolling Definition */
#define ST7796_MADCTL     0x36   /* Memory Data Access Control */
#define ST7796_VSCRSADD   0x37   /* Vertical Scrolling Start Address */
#define ST7796_COLMOD     0x3a   /* Interface Pixel Format */

FAR struct lcd_dev_s *st7796_lcdinitialize(FAR struct spi_dev_s *spi);

#endif /* __DRIVERS_LCD_ST7796_H */
