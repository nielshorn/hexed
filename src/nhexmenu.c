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

#include <stdlib.h>
#include <string.h>

#include <ncurses.h>
#include <panel.h>

#include "nhexed.h"
#include "nhexmenu.h"

/* main menu */
int nhexMenu(struct nhMenuItem nhexMenuItems[], int iCnt)
{
	int	i, j, iCur=1;
	//char	*p;
	int	c;
	int	iResult=0;
	bool	ready=false, found=false, select=false;
	int	pwidth, pheight;
	int	y, x;
	int	nItems=0;
	char	sItem[MAXMENUCATS][MAXMENUITEMLENGTH+3];	/* 2x space + \0 */
	int	iItemPos[MAXMENUCATS + 1];
	int	iItemVal[MAXMENUCATS];
	char	sItemIndex[MAXMENUCATS]="";
	int	iItemSel=0;
	int	iItemSkip=0;
	int	iItemBase=0;

	getmaxyx(stdscr, pheight, pwidth);

	/* main menu on first line */
	for(i=0; i<iCnt; i++)
	{
		if(nhexMenuItems[i].iRef < 100)
		{
			snprintf(sItem[nItems], MAXMENUITEMLENGTH+2, " %s ", nhexMenuItems[i].sItem);
			iItemPos[nItems]=strlen(sItem[nItems]);
			if(nItems > 0) iItemPos[nItems]+=iItemPos[nItems-1];
			iItemVal[nItems]=nhexMenuItems[i].iRef;
			sItemIndex[nItems]=nhexMenuItems[i].cShortCut;
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
				/* get position of shortcut, first check case-sensitive */
				found=false;
				for(j=1; j<strlen(sItem[i])-1; j++)
				{
					if(sItem[i][j] == sItemIndex[i])
					{
						found=true;
						break;
					}
				}
				if(!found)
				{
					/* check case-insensative */
					for(j=1; j<strlen(sItem[i])-1; j++)
					{
						if((sItem[i][j] & 95) == (sItemIndex[i] & 95))
						{
							found=true;
							break;
						}
					}
				}
				if(found)
				{
					if(i == iItemSel)
						iCur=j;
					else
					{
						mvchgat(0, x+j, 1, 0, 0, NULL);
						move(0, x+strlen(sItem[i]));
					}
				}
				i++;
			}
			else
			{
				mvprintw(0, pwidth-1, ">");
				break;
			}
		}
		chgat(-1, A_REVERSE, 0, NULL);
		attroff(A_REVERSE);
		mvchgat(0, 1 + iItemPos[iItemSel]-strlen(sItem[iItemSel])-iItemBase, strlen(sItem[iItemSel]), 0, 0, NULL);
		move(0, 1 + iItemPos[iItemSel]-strlen(sItem[iItemSel])-iItemBase + iCur);
		refresh();

		if(select)
		{
			/* we have a submenu selected - let's show it */
			getyx(stdscr, y, x);
			move(0, x-iCur+1);
			iResult=nhexSubMenu(nhexMenuItems, iCnt, iItemVal[iItemSel]);
			if(iResult > 100)
			{
				for(i=0; i<iCnt; i++)
				{
					if(nhexMenuItems[i].iRef == iResult)
					{
						iResult=i;
						break;
					}
				}
				ready=true;
				break;
			}
			else if(iResult != 0)
				iItemSel=iResult-1;
			else
				select=false;
		}

		if(!select)
		{
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
				case HNKEY_ENTER:
					select=true;
					break;
				case KEY_F(12):
				case HNKEY_ESC:
					iResult=0;
					ready=true;
					break;
				default:
					c=c & 95;
					for(i=0; i<nItems; i++)
					{
						if(iItemVal[i] != 0 && c == (sItemIndex[i] & 95))
						{
							iItemSel=i;
							select=true;
							break;
						}
					}
					break;
			}
			if(ready) break;
		}
	}

	return iResult;
}

/* submenu */
int nhexSubMenu(struct nhMenuItem nhexMenuItems[], int iCnt, int submenu)
{
	int	i, j, iCur;
	int	c;
	int	iResult;
	int	iSub, maxSub=0;
	bool	ready=false, found=false;
	int	ixPos, iyPos;
	int	pheight, pwidth;
	int	mheight, mwidth;
	int	iMax=0;
	int	nItems=0;
	char	sItem[MAXMENUITEMS][MAXMENUITEMLENGTH+7];	/* " " + item + " " + keycode[3] + " " + \0 */
	char	sTemp[MAXMENUITEMLENGTH+7];
	char	sKeyCode[MAXMENUITEMS][3+1];			/* keycode[3] + \0 */
	int	iItemVal[MAXMENUITEMS];
	char	sItemIndex[MAXMENUITEMS]="";
	int	iItemSel=0;
	WINDOW	*menuWin;
	PANEL	*menuPanel[2];

	getyx(stdscr, iyPos, ixPos);

	getmaxyx(stdscr, pheight, pwidth);
	
	for(i=0; i<iCnt; i++)
	{
		iSub=nhexMenuItems[i].iRef % 100;
		if(nhexMenuItems[i].iRef/100 == 0)
		{
			if(iSub > maxSub) maxSub=iSub;
		}
		else if(nhexMenuItems[i].iRef / 100 == submenu)
		{
			snprintf(sItem[nItems], MAXMENUITEMLENGTH+2, " %s ",nhexMenuItems[i].sItem);
			strcpy(sKeyCode[nItems], nhexMenuItems[i].sKeyCode);
			if(strlen(sItem[nItems]) > iMax) iMax=strlen(sItem[nItems]);
			iItemVal[nItems]=iSub;
			sItemIndex[nItems]=nhexMenuItems[i].cShortCut;
			nItems++;
			if(nItems == MAXMENUITEMS) break;
		}
	}

	/* add keycodes if there are any */
	for(i=0; i<nItems; i++)
	{
		if(strlen(sKeyCode[i]) != 0)
		{
			sprintf(sTemp, "%*s%3s ", -iMax, sItem[i], sKeyCode[i]);
			strcpy(sItem[i], sTemp);
			found=true;
		}
	}
	if(found) iMax+=4;

	/* check for separators */
	for(i=0; i<nItems; i++)
	{
		if(strcmp(sItem[i], " - ") == 0)
		{
			memset(sItem[i], '-', iMax);
			sItem[i][iMax]='\0';
			iItemVal[i]=0;
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

	/* TODO: select first valid option (not necessarily 0, might be disabled...) */

	/* loop through options */
	while(1)
	{
		for(i=0; i<nItems; i++)
		{
			/* get position of shortcut, first check case-sensitive */
			found=false;
			for(j=1; j<strlen(sItem[i])-1; j++)
			{
				if(sItem[i][j] == sItemIndex[i])
				{
					found=true;
					break;
				}
			}
			if(!found)
			{
				/* check case-insensative */
				for(j=1; j<strlen(sItem[i])-1; j++)
				{
					if((sItem[i][j] & 95) == (sItemIndex[i] & 95))
					{
						found=true;
						break;
					}
				}
			}

			mvwprintw(menuWin, 1+i, 1, sItem[i]);
			if(i == iItemSel)
			{
				mvwchgat(menuWin, 1+i, 2, strlen(sItem[i])-2, 0, 0, NULL);
				if(found)
					iCur=j;
				else
					iCur=1;
			}
			else if(sItemIndex[i] != 0 && found)
				mvwchgat(menuWin, 1+i, 1+j, 1, 0, 0, NULL);
		}
		wmove(menuWin, 1+iItemSel, 1+iCur);	/* set cursor on shortcut */
		wrefresh(menuWin);
		update_panels();
		doupdate();
		c=getch();
		switch(c)
		{
			case KEY_UP:
				if(iItemSel > 0)
				{
					/* select previous valid item */
					i=-1;
					while(iItemSel+i >= 0)
					{
						if(iItemVal[iItemSel+i] != 0)
						{
							iItemSel=iItemSel+i;
							break;
						}
						i--;
					}
				}
				else
				{
					iResult=0;
					ready=true;
				}
				break;
			case KEY_DOWN:
				if(iItemSel < nItems-1)
				{
					/* select next valid item */
					i=1;
					while(iItemSel+1 < nItems)
					{
						if(iItemVal[iItemSel+i] != 0)
						{
							iItemSel=iItemSel+i;
							break;
						}
						i++;
					}
				}
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
			case HNKEY_ENTER:
				iResult=submenu*100 + iItemVal[iItemSel];
				ready=true;
				break;
			case HNKEY_ESC:
				iResult=0;
				ready=true;
				break;
			default:
				c=c & 95;
				for(i=0; i<nItems; i++)
				{
					if(iItemVal[i] != 0 && c == (sItemIndex[i] & 95))
					{
						iItemSel=i;
						iResult=submenu*100 + iItemVal[iItemSel];
						ready=true;
						break;
					}
				}
				break;
		}
		if(ready)
		{
			break;
		}
	}

	del_panel(menuPanel[1]);
	delwin(menuWin);
	del_panel(menuPanel[0]);
	update_panels();
	doupdate();

	return iResult;
}

