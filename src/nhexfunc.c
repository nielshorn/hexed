/*
 * nhexfunc.c - functions for nHex-Ed
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

//#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "nhexed.h"
#include "nhexmsg.h"
#include "nhexfile.h"

int nhexFunctions(int function, struct nhexBuff *nhexFile)
{
	int	iRes;
	char	sMsg[MAXMSGLINES * MAXMSGWIDTH];
	int	iReturn=0;
	bool	exit=false;

	switch(function)
	{
		case 102:
			/* file - save */
			if(nhexFile->iChangeCnt > 0)
			{
				iRes=nhexFileSave(nhexFile, "");
				if(iRes != 0)
				{
					nhexMsg(NHMSGERR + NHMSGOK, "Error writing file.");
				}
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
