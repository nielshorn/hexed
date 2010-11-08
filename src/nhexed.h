/*
 * nhexed.h - general header for nHex-Ed
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

#define MINCOLUMNS	45		/* 45=1 chunk, 34 for each extra chunk */
#define MINROWS		9		/* minimal number of rows on screen */
#define MAXLENGTH	4294967295	/* max file length 4GB for uint */
#define MAXFILENAME	255		/* max length of filename */
#define MAXCHANGE	1000		/* maximum number of changes allowed */

struct	nhexBuff {
	FILE		*fp;			/* FILE pointer */
	char		sFileName[MAXFILENAME];	/* complete path+filename */
	unsigned int	iFileLength;		/* length of file */
	unsigned int	iOff;			/* position in file of 1st pos on screen */
	int		ixPos, iyPos;		/* current x/y pos on screen */
	bool		bPos;			/* position hex(=false)/asc(=true) */
	bool		bHiLo;			/* hi(=false)/lo(=true) nibble */
	unsigned int	iChangeAddr[MAXCHANGE];	/* array of addresses that have changed */
	unsigned char	cChangeByte[MAXCHANGE];	/* array of changed values */
	int		iChangeCnt;		/* number of changes entered */
};

