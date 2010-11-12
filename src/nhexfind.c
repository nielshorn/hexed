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
#include <string.h>
#include <stdbool.h>

#include "nhexed.h"
#include "nhexfile.h"

int nhexFind(struct nhexBuff *nhexFile, char *sFind, long *lPos)
{
	unsigned int	i, iRet;
	int		iLen, iSub;
	bool		bFound;
	char		c, style;

	iLen=strlen(sFind);

	for(i=*lPos+1; i<nhexFile->iFileLength-iLen; i++)
	{
		bFound=true;
		for(iSub=0; iSub<iLen; iSub++)
		{
			c=nhexFileReadPos(nhexFile, i+iSub, &style);
			if(c != sFind[iSub])
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
			iRet=nhexFind(nhexFile, sFind, lPos);
		}
		else
			iRet=0;
	}

	return iRet;
}
