/*
 * nhexfunc.c - functions for nHex-Ed
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "nhexed.h"
#include "nhexmsg.h"
#include "nhexfile.h"
#include "nhexforms.h"
#include "nhexscreen.h"

int nhexFunctions(int function, struct nhexBuff *nhexFile, struct Screen *nhexScreen)
{
	int		iRes;
	char		sMsg[MAXMSGLINES * MAXMSGWIDTH];
	char		newFile[MAXFILENAME]="";
	char		newPos[11];
	unsigned int	iPos;
	char		*p;
	int		iReturn=0;
	bool		exit=false;

	switch(function)
	{
		case 101:
			/* file - open */
			if(nhexFile->iChangeCnt > 0)
			{
				sprintf(sMsg, "File has %i changes. Save current file first?", nhexFile->iChangeCnt);
				iRes=nhexMsg(NHMSGWARN + NHMSGCANCEL + NHMSGNO + NHMSGYES, sMsg);
				switch(iRes)
				{
					case NHMSGCANCEL:
						/* do nothing, just return */
						break;
					case NHMSGYES:
						/* save file */
						iRes=nhexFileSave(nhexFile, "");
						if(iRes != 0) break;
					case NHMSGNO:
						exit=true;
						break;
				}
			}
			else
				exit=true;
			if(exit)
			{
				if(nhexFile->fp)
				{
					fclose(nhexFile->fp);
					nhexFileReset(nhexFile);
					nhexFile->fp=NULL;
				}
				iRes=nhexFileGetName(newFile, 0);
				if(iRes)
				{
					strcpy(nhexFile->sFileName,newFile);
					nhexFile->fp=nhexFileReadOpen(nhexFile->sFileName, &nhexFile->iFileLength);
				}
				if(nhexFile->fp == NULL) nhexFile->iFileLength=0;
				exit=false;
			}
			break;
		case 102:
			/* file - save */
			if(nhexFile->fp)
			{
				iRes=nhexFileSave(nhexFile, "");
				if(iRes != 0)
				{
					nhexMsg(NHMSGERR + NHMSGOK, "Error writing file.");
				}
			}
			break;
		case 103:
			/* file - save as... */
			if(nhexFile->fp)
			{
				p=strrchr(nhexFile->sFileName,'/');
				if(p)
					p++;
				else
					p=nhexFile->sFileName;
				strcpy(newFile,p);
				iRes=nhexFileGetName(newFile, 1);	/* 1=get new name for existing file */
				if(iRes)
					iRes=nhexFileSave(nhexFile, newFile);
			}
			break;
		case 104:
			/* file - exit */
			if(nhexFile->iChangeCnt > 0)
			{
				/* unsaved changes, ask what to do */
				sprintf(sMsg, "File has %i changes. Save File?", nhexFile->iChangeCnt);
				iRes=nhexMsg(NHMSGWARN + NHMSGCANCEL + NHMSGNO + NHMSGYES, sMsg);
				switch(iRes)
				{
					case NHMSGCANCEL:
						/* do nothing, just return */
						break;
					case NHMSGYES:
						/* save file */
						iRes=nhexFileSave(nhexFile, "");
						if(iRes != 0) break;
					case NHMSGNO:
						exit=true;
						break;
				}
			}
			else
				exit=true;
			break;
		case 202:
			/* edit - undo all */
			if(nhexFile->iChangeCnt > 0)
			{
				sprintf(sMsg, "Really discard %i changes?", nhexFile->iChangeCnt);
				iRes=nhexMsg(NHMSGWARN + NHMSGNO + NHMSGYES, sMsg);
				if(iRes == NHMSGYES) nhexFile->iChangeCnt=0;
			}
			break;
		case 303:
			/* search - goto */
			if(nhexFile->fp)
			{
				iPos=nhexFile->iOff + nhexFile->iyPos*nhexScreen->iChunks*8 + nhexFile->ixPos;
				if(iPos == 0)
					strcpy(newPos,"");
				else
					sprintf(newPos, "%i", iPos);
				iRes=nhexFrmInput("Goto...", "Address (end with 'h' for hex):", newPos, 10);
				if(iRes)
				{
					if(newPos[strlen(newPos)-1] == 'h')
						iRes=sscanf(newPos, "%X", &iPos);
					else
						iRes=sscanf(newPos, "%i", &iPos);
					if(iPos < nhexFile->iFileLength)
					{
						nhexFile->iOff=(iPos/(nhexScreen->iChunks*8)) * (nhexScreen->iChunks*8);
						nhexFile->ixPos=iPos-nhexFile->iOff;
						nhexFile->iyPos=0;
					}
					else
					{
						nhexMsg(NHMSGERR + NHMSGOK, "Address out of range");
					}
				}
			}
			break;
		case 401:
			/* help - help */
			sprintf(sMsg, "Basic Help\n==========\n");
			sprintf(sMsg, "%sLeft/Right/Up/Down       : Move Left/Right/Up/Down\n", sMsg);
			sprintf(sMsg, "%sShift-Left / Shift-Right : Beginning / End of line\n", sMsg);
			sprintf(sMsg, "%sPgUp / PgDown            : Move Up/Down one page\n", sMsg);
			sprintf(sMsg, "%sHome/End                 : Beginning / End of file\n", sMsg);
			sprintf(sMsg, "%sTab                      : Alternate between Hex/ASCII\n", sMsg);
			sprintf(sMsg, "%s^X                       : Undo last change\n", sMsg);
			sprintf(sMsg, "%sEsc / F1 / F12           : Goto Menu (Esc to leave)\n", sMsg);
			nhexMsg(NHMSGML + NHMSGINFO + NHMSGOK, sMsg);
			break;
		case 402:
			/* help - about */
			sprintf(sMsg, "%s %s\nOpen Source Hex Editor\n \n", PACKAGE, VERSION);
			sprintf(sMsg, "%sby Niels Horn\nniels.horn@gmail.com",sMsg);
			nhexMsg(NHMSGML + NHMSGINFO + NHMSGOK, sMsg);
			break;
	}
	if(exit) iReturn=-1;
	return iReturn;
}
