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
#include <panel.h>

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
	FORM	*frmForm;
	WINDOW	*frmWin, *frmSubWin;
	PANEL	*frmPanel[2];
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

	/* set up label & input field */
	field[0]=new_field(1, strlen(pQuestion), 0, 0, 0, 0);
	field[1]=new_field(1, maxWidth, 1, 0, 0, 0);
	field[2]=NULL;
	set_field_buffer(field[0], 0, pQuestion);
	set_field_back(field[0], A_BOLD);
	field_opts_off(field[0], O_ACTIVE);
	set_field_buffer(field[1], 0, pAnswer);
	set_field_back(field[1], A_UNDERLINE);
	field_opts_off(field[1], O_AUTOSKIP + fOption);

	/* set up form */
	frmPanel[0]=new_panel(stdscr);
	frmForm=new_form(field);
	scale_form(frmForm, &iyForm, &ixForm);
	frmWin=newwin(iyForm+4, ixForm+4, (pheight-iyForm-4)/2, (pwidth-ixForm-4)/2);
	wattron(frmWin, A_REVERSE);
	wborder(frmWin, '|', '|', '-', '-', '+', '+', '+', '+');
	set_form_win(frmForm, frmWin);
	frmSubWin=derwin(frmWin, iyForm, ixForm, 2, 2);
	set_form_sub(frmForm, frmSubWin);
	frmPanel[1]=new_panel(frmWin);

	mvwprintw(frmWin, 0, 2, pTitle);
	post_form(frmForm);
	wrefresh(frmWin);
	wmove(frmWin, 3, 2);
	form_driver(frmForm, REQ_BEG_FIELD);
	update_panels();
	doupdate();

	/* handle input */
	keypad(frmWin, TRUE);
	ready=false;
	while(1)
	{
		ch=wgetch(frmWin);
		switch(ch)
		{
			case KEY_ENTER:
			case HNKEY_ENTER:
				form_driver(frmForm, REQ_NEXT_FIELD);
				iRet=1;
				ready=true;
				break;
			case HNKEY_ESC:
				ready=true;
				break;
			case HNKEY_BS:
			case HNKEY_ERASE:
				/* two common "back-space" keys */
				form_driver(frmForm, REQ_DEL_PREV);
				break;
			case HNKEY_DEL:
				form_driver(frmForm, REQ_DEL_CHAR);
				break;
			case HNKEY_INS:
				form_driver(frmForm, REQ_INS_CHAR);
				break;
			case KEY_LEFT:
				form_driver(frmForm, REQ_PREV_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(frmForm, REQ_NEXT_CHAR);
				break;
			case KEY_HOME:
				form_driver(frmForm, REQ_BEG_FIELD);
				break;
			case KEY_END:
				form_driver(frmForm, REQ_END_FIELD);
				break;
			default:
				if(ch >= 32 && ch < 127)
					form_driver(frmForm, ch);
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

	unpost_form(frmForm);
	free_form(frmForm);
	free_field(field[0]);
	free_field(field[1]);
	del_panel(frmPanel[1]);
	delwin(frmWin);
	del_panel(frmPanel[0]);
	update_panels();
	doupdate();

	return iRet;
}
