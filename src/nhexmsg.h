/*
 * nhexmsg.h - header for message routines for nHex-Ed
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

/* types of messages */
#define NHMSGINFO	256
#define NHMSGWARN	512
#define NHMSGERR	1024
#define NHMSGML		32768

/* "buttons" to include */
#define NHMSGOK		1
#define NHMSGYES	2
#define NHMSGNO		4
#define NHMSGCANCEL	8

#define MAXMSGLINES	20
#define MAXMSGWIDTH	80

int nhexMsg(int flags, char *msg);

