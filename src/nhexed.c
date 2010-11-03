/*
 * nhexed.c - main program of nHex-Ed
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "nhexfile.h"

#define DEBUG		1
#define MINCOLUMNS	43		/* 43=1 chunk, 34 for each extra chunk */
#define MINROWS		3

static	int		iRows;		/* # of rows available for output */
static	int		iCols;		/* # of coluns available for output */
static	int		iChunks;	/* # of 8-byte chunks per line */

/* Initialization */
void nhexScreenSetup(void)
{
	int y;

	initscr();

	getmaxyx(stdscr, y, iCols);

	iRows=y-2;			/* -1 for menu, -1 for status */
	iChunks=(iCols-43)/34+1;

	mvprintw(0, 0, "%s-v%s",PACKAGE,VERSION);
	mvprintw(0, iCols-20, "|Press <F1> for menu");
	mvchgat(0, 0, -1, A_REVERSE, 0, NULL);
	mvchgat(iRows+1, 0, -1, A_REVERSE, 0, NULL);

	refresh();
	return;
}

/* Show file */
void nhexScreenShow(FILE *fp, int iOff, int iFileLength)
{
	int i=0, j=-1;
	int k, spacer;
	int c;

	if(fseek(fp, (long)iOff, SEEK_SET))
	{
		printf("** Seek error\n");
		exit(1);
	}

	for(i=iOff; i < iFileLength; i++)
	{
		if (i%(iChunks*8) == 0)		/* new line */
		{
			j++;
			k=0;
			spacer=0;
			mvprintw(j+1, 0, "%06X | ", i);
			mvprintw(j+1, 9+(iChunks*8)*3+(iChunks-1), "|");
		}

		c=getc(fp);

		if (k%8 == 0 && k != -0)
			spacer++;

		mvprintw(j+1, 9+k*3+spacer, "%02X", c);
		if (c < 32 || c > 126) c=(int)'.';
		mvprintw(j+1, 9+(iChunks*8)*3+(iChunks-1)+2+k+spacer, "%c",c);
		
		k++;
		if (k == iChunks*8 && j == iRows-1)
			i=iFileLength;
	}

	attron(A_REVERSE);
	mvprintw(iRows+1,0,"[j = %i]",j);
	attroff(A_REVERSE);

	refresh();
}

/* Show details on screen */
void nhexScreenDetails(char *pFileName, int iOff, int ixPos, int iyPos, bool bPos, bool bHiLo)
{
	char sType[6];
	int iPos;
	int iChunkPos;
	int ixHexPos, ixAscPos, ixCurPos;

	iPos=iOff+iyPos*8*iChunks+ixPos;
	iChunkPos=ixPos/8;
	ixHexPos=9+ixPos*3+iChunkPos;
	ixAscPos=9+(iChunks*8)*3+(iChunks-1)+2+ixPos+iChunkPos;

	if(bPos)
	{
		strcpy(sType, "ASCII");
		ixCurPos=ixAscPos;
	}
	else
	{
		if (bHiLo)
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
	attron(A_REVERSE);
	mvprintw(iRows+1,0,"%s",pFileName);
	mvprintw(iRows+1,iCols-6,"|%s",sType);
	mvprintw(iRows+1,iCols-6-16,"|%08i/%06X", iPos, iPos);
	attroff(A_REVERSE);

	/* show edit-position */
	mvchgat(iyPos+1, ixHexPos, 2, A_BOLD | A_UNDERLINE, 0, NULL);
	mvchgat(iyPos+1, ixAscPos, 1, A_BOLD | A_UNDERLINE, 0, NULL);
	refresh();

	/* set cursor on right position */
	move(iyPos+1,ixCurPos);
	return;
}

/* Cleanup */
void nhexCleanup(void)
{
	clrtoeol();
	endwin();
	return;
}

/* Check for minimal window size etc. */
int nhexSanityCheck(void)
{
	int y, x;
	int ret = 0;

	initscr();
	getmaxyx(stdscr, y, x);
	endwin();

	if (x < MINCOLUMNS)
	{
		printf("** Columns = %i, minimum = %i\n", x, MINCOLUMNS);
		ret = 1;
	}
	if (y < MINROWS)
	{
		printf("** Rows = %i, minimum = %i\n", y, MINROWS);
		ret = 1;
	}
	return ret;
}

/* main entry point */
int main(int argc, char *argv[])
{
	/*
	 * TODO:
	 * - more options
	 *   -v	(verbose)
	 *   -i (input file?)
	 *   --position= (initial position in file)
	 *   --find=xxx
	 *   --version
	 */

	FILE *fp;
	int iFileLength;
	int iOff=0;			/* Offset first byte on screen from file */
	int ixPos=0, iyPos=0;		/* Position of 'cursor' */
	bool bPos=false;		/* false = hex / true = ascii */
	bool bHiLo=false;		/* false = High / true = Low */
	//bool bNeedSave=false;
	char sFileName[MAXFILENAME];
	int c;
	bool ready, update;

	if(argc != 2)
	{
		printf("Usage: %s <file_to_edit>\n", argv[0]);
		exit(1);
	}

	if(strlen(argv[1]) > MAXFILENAME)
	{
		printf("** Filename too long (max=%i)",MAXFILENAME);
		exit(1);
	}
	strcpy(sFileName,argv[1]);

	if(nhexSanityCheck())
	{
		exit(1);
	}

	fp=nhexFileReadOpen(sFileName, &iFileLength);
	if (fp == NULL)
	{
		//perror("Cannot open file");
		printf("** Error opening file\n");
		exit(1);
	}

	iOff=0;				/* initial position */
	ixPos=0;
	iyPos=0;
	bPos=false;
	
	nhexScreenSetup();
	nhexScreenShow(fp, iOff, iFileLength);
	nhexScreenDetails(sFileName, iOff, ixPos, iyPos, bPos, bHiLo);

	/* Main loop */
	noecho();
	cbreak();
	keypad(stdscr, true);
	ready=false;
	while(1)
	{
		c=getch();
		update=false;
		switch(c)
		{	case KEY_UP:
				if(iyPos > 0) iyPos--;
				update=true;
				break;
			case KEY_DOWN:
				if(iyPos < iRows-1) iyPos++;
				update=true;
				break;
			case KEY_LEFT:
				if(ixPos > 0) ixPos--;
				update=true;
				break;
			case KEY_RIGHT:
				if(ixPos < iChunks*8-1) ixPos++;
				update=true;
				break;
			case KEY_NPAGE:
				if(iOff+iRows*iChunks*8 < iFileLength) iOff+=iRows*iChunks*8;
				update=true;
				break;
			case KEY_PPAGE:
				if(iOff >= iRows*iChunks*8) iOff-=iRows*iChunks*8;
				update=true;
				break;
			/*case 27:
				ready=true;
				break;*/
			case KEY_CANCEL:
				ready=true;
				break;
			default:
				attron(A_REVERSE);
				mvprintw(iRows+1,iCols-6-16-4,"|%3d",c);
				//printw("|%3d-",c);
				attroff(A_REVERSE);
				//printf("[%3d]",c);
				break;
		}
		if (update)
		{
			nhexScreenShow(fp, iOff, iFileLength);
			nhexScreenDetails(sFileName, iOff, ixPos, iyPos, bPos, bHiLo);
		}
		if(ready) break;
	}

	fclose(fp);

	nhexCleanup();
	return 0;
}
