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
 *  Filename: wukong_obm_transfer.h
 *
 *  Description: Export obm tranfer thread function address, external structure and API.
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef __WUKONG_OBM_TRANSFER_H
#define __WUKONG_OBM_TRANSFER_H

typedef enum download_state
{
	DOWNLOADING_IDLE = 0, DOWNLOADING_IMG
}DOWNLOAD_STATE;

typedef enum _obm_img_type
{
	TIM_OBM_IMG_TYPE = 0,
	DKB_OBM_IMG_TYPE
}OBM_IMG_TYPE;

extern void set_download_state(DOWNLOAD_STATE state);
extern DOWNLOAD_STATE get_download_state(void);

int obm_transfer_handler(void);
#endif
