/*
 * nhexfind.c - find routines for nHex-Ed
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "nhexed.h"
#include "nhexfile.h"

int nhexFind(struct nhexBuff *nhexFile, char *sFind, long *lPos, bool *bHex)
{
	unsigned int	i, cTmp, iRet;
	int		iLen, iSub;
	bool		bFound=false;
	char		c, *cFind, style;

	/* find correct length of string */
	if(*bHex)
	{
		iLen=strlen(sFind)/2;
		cFind=malloc(iLen);
		for(i=0; i<iLen; i++)
		{
			sscanf(&sFind[i*2], "%2X", &cTmp);
			cFind[i]=(unsigned char)cTmp;
		}
	}
	else
	{
		iLen=strlen(sFind);
		cFind=sFind;
	}
	if(iLen == 0) return 1;

	/* start search */
	for(i=*lPos+1; i<nhexFile->iFileLength-iLen+1; i++)
	{
		bFound=true;
		for(iSub=0; iSub<iLen; iSub++)
		{
			c=nhexFileReadPos(nhexFile, i+iSub, &style);
			if(c != cFind[iSub])
			{
				bFound=false;
				break;
			}
		}
		if(bFound) break;
	}

	if(bFound)
	{
		iRet=1;
		*lPos=i;
	}
	else
	{
		if(*lPos > 0)
		{
			/* search from beginning */
			*lPos=-1;
			iRet=nhexFind(nhexFile, sFind, lPos, bHex);
		}
		else
		{
			iRet=0;
		}
	}

	if(*bHex) free(cFind);
	return iRet;
}
