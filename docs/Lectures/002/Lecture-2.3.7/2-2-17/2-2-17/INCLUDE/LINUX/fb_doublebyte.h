/*
 * linux/include/linux/fb_widechar.h
 *
 * Copyright (C) 1999		Christopher Li, Jim Chen
 *				GNU/Linux Research Center
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 *
 */

#ifndef _LINUX_FB_DOUBLEBYTE_H
#define _LINUX_FB_DOUBLEbYTE_H

#define DB_VALIDATE	0x8000
#define DB_RIGHT_MASK 	0x4000
#define DB_HALF_MASK	0x2000
#define DB_SYMBOL       0x1000
#define DB_ASCII	0

#define DB_RIGHT 	(DB_VALIDATE|DB_RIGHT_MASK)
#define DB_LEFT		(DB_VALIDATE)
#define DB_NUM		8

#define DB_INDEX_ERROR -512
struct double_byte
{
	unsigned int	num;
	char 		name[16];
	int 		(*is_left)(int );
	int 		(*is_right)(int );
	int 		(*font_index)(int left,int right);
	unsigned int   	width,height;	/* right now only support 16x16 */
	int		charcount;
	unsigned char * font_data;
};
extern int register_doublebyte(struct double_byte *);
extern int unregister_doublebyte(struct double_byte *);
extern struct double_byte * doublebyte_encodeing[DB_NUM];
extern struct double_byte * doublebyte_default;
#endif
