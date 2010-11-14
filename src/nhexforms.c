/*
 * nhexforms.c - forms routines for nHex-Ed
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

#include <string.h>

#include <ncurses.h>
#include <form.h>

#include "nhexed.h"

/* simple trim function... */
void nhexFrmTrim(char *sIn)
{
	int	i;

	for(i=strlen(sIn)-1; i>=0 && strchr(&sIn[i], ' ') != NULL; i--)
		sIn[i]='\0';
}

/* single line input form */
int nhexFrmInput(char *pTitle, char *pQuestion, char *pAnswer, int length)
{
	FIELD	*field[3];		/* question + answer + NULL-pointer */
	FORM	*fileForm;
	WINDOW	*fileWin;
	int	ixForm, iyForm;
	int	pheight, pwidth;
	int	ch;
	int	iRet=0;
	bool	ready;
	int	maxWidth;
	int	fOption;

	/* get max height, width */
	getmaxyx(stdscr, pheight, pwidth);
	maxWidth=pwidth-4;
	if(maxWidth > length)
	{
		maxWidth=length;
		fOption=0;
	}
	else
	{
		fOption=O_STATIC;
	}

	/*
	 * TODO: take out the form handler and put it in
	 * a separate routine
	 */

	/* set up form */
	field[0]=new_field(1, strlen(pQuestion), 0, 0, 0, 0);
	field[1]=new_field(1, maxWidth, 1, 0, 0, 0);
	field[2]=NULL;
	set_field_buffer(field[0], 0, pQuestion);
	field_opts_off(field[0], O_ACTIVE);
	set_field_buffer(field[1], 0, pAnswer);
	set_field_back(field[1], A_REVERSE);
	field_opts_off(field[1], O_AUTOSKIP + fOption);

	fileForm=new_form(field);
	scale_form(fileForm, &iyForm, &ixForm);
	fileWin=newwin(iyForm+4, ixForm+4, (pheight-iyForm-4)/2, (pwidth-ixForm-4)/2);
	keypad(fileWin, TRUE);
	set_form_win(fileForm, fileWin);
	set_form_sub(fileForm, derwin(fileWin, iyForm, ixForm, 2, 2));
	box(fileWin, 0, 0);
	mvwprintw(fileWin, 0, 2, pTitle);
	post_form(fileForm);
	wrefresh(fileWin);
	wmove(fileWin, 3, 2);
	form_driver(fileForm, REQ_END_FIELD);
	refresh();

	ready=false;
	while(1)
	{
		ch=wgetch(fileWin);
		switch(ch)
		{
			case KEY_ENTER:
			case HNKEY_ENTER:
				form_driver(fileForm, REQ_NEXT_FIELD);
				iRet=1;
				ready=true;
				break;
			case HNKEY_ESC:
				ready=true;
				break;
			case HNKEY_BS:
			case HNKEY_ERASE:
				/* two common "back-space" keys */
				form_driver(fileForm, REQ_DEL_PREV);
				break;
			case KEY_LEFT:
				form_driver(fileForm, REQ_PREV_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(fileForm, REQ_NEXT_CHAR);
				break;
			default:
				if(ch >= 32 && ch < 127)
					form_driver(fileForm, ch);
				break;
		}
		if(ready) break;
	}

	if(iRet)
	{
		strncpy(pAnswer, field_buffer(field[1], 0), length);
		pAnswer[length]='\0';
		nhexFrmTrim(pAnswer);
	}

	unpost_form(fileForm);
	free_form(fileForm);
	free_field(field[0]);
	delwin(fileWin);
	refresh();

	return iRet;
}
