/*
 * nhexmenu.c - menu routines for nHex-Ed
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

#include <ncurses.h>
#include <panel.h>
#include <stdlib.h>
#include <string.h>

#include "nhexmenu.h"

#define MAXMENUITEMS		30
#define MAXMENUITEMLENGTH	20
#define MAXMENUCATS		9	/* can't be increased w/o changing some code... */

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

char	*nhexMenuItems[]={"001.File",
				"101.Open",      "102.Save",      "103.Save As...", "104.Exit",
			  "002.Edit",
				"201.Undo last", "202.Undo All",
			  "003.Search",
				"301.Find...",   "302.Find Next", "303.Goto...",
			  "004.Help",
				"401.Help",      "402.About",
			 };

int nhexMenu()
{
	int	i;
	int	c;
	int	iResult=0;
	bool	ready=false;
	int	pwidth, pheight;
	int	y, x;
	int	nItems=0;
	char	sItem[MAXMENUCATS][MAXMENUITEMLENGTH+3];	/* 2x space + \0 */
	int	iItemPos[MAXMENUCATS + 1];
	int	iItemVal[MAXMENUCATS];
	int	iItemSel=0;
	int	iItemSkip=0;
	int	iItemBase=0;

	getmaxyx(stdscr, pheight, pwidth);

	/* main menu on first line */
	for(i=0; i<ARRAY_SIZE(nhexMenuItems); i++)
	{
		if(nhexMenuItems[i][0] == '0')
		{
			snprintf(sItem[nItems], MAXMENUITEMLENGTH+2, " %s ",&nhexMenuItems[i][4]);
			iItemPos[nItems]=strlen(sItem[nItems]);
			if(nItems > 0) iItemPos[nItems]+=iItemPos[nItems-1];
			iItemVal[nItems]=atoi(&nhexMenuItems[i][1]);
			nItems++;
			if(nItems == MAXMENUCATS) break;
		}
	}

	/* loop for a choice */
	while(1)
	{
		/* scroll menu left/right if necessary */
		while(iItemPos[iItemSel]-iItemBase > pwidth-1)
		{
			iItemBase=iItemPos[iItemSkip];
			iItemSkip++;
		}
		while(iItemPos[iItemSel]-strlen(sItem[iItemSel]) < iItemBase)
		{
			iItemSkip--;
			iItemBase=iItemPos[iItemSkip]-strlen(sItem[iItemSkip]);
		}
		i=iItemSkip;
		attron(A_REVERSE);
		if(iItemSkip > 0)
			mvprintw(0, 0, "<");
		else
			mvprintw(0, 0, " ");
		clrtoeol();
		while(i<nItems)
		{
			getyx(stdscr, y, x);
			if(x + strlen(sItem[i]) <= pwidth-1)
			{
				printw("%s",sItem[i]);
				i++;
			}
			else
			{
				mvprintw(0, pwidth-1, ">");
				break;
			}
		}
		mvchgat(0,0,-1, A_REVERSE, 0, NULL);
		attroff(A_REVERSE);
		mvchgat(0, 1 + iItemPos[iItemSel]-strlen(sItem[iItemSel])-iItemBase, strlen(sItem[iItemSel]), 0, 0, NULL);
		move(0, 1 + iItemPos[iItemSel]-strlen(sItem[iItemSel])-iItemBase + 1);
		refresh();

		c=getch();
		switch(c)
		{
			case KEY_LEFT:
				if(iItemSel > 0) iItemSel--;
				break;
			case KEY_RIGHT:
				if(iItemSel < nItems-1) iItemSel++;
				break;
			case KEY_HOME:
				iItemSel=0;
				break;
			case KEY_END:
				iItemSel=nItems-1;
				break;
			case KEY_ENTER:
			case KEY_DOWN:
			case 10:
				while(1)
				{
					iResult=nhexSubMenu(iItemVal[iItemSel]);
					if(iResult > 100)
					{
						ready=true;
						break;
					}
					else if(iResult != 0)
					{
						mvchgat(0, iItemPos[iItemSel]-strlen(sItem[iItemSel])-iItemBase + 1, \
								strlen(sItem[iItemSel]), A_REVERSE, 0, NULL);
						iItemSel=iResult-1;
						mvchgat(0, 1 + iItemPos[iItemSel]-strlen(sItem[iItemSel])-iItemBase, \
								strlen(sItem[iItemSel]), 0, 0, NULL);
						move(0, 1 + iItemPos[iItemSel]-strlen(sItem[iItemSel])-iItemBase + 1 );
					}
					else
						break;
				}
				break;
			case KEY_F(12):
			case 27:
				iResult=0;
				ready=true;
				break;
		}
		if(ready) break;
	}

	return iResult;
}

int nhexSubMenu(int submenu)
{
	int	i;
	int	c;
	int	iResult;
	int	iSub, maxSub=0;
	bool	ready=false;
	int	ixPos, iyPos;
	int	pheight, pwidth;
	int	mheight, mwidth;
	int	iMax=0;
	int	nItems=0;
	char	sItem[MAXMENUITEMS][MAXMENUITEMLENGTH+3];	/* 2x space + \0 */
	int	iItemVal[MAXMENUITEMS];
	int	iItemSel=0;
	WINDOW	*menuWin;
	PANEL	*menuPanel[2];

	getyx(stdscr, iyPos, ixPos);

	getmaxyx(stdscr, pheight, pwidth);
	
	for(i=0; i<ARRAY_SIZE(nhexMenuItems); i++)
	{
		iSub=atoi(&nhexMenuItems[i][1]);
		if(nhexMenuItems[i][0] =='0')
		{
			if(iSub > maxSub) maxSub=iSub;
		}
		else if(nhexMenuItems[i][0] == 48+submenu)	/* 48='0' */
		{
			snprintf(sItem[nItems], MAXMENUITEMLENGTH+2, " %s ",&nhexMenuItems[i][4]);
			if(strlen(sItem[nItems]) > iMax) iMax=strlen(sItem[nItems]);
			iItemVal[nItems]=iSub;
			nItems++;
			if(nItems == MAXMENUITEMS) break;
		}
	}
	iyPos=1;
	ixPos--;
	if(ixPos+iMax+2 > pwidth) ixPos=pwidth-iMax-2;
	mwidth=iMax+2;
	mheight=nItems+2;
	menuPanel[0]=new_panel(stdscr);
	menuWin=newwin(mheight, mwidth, iyPos, ixPos);
	menuPanel[1]=new_panel(menuWin);

	wattron(menuWin, A_REVERSE);
	wborder(menuWin, '|', '|', '-', '-', '+', '+', '+', '+');
	for(i=0; i<mheight; i++) mvwchgat(menuWin, i, 0, -1, A_REVERSE, 0, NULL);

	/* loop through options */
	while(1)
	{
		for(i=0; i<nItems; i++)
		{
			mvwprintw(menuWin, 1+i, 1, sItem[i]);
			if(i == iItemSel)
				mvwchgat(menuWin, 1+i, 2, strlen(sItem[i])-2, 0, 0, NULL);
		}
		wmove(menuWin, 1+iItemSel, 2);
		wrefresh(menuWin);
		update_panels();
		doupdate();
		c=getch();
		switch(c)
		{
			case KEY_UP:
				if(iItemSel > 0)
					iItemSel--;
				else
				{
					iResult=0;
					ready=true;
				}
				break;
			case KEY_DOWN:
				if(iItemSel < nItems-1) iItemSel++;
				break;
			case KEY_HOME:
				iItemSel=0;
				break;
			case KEY_END:
				iItemSel=nItems-1;
				break;
			case KEY_LEFT:
				if(submenu > 1){
					iResult=submenu-1;
					ready=true;
				}
				break;
			case KEY_RIGHT:
				if(submenu < maxSub){
					iResult=submenu+1;
					ready=true;
				}
				break;
			case KEY_ENTER:
			case 10:
				iResult=submenu*100 + iItemVal[iItemSel];
				ready=true;
				break;
		}
		if(ready)
		{
			break;
		}
	}

	for(i=0; i<mheight; i++) mvwchgat(menuWin, i, 0, -1, 0, 0, NULL);
	wrefresh(menuWin);
	delwin(menuWin);
	del_panel(menuPanel[1]);
	del_panel(menuPanel[0]);
	update_panels();
	doupdate();
	return iResult;
}

