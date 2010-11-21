/*
 * nhexscreen.c - screen routines for nHex-Ed
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
#include <string.h>

#include "nhexed.h"
#include "nhexfile.h"
#include "nhexscreen.h"

struct	Screen	nhexScreen;

/* Show standard header line */
void nhexScreenHeader(void)
{
	mvprintw(0, 0, "%s-v%s",PACKAGE,VERSION);
	clrtoeol();
	mvprintw(0, nhexScreen.iCols-21, "|Press <F12> for menu");
	mvchgat(0, 0, -1, A_REVERSE, 0, NULL);
	mvchgat(nhexScreen.iRows+1, 0, -1, A_REVERSE, 0, NULL);
	refresh();
}

/* Initialization */
void nhexScreenSetup(struct Screen *nhexScreen)
{
	int		y;

	initscr();

	getmaxyx(stdscr, y, nhexScreen->iCols);

	nhexScreen->iRows=y-2;			/* -1 for menu, -1 for status */
	nhexScreen->iChunks=(nhexScreen->iCols-45)/34+1;
}

/* Show one byte */
void nhexScreenShowByte(struct nhexBuff *nhexFile, int i, int iyPos, int ixHexPos, int ixAscPos)
{
	char		style;
	unsigned char	c;

	c=nhexFileReadPos(nhexFile, i, &style);
	if(style == 'c') attron(A_REVERSE);
	mvprintw(iyPos+1, ixHexPos, "%02X", c);
	if(c < 32 || c > 126) c='.';
	mvprintw(iyPos+1, ixAscPos, "%c", c);
	if(style == 'c') attroff(A_REVERSE);
}

/* Show file */
void nhexScreenShow(struct nhexBuff *nhexFile)
{
	unsigned int	i=0;
	int		j=-1;
	int		k=0, spacer=0;
	int		ixHexPos, ixAscPos;

	for(i=nhexFile->iOff; i < nhexFile->iFileLength; i++)
	{
		if(i%(nhexScreen.iChunks*8) == 0)		/* new line */
		{
			j++;
			k=0;
			spacer=0;
			move(j+1, 0);
			clrtoeol();
			printw("%08X | ", i);
			mvprintw(j+1, 11 + (nhexScreen.iChunks*8)*3 + (nhexScreen.iChunks-1), "|");
		}

		if(k%8 == 0 && k != -0)
			spacer++;

		ixHexPos=11+k*3+spacer;
		ixAscPos=11+(nhexScreen.iChunks*8)*3+(nhexScreen.iChunks-1)+2+k+spacer;

		nhexScreenShowByte(nhexFile, i, j, ixHexPos, ixAscPos);

		k++;
		if(k == nhexScreen.iChunks*8 && j == nhexScreen.iRows-1)
			i=nhexFile->iFileLength;
	}

	/* clean rest of window */
	if(k > 0)
	{
		for(i=k; i<nhexScreen.iChunks*8; i++)
		{
			if(i%8 == 0)
				spacer++;
			mvprintw(j+1, 11+i*3+spacer, "  ");
			mvprintw(j+1, 11+(nhexScreen.iChunks*8)*3+(nhexScreen.iChunks-1)+2+i+spacer," ");
		}
	}
	if(j < nhexScreen.iRows-1)
	{
		for(i=j+1; i<nhexScreen.iRows; i++)
		{
			move(i+1, 0);
			for(k=0; k<nhexScreen.iCols; k++) printw(" ");
		}
	}

	refresh();
}

/* Show details on screen */
void nhexScreenDetails(struct nhexBuff *nhexFile)
{
	char		sType[6];
	unsigned int	iPos;
	int		iChunkPos;
	int		ixHexPos, ixAscPos, ixCurPos;
	char		*p;

	iPos=nhexFile->iOff + nhexFile->iyPos * nhexScreen.iChunks*8 + nhexFile->ixPos;
	iChunkPos=nhexFile->ixPos / 8;
	ixHexPos=11+nhexFile->ixPos * 3 + iChunkPos;
	ixAscPos=11+(nhexScreen.iChunks*8) * 3 + (nhexScreen.iChunks-1) + 2 + nhexFile->ixPos + iChunkPos;

	if(nhexFile->bPos)
	{
		strcpy(sType, "ASCII");
		ixCurPos=ixAscPos;
	}
	else
	{
		if (nhexFile->bHiLo)
		{
			strcpy(sType, "HEX-L");
			ixCurPos=ixHexPos+1;
		}
		else
		{
			strcpy(sType, "HEX-H");
			ixCurPos=ixHexPos;
		}
	}

	/* show filename, pos & area */
	move(nhexScreen.iRows+1, 0);
	clrtoeol();
	chgat(-1, A_REVERSE, 0, NULL);
	attron(A_REVERSE);
	if(nhexFile->iChangeCnt) printw("+ ");
	if(nhexFile->fp)
	{
		p=strrchr(nhexFile->sFileName,'/');
		if(p)
			p++;
		else
			p=nhexFile->sFileName;
		printw("%s", p);
	}
	mvprintw(nhexScreen.iRows+1, nhexScreen.iCols-6, "|%s",sType);
	mvprintw(nhexScreen.iRows+1, nhexScreen.iCols-26, "|%010u/%08X", iPos, iPos);
	mvprintw(nhexScreen.iRows+1, nhexScreen.iCols-31, "|%04i", nhexFile->iChangeCnt);
	attroff(A_REVERSE);

	/* show edit-position & set cursor */
	if(nhexFile->fp){
		nhexScreenShowByte(nhexFile, iPos, nhexFile->iyPos, ixHexPos, ixAscPos);
		move(nhexFile->iyPos + 1, ixCurPos);
	}
	else
	{
		mvprintw(nhexScreen.iRows/2-1, (nhexScreen.iCols-7)/2, "nHex-Ed");
		mvprintw(nhexScreen.iRows/2+1, (nhexScreen.iCols-23)/2, "Pess <F12> for the menu");
	}

	refresh();

	return;
}

/* reset screen attributes on ixPos / iyPos */
void nhexScreenDetReset(struct nhexBuff *nhexFile)
{
	unsigned int	iPos;
	int		iChunkPos, ixHexPos, ixAscPos;

	iPos=nhexFile->iOff + nhexFile->iyPos * 8 * nhexScreen.iChunks + nhexFile->ixPos;
	iChunkPos=nhexFile->ixPos/8;
	ixHexPos=11 + nhexFile->ixPos * 3 + iChunkPos;
	ixAscPos=11 + (nhexScreen.iChunks*8) * 3 + (nhexScreen.iChunks-1) + 2 + nhexFile->ixPos + iChunkPos;

	if(nhexFile->fp)
		nhexScreenShowByte(nhexFile, iPos, nhexFile->iyPos, ixHexPos, ixAscPos);

	return;
}

