/******************************************************************************
*(C) Copyright 2011 Marvell International Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------
 *  -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: wukong_transfer_api.h
 *
 *  Description: export UART operations API in kernel.
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef __WUKONG_TRANSFER_API_H_
#define __WUKONG_TRANSFER_API_H_

#include <linux/fs.h>

int uart_set_speed(struct file* fd, int speed);
int uart_set_parity(struct file* fd, int databits, int stopbits, int parity);
struct file* open_dev(char *Dev);
int read_dev(struct file* fd, char* buf, int len);
int write_dev(struct file* fd, char* buf, int len);
int close_dev(struct file* fd);
int uart_set_hardware_flow_ctl(struct file* fd, int fcFlag);
int uart_set_software_flow_ctl(struct file* fd, int fcFlag);
int uart_set_raw_mode(struct file* fd, int rawFlag);
int uart_set_pure_raw_mode(struct file* fd);
void uart_flush(struct file* fd);

#endif
