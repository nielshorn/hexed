/*
 * nhexfile.c - file routines for nHex-Ed
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "nhexed.h"
#include "nhexfile.h"
#include "nhexmsg.h"

/* Open file, return iFileLength & *fp */
FILE* nhexFileReadOpen(char *pFileName, unsigned int *iFileLength)
{
	FILE *fp;

	fp = fopen(pFileName, "r+b");
	if(fp == NULL)
	{
		//nhexMsg(NHMSGERR + NHMSGOK, "Cannot open file");
		nhexMsg(257, "Cannot open file");
		return NULL;
	}
	
	fseek(fp, 0, SEEK_END);
	*iFileLength = ftell(fp);

	if(*iFileLength > MAXLENGTH)
	{
		printf("** File too large\n");
		return NULL;		/* file too large */
	}

	return fp;
}

/* Return byte from file (or changes) */
//char nhexFileReadPos(FILE *fp, unsigned int iChangeAddr[], unsigned char cChangeByte[], int iChangeCnt, unsigned int iAddr, char *style)
char nhexFileReadPos(struct nhexBuff *nhexFile, unsigned int iAddr, char *style)
{
	int		i;
	unsigned char	c;
	bool		bChange=false;

	*style='n';

	/* search in list of changes first */
	for(i=nhexFile->iChangeCnt-1; i>=0; i--)
	{
		if(iAddr == nhexFile->iChangeAddr[i])
		{
			c=nhexFile->cChangeByte[i];
			bChange=true;
			break;
		}
	}

	if(!bChange)
	{
		if(fseek(nhexFile->fp, (long)iAddr, SEEK_SET))
		{
			//nhexMsg(NHMSGERR + NHMSGOK,"Seek error");
			nhexMsg(257, "Seek error");
			exit(1);
		}
		c=getc(nhexFile->fp);
	}

	if(bChange) *style='c';
	
	return c;
}

