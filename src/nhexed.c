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

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <ncurses.h>

#include "nhexed.h"
#include "nhexfile.h"
#include "nhexscreen.h"
#include "nhexfunc.h"
#include "nhexmsg.h"
#include "nhexmenu.h"

struct	nhexBuff	nhexFile;
struct 	Screen		nhexScreen;

/* menu structure
 * hierarchy,name,shortcut,functioncode,direct key
 * TODO: checkbox / enabled-disabled
 */
struct nhMenuItem nhexMenuItems[]={
	{  1,"File"      ,'F'},
	{101,"Open..."   ,'O',"FileOpen"      ,"^O"},
	{102,"Save"      ,'S',"FileSave"},
	{103,"Save As...",'A',"FileSaveAs"},
	{104,"-"},
	{105,"Exit"      ,'X',"FileExit"      ,"^E"},
	{  2,"Edit"      ,'E'},
	{201,"Undo last" ,'U',"EditUndoLast"  ,"^X"},
	{202,"Undo All"  ,'A',"EditUndoAll"},
	{  3,"Search"    ,'S'},
	{301,"Find..."   ,'F',"SearchFind"    ,"^F"},
	{302,"Find Next" ,'N',"SearchFindNext","F3"},
	{303,"-"},
	{304,"Goto..."   ,'G',"SearchGoto"    ,"^G"},
	{  4,"Help"      ,'H'},
	{401,"Help"      ,'H',"HelpHelp"      ,"F1"},
	{402,"About"     ,'A',"HelpAbout"},
};

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
		fprintf(stderr, "** Columns = %i, minimum = %i\n", x, MINCOLUMNS);
		ret = 1;
	}
	if (y < MINROWS)
	{
		fprintf(stderr, "** Rows = %i, minimum = %i\n", y, MINROWS);
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

	if(nhexFile->iOff + nhexFile->iyPos * nhexScreen.iChunks*8 + nhexFile->ixPos +1 +1 <= nhexFile->iFileLength)
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
	unsigned int	iTmp;
	int 		result=0;	/* 0=nothing, 1=update, 2=redraw */

	if(nhexFile->iChangeCnt>0)
	{
		iTmp=nhexFile->iChangeAddr[nhexFile->iChangeCnt-1];
		result=nhexJumpPos(nhexFile, iTmp);
		nhexFile->iChangeCnt--;
	}

	return result;
}

/* jump to position on screen (from undo, goto, search) */
int nhexJumpPos(struct nhexBuff *nhexFile, unsigned int iNewPos)
{
	int		iRet;		/* 1=update, 2 = redraw */

	if(iNewPos >= nhexFile->iOff && iNewPos<=nhexFile->iOff + nhexScreen.iRows*nhexScreen.iChunks*8 - 1 && \
			nhexFile->iOff % (nhexScreen.iChunks*8) == 0)
	{
		/* new position is on the screen */
		nhexScreenDetReset(nhexFile);
		nhexFile->iyPos=(iNewPos - nhexFile->iOff) / (nhexScreen.iChunks*8);
		nhexFile->ixPos=(iNewPos - nhexFile->iOff) % (nhexScreen.iChunks*8);
		nhexFile->bHiLo=false;
		iRet=1;
	}
	else
	{
		/* need to move the offset, try to center */
		if(iNewPos < (nhexScreen.iRows/2) * (nhexScreen.iChunks*8))
			nhexFile->iOff=0;
		else
			nhexFile->iOff=(iNewPos / (nhexScreen.iChunks*8) - (nhexScreen.iRows/2)) * (nhexScreen.iChunks*8);
		nhexFile->iyPos=(iNewPos - nhexFile->iOff) / (nhexScreen.iChunks*8);
		nhexFile->ixPos=iNewPos % (nhexScreen.iChunks*8);
		iRet=2;
	}

	return iRet;
}

/* main entry point */
int main(int argc, char *argv[])
{
	int		i, opt;
	unsigned int	jval=0;
	int		x, y;
	char		*p;
	struct nhexBuff	nhexFile;
	int 		c;
	bool 		ready, scrUpdate, scrRedraw;
	int		iRes;					/* temp result of functions etc. */
	unsigned char	cTmp;					/* temp char for editing etc. */
	char		style;
	int		sMenuKeys[MAXMENUCATS * MAXMENUITEMS];
	int		nMenuKeys;

	/* verifiy and process command-line options */
	while((opt=getopt(argc, argv, "hj:v")) != -1)
	{
		switch(opt)
		{
			case 'h':
				printf("Usage: %s [options] [filename]\n\n", PACKAGE);
				printf("Options:\n");
				printf("  -h            Print this help text and exit\n");
				printf("  -j <address>  Jump to address in file\n");
				printf("  -v            Print the version number and exit\n");
				printf("\nReport bugs to <niels.horn@gmail.com>\n");
				exit(0);
			case 'j':
				if(optarg[strlen(optarg)-1] == 'h')
					sscanf(optarg, "%X", &jval);
				else
					jval=atoi(optarg);
				break;
			case 'v':
				printf("%s - Version %s\n", PACKAGE, VERSION);
				exit(0);
			default:
				fprintf(stderr, "** Usage: %s [-hv] | [-j] [filename]\n", argv[0]);
				exit(1);
		}
	}

	if(optind < argc)
	{
		/* filename given in command line */
		if(strlen(argv[optind]) > MAXFILENAME)
		{
			fprintf(stderr, "** Filename too long (max=%i)\n", MAXFILENAME);
			exit(1);
		}
		/* open file and start editing! */
		strcpy(nhexFile.sFileName, argv[optind]);
		nhexFileReset(&nhexFile);
		nhexFile.fp=nhexFileReadOpen(nhexFile.sFileName, &nhexFile.iFileLength);
		if (nhexFile.fp == NULL)
		{
			/* did not work :( Clean up & exit */
			fprintf(stderr, "** Unable to open '%s'\n", nhexFile.sFileName);
			exit(1);
		}
	}
	else
	{
		/* no filename given */
		strcpy(nhexFile.sFileName, "");
		nhexFileReset(&nhexFile);
		nhexFile.iFileLength=0;
	}

	if(jval && jval>=nhexFile.iFileLength)
	{
		fprintf(stderr, "** Address larger than filelength\n");
		exit(1);
	}

	/* check if we have a capable terminal */
	if(nhexSanityCheck())
		exit(1);
	
	nhexScreenSetup(&nhexScreen);
	nhexScreenHeader();
	if(jval)
	{
		/* goto initial position in file */
		iRes=nhexJumpPos(&nhexFile, jval);
	}
	nhexScreenShow(&nhexFile);
	nhexScreenDetails(&nhexFile);

	/* Main loop */
	noecho();
	cbreak();
	keypad(stdscr, true);
	ready=false;

	/* mount string with menu codes */
	/* TODO: move within while-loop if menu become dynamic */
	nMenuKeys=sizeof(nhexMenuItems)/sizeof(nhexMenuItems[0]);
	for(i=0; i<nMenuKeys; i++)
	{
		sMenuKeys[i]=0;
		p=nhexMenuItems[i].sKeyCode;
		if(p[0] == '^')
			sMenuKeys[i]=p[1]-64;
		else if(p[0] == 'F')
		{
			if(p[2] == '\0')
				sMenuKeys[i]=KEY_F(p[1]-48);
			else
				sMenuKeys[i]=KEY_F(10+p[2]-48);
		}
	}

	while(1)
	{
		/*
		 * This is a workaround for a bug that only affects Xfce Terminal
		 * We need to "reset" the screen, printing something at 0,0,
		 * so let's call the ScreenHeader routine.
		 * (without this, <tab> messes up some columns in Terminal)
		 */
		getyx(stdscr,y,x);
		nhexScreenHeader();
		move(y,x);

		/* catch a key and process it */
		c=getch();
		scrUpdate=false;
		scrRedraw=false;
		switch(c)
		{
			case KEY_F(11):
				/* catch any key */
				/*while(1)
				{
					c=getch();
					if(c == KEY_F(11))
						break;
					else
						mvprintw(0, 30, "[%i]", c);
				}
				scrRedraw=true;*/
				break;
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
				if(nhexFile.iOff + (nhexFile.iyPos+1) * nhexScreen.iChunks*8 + \
					nhexFile.ixPos +1 <= nhexFile.iFileLength)
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
					if(nhexFile.iOff + nhexFile.iyPos * nhexScreen.iChunks*8 + \
						nhexFile.ixPos > nhexFile.iFileLength - 1)
					{
						nhexFile.iyPos=(nhexFile.iFileLength-1 - nhexFile.iOff) / \
							(nhexScreen.iChunks*8);
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
				if(nhexFile.iOff + nhexFile.iyPos * nhexScreen.iChunks*8 + \
					nhexFile.ixPos > nhexFile.iFileLength)
					nhexFile.ixPos=(nhexFile.iFileLength-1) % (nhexScreen.iChunks*8);
				scrUpdate=true;
				nhexFile.bHiLo=false;
				break;
			case KEY_HOME:
			case HNKEY_HOME:
				nhexFile.iOff=0;
				nhexFile.ixPos=0;
				nhexFile.iyPos=0;
				scrRedraw=true;
				break;
			case KEY_END:
			case HNKEY_END:
				nhexFile.iOff=(nhexFile.iFileLength-1) / (nhexScreen.iChunks*8);
				if(nhexFile.iOff < nhexScreen.iRows+1)
					nhexFile.iOff=0;
				else
					nhexFile.iOff=(nhexFile.iOff - nhexScreen.iRows+1) * nhexScreen.iChunks*8;
				nhexFile.iyPos=(nhexFile.iFileLength-1 - nhexFile.iOff) / (nhexScreen.iChunks*8);
				nhexFile.ixPos=(nhexFile.iFileLength-1) % (nhexScreen.iChunks*8);
				scrRedraw=true;
				break;
			case HNKEY_TAB:
				/* tab - switch between hex & ascii */
				nhexFile.bPos=!nhexFile.bPos;
				nhexFile.bHiLo=false;
				scrUpdate=true;
				break;
			case KEY_F(12):
			case HNKEY_ESC:
				/* we accept F12 & Esc for menu */
				iRes=nhexMenu(nhexMenuItems, sizeof(nhexMenuItems)/sizeof(nhexMenuItems[0]));
				if(iRes != 0)
					iRes=nhexFunctions(nhexMenuItems[iRes].sFunction, &nhexFile, &nhexScreen);
				if(iRes == -1)
					ready=true;
				else
					scrRedraw=true;
				break;
			case KEY_RESIZE:
				/* recalculate iRows/iChunks/iCols, reset screen & jump to where we were */
				jval=nhexFile.iOff + nhexFile.iyPos*nhexScreen.iChunks*8 + nhexFile.ixPos;
				nhexScreenSetup(&nhexScreen);
				nhexJumpPos(&nhexFile, jval);
				scrRedraw=true;
				break;
			default:
				/* check menu keycodes */
				iRes=-1;
				for(i=0; i<nMenuKeys; i++)
				{
					if(sMenuKeys[i] == c)
					{
						iRes=nhexFunctions(nhexMenuItems[i].sFunction, &nhexFile, &nhexScreen);
						if(iRes == -1)
							ready=true;
						else
							scrRedraw=true;
						break;
					}
				}
				if(ready || scrRedraw || nhexFile.fp==NULL) break;
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
						nhexFile.iChangeAddr[nhexFile.iChangeCnt]= \
							nhexFile.iOff + nhexFile.iyPos * nhexScreen.iChunks*8 + \
							nhexFile.ixPos;
						nhexFile.cChangeByte[nhexFile.iChangeCnt]= \
							(nhexFileReadPos(&nhexFile, \
							nhexFile.iChangeAddr[nhexFile.iChangeCnt], &style) & 15) + cTmp;
						nhexFile.iChangeCnt++;
						nhexFile.bHiLo=true;
						scrUpdate=true;
					}
					else
					{
						/* change low bits of last change */
						nhexFile.cChangeByte[nhexFile.iChangeCnt-1]= \
							(nhexFile.cChangeByte[nhexFile.iChangeCnt-1] & 240) + cTmp;
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
						nhexFile.iChangeAddr[nhexFile.iChangeCnt]= \
							nhexFile.iOff + nhexFile.iyPos * nhexScreen.iChunks*8 + \
							nhexFile.ixPos;
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
		 * disabling if fp=null, etc.
		 */
		if(ready) break;
	}

	if(nhexFile.fp) fclose(nhexFile.fp);

	endwin();
	return 0;
}
