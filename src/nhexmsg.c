/*
 * nhexmsg.c - message routines for nHex-Ed
 *
 * Copyright (C) 2010 Niels Horn <niels.horn@gmail.com>
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

#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#include "nhexed.h"
#include "nhexmsg.h"

int	nButton=0;
char	sButton[80]="";
int	iButPos[11];			/* one longer to hold end position */
int	iButVal[10];

WINDOW *msgBoxCreate(int height, int width, int startx, int starty)
{
	WINDOW	*msgWin;
	int	i;

	msgWin=newwin(height, width, starty, startx);
	wattron(msgWin, A_REVERSE);
	wborder(msgWin, '|', '|', '-', '-', '+', '+', '+', '+');
	for(i=0; i<height; i++) mvwchgat(msgWin, i, 0, -1, A_REVERSE, 0, NULL);
	return msgWin;
}

void msgBoxDestroy(WINDOW *msgWin)
{
	delwin(msgWin);
}

void msgButAdd(char *button, int val)
{
	iButPos[nButton]=strlen(sButton);
	iButVal[nButton]=val;
	sprintf(sButton, "%s[ %s ] ", sButton, button);
	nButton++;
}

/* show popup message */
int nhexMsg(int flags, char *msg)
{
	int	iResult=0;
	int	width, height;
	int	pwidth, pheight;
	WINDOW	*msgWin;
	char	mType[10]="";
	bool	ready=false;
	int	iButSel=0;
	int	c;

	nButton=0;
	strcpy(sButton, "");

	getmaxyx(stdscr, pheight, pwidth);

	if(flags & NHMSGCANCEL) msgButAdd("cancel", NHMSGCANCEL);
	if(flags & NHMSGNO) msgButAdd("no", NHMSGNO);
	if(flags & NHMSGYES) msgButAdd("yes", NHMSGYES);
	if(flags & NHMSGOK) msgButAdd("ok", NHMSGOK);

	if(strlen(msg) > strlen(sButton)+1)
	{
		width=strlen(msg)+4;
	}
	else
	{
		width=strlen(sButton)+3;
	}

	if(width > pwidth) width=pwidth;
	height=6;

	msgWin=msgBoxCreate(height, width, (pwidth - width)/2, (pheight - height)/2);
	switch(flags & 65280)
	{
		case NHMSGINFO:
			strcpy(mType, "INFO");
			break;
		case NHMSGWARN:
			strcpy(mType, "WARNING");
			break;
		case NHMSGERR:
			strcpy(mType, "ERROR");
			break;
	}
	wattroff(msgWin, A_REVERSE);
	mvwprintw(msgWin,0,(width-strlen(mType))/2,"%s", mType);
	wattron(msgWin, A_REVERSE);
	wattron(msgWin, A_BOLD);
	mvwprintw(msgWin,2,1," %s ",msg);
	wattroff(msgWin, A_BOLD);

	mvwprintw(msgWin,4,width-strlen(sButton)-1,sButton);

	/* cycle through buttons */
	iButPos[nButton]=strlen(sButton);
	while(1)
	{
		mvwchgat(msgWin, 4, width-strlen(sButton)-1 + iButPos[iButSel], \
				iButPos[iButSel+1]-iButPos[iButSel]-1, 0, 0, NULL);
		wmove(msgWin, 4, width-strlen(sButton)-1 + iButPos[iButSel]+2);
		wrefresh(msgWin);
		c=getch();
		switch(c)
		{
			case KEY_LEFT:
				if(iButSel > 0) iButSel--;
				mvwchgat(msgWin, 4, 2, width-4, A_REVERSE, 0, NULL);
				break;
			case KEY_RIGHT:
				if(iButSel < nButton-1) iButSel++;
				mvwchgat(msgWin, 4, 2, width-4, A_REVERSE, 0, NULL);
				break;
			case KEY_HOME:
				iButSel=0;
				mvwchgat(msgWin, 4, 2, width-4, A_REVERSE, 0, NULL);
				break;
			case KEY_END:
				iButSel=nButton-1;
				mvwchgat(msgWin, 4, 2, width-4, A_REVERSE, 0, NULL);
				break;
			case KEY_ENTER:
			case KEY_DOWN:
			case 10:
				iResult=iButVal[iButSel];
				ready=true;
				break;
		}
		if(ready) break;
	}

	msgBoxDestroy(msgWin);
	refresh();

	return iResult;
}
