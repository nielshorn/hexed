/*
 * nhexmenug.c - menu routines for nHex-Ed
 *
 * Copyright (C) 2003,2004,2010 Niels Horn <niels.horn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either 
 * version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#define MAXMENUITEMS		30
#define MAXMENUITEMLENGTH	20
#define MAXMENUCATS		9

struct nhMenuItem {
	int	iRef;				/* internal reference */
	char	sItem[MAXMENUITEMLENGTH+1];	/* Text in menu */
	char	cShortCut;			/* 1-char shortcut in menu */
	char	sFunction[MAXMENUITEMLENGTH+1];	/* Name of function to call */
	char	sKeyCode[3+1];			/* direct key-code (like "F12" or "^Q" etc */
	bool	bEnabled;			/* flag enabled in menu true/false */
	bool	bChecked;			/* flag for checkbox items */
	int	iType;				/* type of menu item (unused for now) */
};

int nhexMenu(struct nhMenuItem nhexMenuItems[], int iCnt);
int nhexSubMenu(struct nhMenuItem nhexMenuItems[], int iCnt, int submenu);
