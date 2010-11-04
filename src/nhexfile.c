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
#include "nhexfile.h"

#define DEBUG		1

FILE* nhexFileReadOpen(char *pFileName, unsigned int *iFileLength)
{
	FILE *fp;

	fp = fopen(pFileName, "r");
	if(fp == NULL)
	{
		printf("** Cannot open file\n");
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
