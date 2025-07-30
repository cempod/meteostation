/****************************************************************************
 * drivers/lcd/st7796.c
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

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/spi/spi.h>
#include <nuttx/lcd/lcd.h>

#include "st7796.h"

#ifdef CONFIG_LCD

struct st7796_dev_s {

    struct lcd_dev_s dev;

    FAR struct spi_dev_s *spi;  
    uint8_t power;              
    uint16_t runbuffer[480];
};

#define LCD_ST7796_SPI_BITS 8
#define LCD_ST7796_SPIMODE SPIDEV_MODE0
#define LCD_ST7796_FREQUENCY 48000000

static void st7796_sleep(FAR struct st7796_dev_s *dev, bool sleep);
static void st7796_select(FAR struct spi_dev_s *spi, int bits);
static void st7796_deselect(FAR struct spi_dev_s *spi);
static inline void st7796_sendcmd(FAR struct st7796_dev_s *dev, uint8_t cmd);
static int st7796_getpower(FAR struct lcd_dev_s *dev);
static int st7796_setpower(FAR struct lcd_dev_s *dev, int power);
static int st7796_getcontrast(FAR struct lcd_dev_s *dev);
static int st7796_setcontrast(FAR struct lcd_dev_s *dev, unsigned int contrast);
static void st7796_fill(FAR struct st7796_dev_s *dev, uint16_t color);
static void st7796_setarea(FAR struct st7796_dev_s *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void st7796_wrram(FAR struct st7796_dev_s *dev, FAR const uint8_t *buff, size_t size, size_t skip, size_t count);
static int st7796_putrun(FAR struct lcd_dev_s *dev, fb_coord_t row, fb_coord_t col, FAR const uint8_t *buffer, size_t npixels);
static int st7796_putarea(FAR struct lcd_dev_s *dev, fb_coord_t row_start, fb_coord_t row_end, fb_coord_t col_start, fb_coord_t col_end, FAR const uint8_t *buffer, fb_coord_t stride);
static int st7796_getvideoinfo(FAR struct lcd_dev_s *dev, FAR struct fb_videoinfo_s *vinfo);
static int st7796_getplaneinfo(FAR struct lcd_dev_s *dev, unsigned int planeno, FAR struct lcd_planeinfo_s *pinfo);

static struct st7796_dev_s g_lcddev;

static void st7796_sleep(FAR struct st7796_dev_s *dev, bool sleep) {
    st7796_sendcmd(dev, sleep ? ST7796_SLPIN : ST7796_SLPOUT);
    up_mdelay(120);
}

static void st7796_select(FAR struct spi_dev_s *spi, int bits) {
    SPI_LOCK(spi, true);
    SPI_SELECT(spi, SPIDEV_DISPLAY(0), true);
    SPI_SETMODE(spi, LCD_ST7796_SPIMODE);
    SPI_SETBITS(spi, bits);
    SPI_SETFREQUENCY(spi, LCD_ST7796_FREQUENCY);
}

static void st7796_deselect(FAR struct spi_dev_s *spi) {
    SPI_SELECT(spi, SPIDEV_DISPLAY(0), false);
    SPI_LOCK(spi, false);
}

static inline void st7796_sendcmd(FAR struct st7796_dev_s *dev, uint8_t cmd) {
    SPI_CMDDATA(dev->spi, SPIDEV_DISPLAY(0), true);
    SPI_SEND(dev->spi, cmd);
    SPI_CMDDATA(dev->spi, SPIDEV_DISPLAY(0), false);
}

static int st7796_getpower(FAR struct lcd_dev_s *dev) {
    FAR struct st7796_dev_s *priv = (FAR struct st7796_dev_s *)dev;

    lcdinfo("power: %d\n", priv->power);
    return priv->power;
}

static int st7796_setpower(FAR struct lcd_dev_s *dev, int power) {

    FAR struct st7796_dev_s *priv = (FAR struct st7796_dev_s *)dev;

    lcdinfo("power: %d\n", power);

    if (power > 0) {
        st7796_sendcmd(priv, ST7796_DISPON);
        priv->power = 1;
    } else {
        st7796_sendcmd(priv, ST7796_DISPOFF);
        priv->power = 0;
    }

    return OK;
}

static int st7796_getcontrast(FAR struct lcd_dev_s *dev) {
    lcdinfo("Not implemented\n");
    return -ENOSYS;
}

static int st7796_setcontrast(FAR struct lcd_dev_s *dev,
                              unsigned int contrast) {
    lcdinfo("contrast: %d\n", contrast);
    return -ENOSYS;
}

static void st7796_fill(FAR struct st7796_dev_s *dev, uint16_t color) {
    FAR struct st7796_dev_s *priv = (FAR struct st7796_dev_s *)dev;

    st7796_setarea(priv, 0, 0, 479, 319);
    st7796_select(dev->spi, LCD_ST7796_SPI_BITS);
	st7796_sendcmd(priv, ST7796_RAMWR);

    for (int i = 0; i < 320 * 480; i++) {
        SPI_SEND(dev->spi, (uint8_t)((color>>8)&0x00FF));
        SPI_SEND(dev->spi, (uint8_t)color&0x00FF);
    }

    st7796_deselect(dev->spi);
}

static void st7796_setarea(FAR struct st7796_dev_s *dev,
                           uint16_t x0, uint16_t y0,
                           uint16_t x1, uint16_t y1) {

    FAR struct st7796_dev_s *priv = (FAR struct st7796_dev_s *)dev;

    st7796_select(dev->spi, LCD_ST7796_SPI_BITS);
    st7796_sendcmd(priv, 0x2A);
	SPI_SEND(dev->spi, x0>>8);
	SPI_SEND(dev->spi, 0x00FF&x0);
	SPI_SEND(dev->spi, x1>>8);
	SPI_SEND(dev->spi, 0x00FF&x1);

	st7796_sendcmd(priv, 0x2B);
	SPI_SEND(dev->spi, y0>>8);
	SPI_SEND(dev->spi, 0x00FF&y0);
	SPI_SEND(dev->spi, y1>>8);
	SPI_SEND(dev->spi, 0x00FF&y1);
    st7796_deselect(dev->spi);
}

static void st7796_wrram(FAR struct st7796_dev_s *dev,
                         FAR const uint8_t *buff, size_t size, size_t skip,
                         size_t count) {

    FAR struct st7796_dev_s *priv = (FAR struct st7796_dev_s *)dev;

    size_t i;

    st7796_select(dev->spi, 8);
    st7796_sendcmd(priv, ST7796_RAMWR);
    st7796_deselect(dev->spi);
    st7796_select(dev->spi, 16);
    for (i = 0; i < count; i++) {
        SPI_SNDBLOCK(dev->spi, buff + (i * (size + skip)), size / 2);
    }

    st7796_deselect(dev->spi);
}

static int st7796_putrun(FAR struct lcd_dev_s *dev,
                         fb_coord_t row, fb_coord_t col,
                         FAR const uint8_t *buffer, size_t npixels) {
    FAR struct st7796_dev_s *priv = (FAR struct st7796_dev_s *)dev;
    DEBUGASSERT(buffer && ((uintptr_t)buffer & 1) == 0);

    st7796_setarea(priv, col, row, col + npixels - 1, row);
    st7796_wrram(priv, buffer, npixels * 2, 0, 1);

    return OK;
}

static int st7796_putarea(FAR struct lcd_dev_s *dev,
                          fb_coord_t row_start, fb_coord_t row_end,
                          fb_coord_t col_start, fb_coord_t col_end,
                          FAR const uint8_t *buffer, fb_coord_t stride) {
    FAR struct st7796_dev_s *priv = (FAR struct st7796_dev_s *)dev;
    size_t cols = col_end - col_start + 1;
    size_t rows = row_end - row_start + 1;
    size_t row_size = cols * 2;
    DEBUGASSERT(buffer && ((uintptr_t)buffer & 1) == 0);

    st7796_setarea(priv, col_start, row_start, col_end, row_end);

    if (stride == row_size) {
        st7796_wrram(priv, buffer, rows * row_size, 0, 1);
    } else {
        st7796_wrram(priv, buffer, row_size, stride - row_size, rows);
    }

    return OK;
}

static int st7796_getvideoinfo(FAR struct lcd_dev_s *dev,
                               FAR struct fb_videoinfo_s *vinfo) {
    DEBUGASSERT(dev && vinfo);

    vinfo->fmt     = FB_FMT_RGB16_565;    
    vinfo->xres    = 480;       
    vinfo->yres    = 320;        
    vinfo->nplanes = 1;                  
    return OK;
}

static int st7796_getplaneinfo(FAR struct lcd_dev_s *dev,
                               unsigned int planeno,
                               FAR struct lcd_planeinfo_s *pinfo) {
    FAR struct st7796_dev_s *priv = (FAR struct st7796_dev_s *)dev;

    DEBUGASSERT(dev && pinfo && planeno == 0);

    pinfo->putrun = st7796_putrun;                  
    pinfo->putarea = st7796_putarea;                
    pinfo->buffer = (FAR uint8_t *)priv->runbuffer; 
    pinfo->bpp    = 16;                      
    pinfo->dev    = dev;                            
    return OK;
}

FAR struct lcd_dev_s *st7796_lcdinitialize(FAR struct spi_dev_s *spi) {
    FAR struct st7796_dev_s *priv = &g_lcddev;

    priv->dev.getvideoinfo = st7796_getvideoinfo;
    priv->dev.getplaneinfo = st7796_getplaneinfo;
    priv->dev.getpower     = st7796_getpower;
    priv->dev.setpower     = st7796_setpower;
    priv->dev.getcontrast  = st7796_getcontrast;
    priv->dev.setcontrast  = st7796_setcontrast;
    priv->spi              = spi;

    up_mdelay(50);
    st7796_select(spi, LCD_ST7796_SPI_BITS);
    st7796_sleep(priv, false);

    st7796_sendcmd(priv, ST7796_MADCTL);
    SPI_SEND(spi, 0x48);

    st7796_sendcmd(priv, ST7796_COLMOD);
    SPI_SEND(spi, 0x55);
    
    st7796_sendcmd(priv, 0xF0);
    SPI_SEND(spi, 0xC3);

    st7796_sendcmd(priv, 0xF0);
    SPI_SEND(spi, 0x96);

    st7796_sendcmd(priv, 0xB4);
    SPI_SEND(spi, 0x01);

    st7796_sendcmd(priv, 0xB7);
    SPI_SEND(spi, 0xC6);

    st7796_sendcmd(priv, 0xC0);
    SPI_SEND(spi, 0x80);
    SPI_SEND(spi, 0x45);

    st7796_sendcmd(priv, 0xC1);
	SPI_SEND(spi, 0x13);

	st7796_sendcmd(priv, 0xC2);
	SPI_SEND(spi, 0xA7);

	st7796_sendcmd(priv, 0xC5);
	SPI_SEND(spi, 0x0A);

	st7796_sendcmd(priv, 0xE8);
	SPI_SEND(spi, 0x40);
	SPI_SEND(spi, 0x8A);
	SPI_SEND(spi, 0x00);
	SPI_SEND(spi, 0x00);
	SPI_SEND(spi, 0x29);
	SPI_SEND(spi, 0x19);
	SPI_SEND(spi, 0xA5);
	SPI_SEND(spi, 0x33);

	st7796_sendcmd(priv, 0xE0);
	SPI_SEND(spi, 0xD0);
	SPI_SEND(spi, 0x08);
	SPI_SEND(spi, 0x0F);
	SPI_SEND(spi, 0x06);
	SPI_SEND(spi, 0x06);
	SPI_SEND(spi, 0x33);
	SPI_SEND(spi, 0x30);
	SPI_SEND(spi, 0x33);
	SPI_SEND(spi, 0x47);
	SPI_SEND(spi, 0x17);
	SPI_SEND(spi, 0x13);
	SPI_SEND(spi, 0x13);
	SPI_SEND(spi, 0x2B);
	SPI_SEND(spi, 0x31);

	st7796_sendcmd(priv, 0xE1);
	SPI_SEND(spi, 0xD0);
	SPI_SEND(spi, 0x0A);
	SPI_SEND(spi, 0x11);
	SPI_SEND(spi, 0x0B);
	SPI_SEND(spi, 0x09);
	SPI_SEND(spi, 0x07);
	SPI_SEND(spi, 0x2F);
	SPI_SEND(spi, 0x33);
	SPI_SEND(spi, 0x47);
	SPI_SEND(spi, 0x38);
	SPI_SEND(spi, 0x15);
	SPI_SEND(spi, 0x16);
	SPI_SEND(spi, 0x2C);
	SPI_SEND(spi, 0x32);


	st7796_sendcmd(priv, 0xF0);
	SPI_SEND(spi, 0x3C);

	st7796_sendcmd(priv, 0xF0);
	SPI_SEND(spi, 0x69);

	st7796_sendcmd(priv, 0x21);

	st7796_sendcmd(priv, 0x29);

    st7796_sendcmd(priv, 0x36);
	SPI_SEND(spi, (1<<3)|(1<<5));
    
    st7796_deselect(spi);

    st7796_fill(priv, 0x0000);

    return &priv->dev;
}

#endif /* CONFIG_LCD */
