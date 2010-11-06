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

#include "nhexed.h"
#include "nhexfile.h"
#include "nhexmsg.h"
#include "nhexmenu.h"

static	int		iRows;		/* # of rows available for output */
static	int		iCols;		/* # of coluns available for output */
static	int		iChunks;	/* # of 8-byte chunks per line */

struct	nhexBuff	nhexFile;

/* Show standar header line */
void nhexScreenHeader(void)
{
	mvprintw(0, 0, "%s-v%s",PACKAGE,VERSION);
	clrtoeol();
	mvprintw(0, iCols-21, "|Press <F12> for menu");
	mvchgat(0, 0, -1, A_REVERSE, 0, NULL);
	mvchgat(iRows+1, 0, -1, A_REVERSE, 0, NULL);

	refresh();
}

/* Initialization */
void nhexScreenSetup(void)
{
	int y;

	initscr();

	getmaxyx(stdscr, y, iCols);

	iRows=y-2;			/* -1 for menu, -1 for status */
	iChunks=(iCols-45)/34+1;

	nhexScreenHeader();

	refresh();
}

/* Show file */
void nhexScreenShow(struct nhexBuff *nhexFile)
{
	unsigned int	i=0;
	int		j=-1;
	int		k, spacer;
	unsigned char	c;
	char		style;

	for(i=nhexFile->iOff; i < nhexFile->iFileLength; i++)
	{
		if(i%(iChunks*8) == 0)		/* new line */
		{
			j++;
			k=0;
			spacer=0;
			mvprintw(j+1, 0, "%08X | ", i);
			mvprintw(j+1, 11 + (iChunks*8)*3 + (iChunks-1), "|");
		}

		if(k%8 == 0 && k != -0)
			spacer++;

		c=nhexFileReadPos(nhexFile, i, &style);
		if(style == 'c') attron(A_REVERSE);
		mvprintw(j+1, 11+k*3+spacer, "%02X", c);
		if(c < 32 || c > 126) c='.';
		mvprintw(j+1, 11+(iChunks*8)*3+(iChunks-1)+2+k+spacer, "%c",c);
		if(style == 'c') attroff(A_REVERSE);
		
		k++;
		if(k == iChunks*8 && j == iRows-1)
			i=nhexFile->iFileLength;
	}

	/* clean rest of window */
	if(k > 0)
	{
		for(i=k; i<iChunks*8; i++)
		{
			if(i%8 == 0)
				spacer++;
			mvprintw(j+1, 11+i*3+spacer, "  ");
			mvprintw(j+1, 11+(iChunks*8)*3+(iChunks-1)+2+i+spacer," ");
		}
	}
	if(j < iRows-1)
	{
		for(i=j+1; i<iRows; i++)
		{
			move(i+1, 0);
			for(k=0; k<iCols; k++) printw(" ");
		}
	}

	refresh();
}

/* Show details on screen */
void nhexScreenDetails(struct nhexBuff *nhexFile)
{
	char		sType[6], style;
	unsigned char	c;
	unsigned int	iPos;
	int		iChunkPos;
	int		ixHexPos, ixAscPos, ixCurPos;

	iPos=nhexFile->iOff + nhexFile->iyPos * iChunks*8 + nhexFile->ixPos;
	iChunkPos=nhexFile->ixPos / 8;
	ixHexPos=11+nhexFile->ixPos * 3 + iChunkPos;
	ixAscPos=11+(iChunks*8) * 3 + (iChunks-1) + 2 + nhexFile->ixPos + iChunkPos;

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
	attron(A_REVERSE);
	mvprintw(iRows+1, 0, "%s", nhexFile->sFileName);
	mvprintw(iRows+1, iCols-6, "|%s",sType);
	mvprintw(iRows+1, iCols-26, "|%010u/%08X", iPos, iPos);
	mvprintw(iRows+1, iCols-31, "|%04i", nhexFile->iChangeCnt);
	attroff(A_REVERSE);

	/* show edit-position */
	c=nhexFileReadPos(nhexFile, iPos, &style);
	if(style == 'c') attron(A_REVERSE);
	attron(A_BOLD | A_UNDERLINE);
	mvprintw(nhexFile->iyPos + 1, ixHexPos, "%02X",c);
	if(c < 32 || c > 126) c='.';
	mvprintw(nhexFile->iyPos + 1, ixAscPos, "%c", c);
	attroff(A_BOLD | A_UNDERLINE);
	if(style == 'c') attroff(A_REVERSE);
	refresh();

	/* set cursor on right position */
	move(nhexFile->iyPos + 1,ixCurPos);
	return;
}

/* reset screen attributes on ixPos / iyPos */
void nhexScreenDetReset(struct nhexBuff *nhexFile)
{
	unsigned int	iPos;
	int		iChunkPos, ixHexPos, ixAscPos;
	unsigned char	c;
	char		style;

	iPos=nhexFile->iOff + nhexFile->iyPos * 8 * iChunks + nhexFile->ixPos;
	iChunkPos=nhexFile->ixPos/8;
	ixHexPos=11 + nhexFile->ixPos * 3 + iChunkPos;
	ixAscPos=11 + (iChunks*8) * 3 + (iChunks-1) + 2 + nhexFile->ixPos + iChunkPos;

	c=nhexFileReadPos(nhexFile, iPos, &style);
	if(style == 'c') attron(A_REVERSE);
	mvprintw(nhexFile->iyPos + 1, ixHexPos, "%02X",c);
	if(c < 32 || c > 126) c='.';
	mvprintw(nhexFile->iyPos + 1, ixAscPos, "%c", c);
	if(style == 'c') attroff(A_REVERSE);
	
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

/* move cursor left one pos. */
int nhexMvLeft(struct nhexBuff *nhexFile)
{
	int result=0;			/* 0=nothing, 1=update, 2-redraw */

	if(nhexFile->iOff + nhexFile->iyPos * iChunks*8 + nhexFile->ixPos > 0)
	{
		nhexScreenDetReset(nhexFile);
		nhexFile->ixPos--;
		result=1;
		nhexFile->bHiLo=false;
		if(nhexFile->ixPos < 0)
		{
			nhexFile->ixPos=iChunks*8 - 1;
			if(nhexFile->iyPos == 0)
			{
				nhexFile->iOff-=iChunks*8;
				result=2;
			}
			else
			{
				nhexFile->iyPos--;
			}
		}
	}

	return result;
}

/* move cursor right one pos. */
int nhexMvRight(struct nhexBuff *nhexFile)
{
	int result=0;			/* 0=nothing, 1=update, 2-redraw */

	if(nhexFile->iOff + nhexFile->iyPos * iChunks*8 + nhexFile->ixPos + 1 <= nhexFile->iFileLength - 1)
	{
		nhexScreenDetReset(nhexFile);
		nhexFile->ixPos++;
		result=1;
		nhexFile->bHiLo=false;
		if(nhexFile->ixPos == iChunks*8)
		{
			nhexFile->ixPos=0;
			if(nhexFile->iyPos == iRows-1)
			{
				nhexFile->iOff+=iChunks*8;
				result=2;
			}
			else
			{
				nhexFile->iyPos++;
			}
		}
	}

	return result;
}

/* main entry point */
int main(int argc, char *argv[])
{
	struct nhexBuff	nhexFile;
	int 		c;
	bool 		ready, scrUpdate, scrRedraw;
	int		iRes;		/* temp result of functions etc. */
	unsigned int	iTmp;		/* temp address for moves etc. */
	unsigned char	cTmp;		/* temp char for editing etc. */
	char		style;

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
	strcpy(nhexFile.sFileName,argv[1]);

	if(nhexSanityCheck())
	{
		exit(1);
	}

	nhexFile.fp=nhexFileReadOpen(nhexFile.sFileName, &nhexFile.iFileLength);
	if (nhexFile.fp == NULL)
	{
		//perror("Cannot open file");
		printf("** Error opening file\n");
		exit(1);
	}

	nhexFile.iOff=0;				/* initial position */
	nhexFile.ixPos=0;
	nhexFile.iyPos=0;
	nhexFile.iChangeCnt=0;
	nhexFile.bPos=false;
	nhexFile.bHiLo=false;
	
	nhexScreenSetup();
	nhexScreenShow(&nhexFile);
	nhexScreenDetails(&nhexFile);

	/* Main loop */
	noecho();
	cbreak();
	keypad(stdscr, true);
	ready=false;
	while(1)
	{
		c=getch();
		scrUpdate=false;
		scrRedraw=false;
		switch(c)
		{	case KEY_UP:
				if(nhexFile.iyPos == 0)
				{
					if(nhexFile.iOff >= iChunks*8)
					{
						nhexFile.iOff-=iChunks*8;
						scrRedraw=true;
					}
				}
				else
				{
					nhexScreenDetReset(&nhexFile);
					nhexFile.iyPos--;
					scrUpdate=true;
					nhexFile.bHiLo=false;
				}
				break;
			case KEY_DOWN:
				if(nhexFile.iOff + (nhexFile.iyPos+1) * iChunks*8 + nhexFile.ixPos <= nhexFile.iFileLength - 1)
				{
					if(nhexFile.iyPos == iRows-1)
					{
						nhexFile.iOff+=iChunks*8;
						scrRedraw=true;
					}
					else
					{
						nhexScreenDetReset(&nhexFile);
						nhexFile.iyPos++;
						scrUpdate=true;
						nhexFile.bHiLo=false;
					}
				}
				break;		
			case KEY_LEFT:
				iRes=nhexMvLeft(&nhexFile);
				if(iRes == 1) scrUpdate=true;
				if(iRes == 2) scrRedraw=true;
				break;
			case KEY_RIGHT:
				iRes=nhexMvRight(&nhexFile);
				if(iRes == 1) scrUpdate=true;
				if(iRes == 2) scrRedraw=true;
				break;
			case KEY_NPAGE:
				if(nhexFile.iOff + iRows * iChunks*8 < nhexFile.iFileLength)
				{
					nhexFile.iOff+=iRows * iChunks*8;
					if(nhexFile.iOff + nhexFile.iyPos * iChunks*8 + nhexFile.ixPos > nhexFile.iFileLength - 1)
					{
						nhexFile.iyPos=(nhexFile.iFileLength-1 - nhexFile.iOff) / (iChunks*8);
						if(nhexFile.ixPos > (nhexFile.iFileLength - 1) % (iChunks*8))
							nhexFile.ixPos=(nhexFile.iFileLength - 1) % (iChunks*8);
					}
					scrRedraw=true;
				}
				break;
			case KEY_PPAGE:
				if(nhexFile.iOff > 0)
				{
					if(nhexFile.iOff >= iRows*iChunks*8)
					{
						nhexFile.iOff-=iRows*iChunks*8;
					}
					else
					{
						nhexFile.iOff=0;
						nhexFile.iyPos=0;
					}
					scrRedraw=true;
				}
				break;
			case KEY_SLEFT:
				nhexScreenDetReset(&nhexFile);
				nhexFile.ixPos=0;
				scrUpdate=true;
				nhexFile.bHiLo=false;
				break;
			case KEY_SRIGHT:
				nhexScreenDetReset(&nhexFile);
				nhexFile.ixPos=iChunks*8 - 1;
				if(nhexFile.iOff + nhexFile.iyPos * iChunks*8 + nhexFile.ixPos > nhexFile.iFileLength)
					nhexFile.ixPos=(nhexFile.iFileLength-1) % (iChunks*8);
				scrUpdate=true;
				nhexFile.bHiLo=false;
				break;
			case KEY_HOME:
				nhexFile.iOff=0;
				nhexFile.ixPos=0;
				nhexFile.iyPos=0;
				scrRedraw=true;
				break;
			case KEY_END:
				nhexFile.iOff=(nhexFile.iFileLength-1) / (iChunks*8);
				nhexFile.iOff=(nhexFile.iOff - iRows+1) * iChunks*8;
				if(nhexFile.iOff < 0) nhexFile.iOff=0;
				nhexFile.iyPos=(nhexFile.iFileLength-1 - nhexFile.iOff) / (iChunks*8);
				nhexFile.ixPos=(nhexFile.iFileLength-1) % (iChunks*8);
				scrRedraw=true;
				break;
			case 9:
				/* tab - switch between hex & ascii */
				nhexFile.bPos=!nhexFile.bPos;
				nhexFile.bHiLo=false;
				scrUpdate=true;
				break;
			case 24:
				/* backspace - undo last change */
				if(nhexFile.iChangeCnt>0)
				{
					iTmp=nhexFile.iChangeAddr[nhexFile.iChangeCnt-1];
					if(iTmp >= nhexFile.iOff && iTmp<=nhexFile.iOff + iRows*iChunks*8 - 1)
					{
						/* last change is on the screen */
						nhexScreenDetReset(&nhexFile);
						nhexFile.iyPos=(iTmp - nhexFile.iOff) / (iChunks*8);
						nhexFile.ixPos=(iTmp - nhexFile.iOff) % (iChunks*8);
						scrUpdate=true;
						nhexFile.bHiLo=false;
					}
					else
					{
						/* need to move the offset, try to center */
						if(iTmp < (iRows/2) * (iChunks*8))
							nhexFile.iOff=0;
						else
							nhexFile.iOff=(iTmp / (iChunks*8) - (iRows/2)) * (iChunks*8);
						//if(nhexFile.iOff < 0) nhexFile.iOff=0;
						nhexFile.iyPos=(iTmp - nhexFile.iOff) / (iChunks*8);
						nhexFile.ixPos=iTmp % (iChunks*8);
						scrRedraw=true;
					}
					nhexFile.iChangeCnt--;
				}
				break;
			case KEY_F(1):
			case KEY_F(12):
				/* menu */
				iRes=nhexMenu();
				scrRedraw=true;
				break;
			case KEY_CANCEL:
				ready=true;
				break;
			default:
				/* 'normal' character */
				if(!nhexFile.bPos)
				{
					/* hex editing */
					if(c >= '0' && c<= '9')
					{
						cTmp=c-48;
					}
					else
					{
						c=c & 95;
						if(c>='A' && c<='F')
						{
							cTmp=c-55;
						}
						else
							break;
					}
					if(!nhexFile.bHiLo)
					{
						cTmp=cTmp*16;
						if(nhexFile.iChangeCnt >= MAXCHANGE)
						{
							nhexMsg(NHMSGWARN + NHMSGOK, "Maximum number of changes reached");
							scrRedraw=true;
							break;
						}
						/* calculate address & change high bits */
						nhexFile.iChangeAddr[nhexFile.iChangeCnt]=nhexFile.iOff + nhexFile.iyPos * iChunks*8 + nhexFile.ixPos;
						nhexFile.cChangeByte[nhexFile.iChangeCnt]=(nhexFileReadPos(&nhexFile, \
									nhexFile.iChangeAddr[nhexFile.iChangeCnt], &style) & 15) + cTmp;
						nhexFile.iChangeCnt++;
						nhexFile.bHiLo=true;
						scrUpdate=true;
					}
					else
					{
						/* change low bits of last change */
						nhexFile.cChangeByte[nhexFile.iChangeCnt-1]=(nhexFile.cChangeByte[nhexFile.iChangeCnt-1] & 240) + cTmp;
						nhexFile.bHiLo=false;
						scrUpdate=true;
						iRes=nhexMvRight(&nhexFile);
						if(iRes == 2) scrRedraw=true;
					}
				}
				else
				{
					/* ascii editing */
					if(c>=32 && c<=126)
					{
						if(nhexFile.iChangeCnt >= MAXCHANGE)
						{
							nhexMsg(NHMSGWARN + NHMSGOK, "Maximum number of changes reached");
							scrRedraw=true;
							break;
						}
						nhexFile.iChangeAddr[nhexFile.iChangeCnt]=nhexFile.iOff + nhexFile.iyPos * iChunks*8 + nhexFile.ixPos;
						nhexFile.cChangeByte[nhexFile.iChangeCnt]=c;
						nhexFile.iChangeCnt++;
						scrUpdate=true;
						iRes=nhexMvRight(&nhexFile);
						if(iRes == 2) scrRedraw=true;
					}
				}
				break;
		}
		if(scrRedraw)
		{
			nhexScreenHeader();
			nhexScreenShow(&nhexFile);
			nhexFile.bHiLo=false;
		}
		if(scrUpdate || scrRedraw)
			nhexScreenDetails(&nhexFile);
		if(ready) break;
	}

	fclose(nhexFile.fp);

	nhexCleanup();
	return 0;
}
