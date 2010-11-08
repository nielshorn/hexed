/*
 * nhexed.c - main program of nHex-Ed
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
#include <stdlib.h>
#include <string.h>

#include "nhexed.h"
#include "nhexfile.h"
#include "nhexscreen.h"
#include "nhexfunc.h"
#include "nhexmsg.h"
#include "nhexmenu.h"

struct	nhexBuff	nhexFile;
struct 	Screen		nhexScreen;

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

	if(nhexFile->iOff + nhexFile->iyPos * nhexScreen.iChunks*8 + nhexFile->ixPos > 0)
	{
		nhexScreenDetReset(nhexFile);
		nhexFile->ixPos--;
		result=1;
		nhexFile->bHiLo=false;
		if(nhexFile->ixPos < 0)
		{
			nhexFile->ixPos=nhexScreen.iChunks*8 - 1;
			if(nhexFile->iyPos == 0)
			{
				nhexFile->iOff-=nhexScreen.iChunks*8;
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
	int result=0;			/* 0=nothing, 1=update, 2=redraw */

	if(nhexFile->iOff + nhexFile->iyPos * nhexScreen.iChunks*8 + nhexFile->ixPos + 1 <= nhexFile->iFileLength - 1)
	{
		nhexScreenDetReset(nhexFile);
		nhexFile->ixPos++;
		result=1;
		nhexFile->bHiLo=false;
		if(nhexFile->ixPos == nhexScreen.iChunks*8)
		{
			nhexFile->ixPos=0;
			if(nhexFile->iyPos == nhexScreen.iRows-1)
			{
				nhexFile->iOff+=nhexScreen.iChunks*8;
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

/* undo last change */
int nhexUndoLast(struct nhexBuff *nhexFile)
{
	int iTmp;
	int result=0;			/* 0=nothing, 1=update, 2=redraw */

	if(nhexFile->iChangeCnt>0)
	{
		iTmp=nhexFile->iChangeAddr[nhexFile->iChangeCnt-1];
		if(iTmp >= nhexFile->iOff && iTmp<=nhexFile->iOff + nhexScreen.iRows*nhexScreen.iChunks*8 - 1)
		{
			/* last change is on the screen */
			nhexScreenDetReset(nhexFile);
			nhexFile->iyPos=(iTmp - nhexFile->iOff) / (nhexScreen.iChunks*8);
			nhexFile->ixPos=(iTmp - nhexFile->iOff) % (nhexScreen.iChunks*8);
			result=1;
			nhexFile->bHiLo=false;
		}
		else
		{
			/* need to move the offset, try to center */
			if(iTmp < (nhexScreen.iRows/2) * (nhexScreen.iChunks*8))
				nhexFile->iOff=0;
			else
				nhexFile->iOff=(iTmp / (nhexScreen.iChunks*8) - (nhexScreen.iRows/2)) * (nhexScreen.iChunks*8);
			//if(nhexFile.iOff < 0) nhexFile.iOff=0;
			nhexFile->iyPos=(iTmp - nhexFile->iOff) / (nhexScreen.iChunks*8);
			nhexFile->ixPos=iTmp % (nhexScreen.iChunks*8);
			result=2;
		}
		nhexFile->iChangeCnt--;
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
	//unsigned int	iTmp;		/* temp address for moves etc. */
	unsigned char	cTmp;		/* temp char for editing etc. */
	char		style;

	/* verifiy and process command-line options */
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

	/* check if we have a capable terminal */
	if(nhexSanityCheck())
	{
		exit(1);
	}

	/* open file and start editing! */
	nhexFile.fp=nhexFileReadOpen(nhexFile.sFileName, &nhexFile.iFileLength);
	if (nhexFile.fp == NULL)
	{
		/* did not work :( Clean up & exit */
		nhexCleanup();
		exit(1);
	}

	nhexFile.iOff=0;				/* initial position */
	nhexFile.ixPos=0;
	nhexFile.iyPos=0;
	nhexFile.iChangeCnt=0;
	nhexFile.bPos=false;
	nhexFile.bHiLo=false;
	
	nhexScreenSetup(&nhexScreen);
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
		{
			/*
			 * TODO: include keys defined in menu
			 * and call their functions
			 */
			case KEY_UP:
				if(nhexFile.iyPos == 0)
				{
					if(nhexFile.iOff >= nhexScreen.iChunks*8)
					{
						nhexFile.iOff-=nhexScreen.iChunks*8;
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
				if(nhexFile.iOff + (nhexFile.iyPos+1) * nhexScreen.iChunks*8 + nhexFile.ixPos <= nhexFile.iFileLength - 1)
				{
					if(nhexFile.iyPos == nhexScreen.iRows-1)
					{
						nhexFile.iOff+=nhexScreen.iChunks*8;
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
				if(nhexFile.iOff + nhexScreen.iRows * nhexScreen.iChunks*8 < nhexFile.iFileLength)
				{
					nhexFile.iOff+=nhexScreen.iRows * nhexScreen.iChunks*8;
					if(nhexFile.iOff + nhexFile.iyPos * nhexScreen.iChunks*8 + nhexFile.ixPos > nhexFile.iFileLength - 1)
					{
						nhexFile.iyPos=(nhexFile.iFileLength-1 - nhexFile.iOff) / (nhexScreen.iChunks*8);
						if(nhexFile.ixPos > (nhexFile.iFileLength - 1) % (nhexScreen.iChunks*8))
							nhexFile.ixPos=(nhexFile.iFileLength - 1) % (nhexScreen.iChunks*8);
					}
					scrRedraw=true;
				}
				break;
			case KEY_PPAGE:
				if(nhexFile.iOff > 0)
				{
					if(nhexFile.iOff >= nhexScreen.iRows*nhexScreen.iChunks*8)
					{
						nhexFile.iOff-=nhexScreen.iRows*nhexScreen.iChunks*8;
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
				nhexFile.ixPos=nhexScreen.iChunks*8 - 1;
				if(nhexFile.iOff + nhexFile.iyPos * nhexScreen.iChunks*8 + nhexFile.ixPos > nhexFile.iFileLength)
					nhexFile.ixPos=(nhexFile.iFileLength-1) % (nhexScreen.iChunks*8);
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
				nhexFile.iOff=(nhexFile.iFileLength-1) / (nhexScreen.iChunks*8);
				if(nhexFile.iOff < nhexScreen.iRows+1)
					nhexFile.iOff=0;
				else
					nhexFile.iOff=(nhexFile.iOff - nhexScreen.iRows+1) * nhexScreen.iChunks*8;
				nhexFile.iyPos=(nhexFile.iFileLength-1 - nhexFile.iOff) / (nhexScreen.iChunks*8);
				nhexFile.ixPos=(nhexFile.iFileLength-1) % (nhexScreen.iChunks*8);
				scrRedraw=true;
				break;
			case 9:
				/* tab - switch between hex & ascii */
				nhexFile.bPos=!nhexFile.bPos;
				nhexFile.bHiLo=false;
				scrUpdate=true;
				break;
			case 24:
				/* ^X - undo last change */
				iRes=nhexUndoLast(&nhexFile);
				if(iRes == 1) scrUpdate=true;
				if(iRes == 2) scrRedraw=true;
				break;
			case KEY_F(1):
			case KEY_F(12):
			case 27:
				/* we accept F1 / F12 / Esc for menu */
				iRes=nhexMenu();
				if(iRes == 201)
					iRes=nhexUndoLast(&nhexFile);
				else if(iRes != 0)
					iRes=nhexFunctions(iRes, &nhexFile);
				if(iRes == -1)
					ready=true;
				else
					scrRedraw=true;
				break;
			case KEY_CANCEL:
				iRes=nhexFunctions(104, &nhexFile);
				if(iRes == -1)
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
						nhexFile.iChangeAddr[nhexFile.iChangeCnt]=nhexFile.iOff + nhexFile.iyPos * nhexScreen.iChunks*8 + nhexFile.ixPos;
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
						nhexFile.iChangeAddr[nhexFile.iChangeCnt]=nhexFile.iOff + nhexFile.iyPos * nhexScreen.iChunks*8 + nhexFile.ixPos;
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
		/*
		 * TODO: Add routine to update menu here,
		 * enabling "save" / "undo" if iChangeCnt != 0
		 */
		if(ready) break;
	}

	fclose(nhexFile.fp);

	nhexCleanup();
	return 0;
}
