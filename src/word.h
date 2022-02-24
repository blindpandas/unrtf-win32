#ifndef _WORD_H_INCLUDED_
#define _WORD_H_INCLUDED_

/*=============================================================================
   GNU UnRTF, a command-line program to convert RTF documents to other formats.
   Copyright (C) 2000,2001,2004 by Zachary Smith

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

   The maintainer is reachable by electronic mail at daved@physiol.usyd.edu.au
=============================================================================*/


/*----------------------------------------------------------------------
 * Module name:    word.h
 * Author name:    Zachary Smith
 * Create date:    1 Sept 2000
 * Purpose:        Definitions for Word class.
 *----------------------------------------------------------------------
 * Changes:
 * 29 Mar 05, daved@physiol.usyd.edu.au: changes requested by ZT Smith
 * 16 Dec 07, daved@physiol.usyd.edu.au: updated to GPL v3
 * 09 Nov 08, arkadiusz.firus@gmail.com: add word_optimise
 * 25 Sep 11, jf@dockes.org: reposition top #ifndef
 *--------------------------------------------------------------------*/

typedef struct _w {
	const char *str;
	struct _w * next;
	struct _w * child;
} Word;

extern Word* word_new (char*);
extern void word_free (Word*);
extern Word* word_read (FILE*);
extern const char* word_string (Word*);
extern void word_dump (Word*);
extern void word_print_html (Word*);

#ifndef MAX_GROUP_DEPTH
#define MAX_GROUP_DEPTH 1000
#endif
 
extern Word* optimize_word(Word*, int depth);

typedef struct _t {
	char *name;
	int has_param;
} Tag;

#define OPT_ARRAY \
{\
	{"\\fs", 1},\
	{"\\f", 1},\
	{"", 0}\
}

#endif /* _WORD_H_INCLUDED_ */
