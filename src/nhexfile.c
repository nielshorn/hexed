/*
 * nhexfile.c - file routines for nHex-Ed
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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "nhexed.h"
#include "nhexfile.h"
#include "nhexforms.h"
#include "nhexmsg.h"

/* Get new filename for "save as..." (flags=1) or for "open..." (flags=0) */
int nhexFileGetName(char *newFile, int flags)
{
	int	iRet;

	if(flags == 1)
		iRet=nhexFrmInput("Save As...", "Enter filename:", newFile, MAXFILENAME-1);
	else
		iRet=nhexFrmInput("Open File...", "Enter filename:", newFile, MAXFILENAME-1);

	return iRet;
}

/* Open file, return iFileLength & *fp */
FILE* nhexFileReadOpen(char *pFileName, unsigned int *iFileLength)
{
	FILE *fp;

	fp=fopen(pFileName, "r+b");
	if(fp == NULL)
	{
		nhexMsg(NHMSGERR + NHMSGOK, "Cannot open file");
		return NULL;
	}
	
	fseek(fp, 0, SEEK_END);
	*iFileLength = ftell(fp);

	if(*iFileLength > MAXLENGTH)
	{
		nhexMsg(NHMSGERR + NHMSGOK, "File too large");
		return NULL;		/* file too large */
	}

	return fp;
}

/* reset offset, position, etc. */
void nhexFileReset(struct nhexBuff *nhexFile)
{
	nhexFile->iOff=0;				/* initial position */
	nhexFile->ixPos=0;
	nhexFile->iyPos=0;
	nhexFile->iChangeCnt=0;
	nhexFile->bPos=false;
	nhexFile->bHiLo=false;
}

/* Save file (save as pFileName if given) */
int nhexFileSave(struct nhexBuff *nhexFile, char *pFileName)
{
	FILE		*fp;
	int		ch;
	char		path[MAXFILENAME]="";
	char		*p;
	unsigned int	i;
	int		iRet=0;

	if(strlen(pFileName) != 0)
	{
		/* filename given, construct path + file */
		if(pFileName[0] != '/')
		{
			/* not an absolute path, so get path from original name */
			p=strrchr(nhexFile->sFileName, '/');
			if(p)
				strncpy(path, nhexFile->sFileName, p-&nhexFile->sFileName[0]+1);
			strncat(path, pFileName, MAXFILENAME);
			strcpy(pFileName, path);
		}
		/* first check: does it exist? */
		fp=fopen(pFileName, "r");
		if(fp != NULL)
		{
			fclose(fp);
			iRet=nhexMsg(NHMSGWARN + NHMSGCANCEL + NHMSGYES, "File exists. Overwrite?");
			if(iRet != NHMSGYES) return 1;
		}
		/* copy file to new */
		fseek(nhexFile->fp, 0, SEEK_SET);
		fp=fopen(pFileName, "wb");
		for(i=0; i<nhexFile->iFileLength; i++)
		{
			ch=getc(nhexFile->fp);
			iRet=putc(ch, fp);
			if(iRet == EOF)
			{
				fclose(fp);
				iRet=nhexMsg(NHMSGERR + NHMSGOK, "Error writing file.");
				return 1;
			}
		}
		/* close both files */
		fclose(nhexFile->fp);
		fclose(fp);
		/* reopen new file r/w  + swap names & fp */
		fp=fopen(pFileName, "r+b");
		nhexFile->fp=fp;
		strcpy(nhexFile->sFileName, pFileName);
		/* now apply the changes... */
	}

	/* loop through changes in order */
	for(i=0; i<nhexFile->iChangeCnt; i++)
	{
		if(fseek(nhexFile->fp, (long)nhexFile->iChangeAddr[i], SEEK_SET))
		{
			nhexMsg(NHMSGERR + NHMSGOK, "Seek error while saving file");
			iRet=1;
			break;
		}
		else
		{
			putc(nhexFile->cChangeByte[i], nhexFile->fp);
		}
	}

	if(iRet == 0) nhexFile->iChangeCnt=0;

	return iRet;
}

/* Return byte from file (or changes) */
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
			nhexMsg(NHMSGERR + NHMSGOK,"Seek error");
			exit(1);
		}
		c=getc(nhexFile->fp);
	}

	if(bChange) *style='c';
	
	return c;
}

