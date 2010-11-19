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

#include <stdlib.h>
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

/* convert to/from hex */
void nhexFrmHexConv(bool *bHex, FIELD *field, char cType)
{
	char		sIn[255], sOut[255]="";
	int		i, iChar;
	unsigned int	lAddr;

	strcpy(sIn, field_buffer(field, 0));
	nhexFrmTrim(sIn);

	switch(cType)
	{
		case 's':
			if(*bHex)
			{
				for(i=0; i<strlen(sIn)/2; i++)
				{
					sscanf(&sIn[i*2], "%2X", &iChar);
					if(iChar <32 || iChar>124) iChar='.';
					sOut[i]=iChar;
				}
				sOut[i]='\0';
			}
			else
			{
				for(i=0; i<strlen(sIn); i++)
					sprintf(sOut, "%s%02X", sOut, sIn[i]);
			}
			break;
		case 'a':
			if(*bHex)
			{
				sscanf(sIn, "%X", &lAddr);
				sprintf(sOut, "%u", lAddr);
			}
			else
			{
				lAddr=atoi(sIn);
				sprintf(sOut, "%X", lAddr);
			}
			break;
	}
	set_field_buffer(field, 0, sOut);
	*bHex=!*bHex;
}

/* generic input form */
int nhexFrmInput(char *pTitle, char *pQuestion, char *pAnswer, int length, char cType, bool *bHex)
{
	FIELD	*field[7];		/* question + answer + 2-way switch/label + NULL-pointer */
	FORM	*frmForm;
	WINDOW	*frmWin, *frmSubWin;
	PANEL	*frmPanel[2];
	int	ixForm, iyForm;
	int	pheight, pwidth;
	int	i, ch;
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
	set_field_buffer(field[0], 0, pQuestion);
	set_field_back(field[0], A_BOLD);
	field_opts_off(field[0], O_ACTIVE);
	field[1]=new_field(1, maxWidth, 1, 0, 0, 0);
	set_field_buffer(field[1], 0, pAnswer);
	set_field_back(field[1], A_UNDERLINE);
	field_opts_off(field[1], O_AUTOSKIP + fOption);
	switch(cType)
	{
		case 't':
			/* text */
			field[2]=NULL;
			break;
		case 's':
			/* search: hex / ascii */
			field[2]=new_field(1, 1, 2, 0, 0, 0);
			set_field_back(field[2], A_UNDERLINE);
			field_opts_off(field[2], O_AUTOSKIP);
			field[3]=new_field(1, 3, 2, 2, 0, 0);
			set_field_buffer(field[3], 0, "HEX");
			set_field_back(field[3], A_BOLD);
			field_opts_off(field[3], O_ACTIVE);
			field[4]=new_field(1, 1, 2, 6, 0, 0);
			set_field_back(field[4], A_UNDERLINE);
			field_opts_off(field[4], O_AUTOSKIP);
			field[5]=new_field(1, 5, 2, 8, 0, 0);
			set_field_buffer(field[5], 0, "ASCII");
			set_field_back(field[5], A_BOLD);
			field_opts_off(field[5], O_ACTIVE);
			field[6]=NULL;
			//if(pAnswer[strlen(pAnswer)-1] == 'h')
			if(*bHex)
			{
				set_field_buffer(field[2], 0, "X");
				set_field_buffer(field[4], 0, " ");
				//*bHex=true;
			}
			else
			{
				set_field_buffer(field[2], 0, " ");
				set_field_buffer(field[4], 0, "X");
				//*bHex=false;
			}
			break;
		case 'a':
			/* address: decimal or hex */
			field[2]=new_field(1, 1, 2, 0, 0, 0);
			set_field_back(field[2], A_UNDERLINE);
			field_opts_off(field[2], O_AUTOSKIP);
			field[3]=new_field(1, 3, 2, 2, 0, 0);
			set_field_buffer(field[3], 0, "HEX");
			set_field_back(field[3], A_BOLD);
			field_opts_off(field[3], O_ACTIVE);
			field[4]=new_field(1, 1, 2, 6, 0, 0);
			set_field_back(field[4], A_UNDERLINE);
			field_opts_off(field[4], O_AUTOSKIP);
			field[5]=new_field(1, 7, 2, 8, 0, 0);
			set_field_buffer(field[5], 0, "DECIMAL");
			set_field_back(field[5], A_BOLD);
			field_opts_off(field[5], O_ACTIVE);
			field[6]=NULL;
			//if(pAnswer[strlen(pAnswer)-1] == 'h')
			if(*bHex)
			{
				set_field_buffer(field[2], 0, "X");
				set_field_buffer(field[4], 0, " ");
				//*bHex=true;
			}
			else
			{
				set_field_buffer(field[2], 0, " ");
				set_field_buffer(field[4], 0, "X");
				//*bHex=false;
			}
			break;
		default:
			/* invalid cType */
			return iRet;
	}


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
			case HNKEY_HOME:
				form_driver(frmForm, REQ_BEG_FIELD);
				break;
			case KEY_END:
			case HNKEY_END:
				form_driver(frmForm, REQ_END_FIELD);
				break;
			case HNKEY_TAB:
				form_driver(frmForm, REQ_NEXT_FIELD);
				break;
			case HNKEY_STAB:
				form_driver(frmForm, REQ_PREV_FIELD);
				break;
			default:
				i=field_index(current_field(frmForm));
				if(i == 2 || i == 4)
				{
					/* hex/ascii/numeric fields */
					if(ch == 32 || (ch & 95) == 'X')
					{
						set_field_buffer(field[i], 0, "X");
						set_field_buffer(field[6-i], 0, " ");
						if(i == 2)
						{
							if(!*bHex) nhexFrmHexConv(bHex, field[1], cType);
						}
						else
						{
							if(*bHex) nhexFrmHexConv(bHex, field[1], cType);
						}
					}
				}
				else
				{
					if(*bHex && ((ch >= '0' && ch <= '9') || (ch >='A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')))
						form_driver(frmForm, ch);
					else if(!*bHex && cType == 'a' && ch >='0' && ch <= '9')
						form_driver(frmForm, ch);
					else if(!*bHex && (cType == 's' || cType == 't') && ch >= 32 && ch <= 126)
						form_driver(frmForm, ch);
				}
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
