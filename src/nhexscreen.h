/*
 * nhexscreen.h - header for screen routines for nHex-Ed
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

struct Screen {
	int		iRows;		/* # of rows available for output */
	int		iCols;		/* # of coluns available for output */
	int		iChunks;	/* # of 8-byte chunks per line */
};

void nhexScreenDetails(struct nhexBuff *nhexFile);
void nhexScreenDetReset(struct nhexBuff *nhexFile);
void nhexScreenHeader(void);
void nhexScreenSetup(struct Screen *nhexScreen);
void nhexScreenShow(struct nhexBuff *nhexFile);

