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
 * Module name:    word
 * Author name:    Zachary Smith
 * Create date:    01 Sep 00
 * Purpose:        Management of Word objects, which contain strings
 *                 as well as other Words.
 *----------------------------------------------------------------------
 * Changes:
 * 14 Oct 00, tuorfa@yahoo.com: fixed \fs bug (# is 2X the point size).
 * 14 Oct 00, tuorfa@yahoo.com: fixed table data printing.
 * 14 Oct 00, tuorfa@yahoo.com: protection against null entries in \info
 * 14 Oct 00, tuorfa@yahoo.com: fixed printing of <body> again
 * 14 Oct 00, tuorfa@yahoo.com: fixed closure of tables
 * 15 Oct 00, tuorfa@yahoo.com: fixed font attributes preceding <tr><td>
 * 15 Oct 00, tuorfa@yahoo.com: attributes now continue if >1 \cell in group
 * 15 Oct 00, tuorfa@yahoo.com: fixed font-size bug, lack of </head>
 *  7 Nov 00, tuorfa@yahoo.com: fixed \'## translatin bug
 *  8 Apr 01, tuorfa@yahoo.com: added check for out of memory after malloc
 * 21 Apr 01, tuorfa@yahoo.com: bug fixes regarding author, date
 * 21 Apr 01, tuorfa@yahoo.com: added paragraph alignment
 * 21 Apr 01, tuorfa@yahoo.com: fix for words getting lost after \par
 * 24 Jul 01, tuorfa@yahoo.com: moved conversion code to convert.c
 * 22 Sep 01, tuorfa@yahoo.com: moved word_dump to here from parse.c
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 * 29 Mar 05, daved@physiol.usyd.edu.au: changes requested by ZT Smith
 * 16 Dec 07, daved@physiol.usyd.edu.au: updated to GPL v3
 *--------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "defs.h"
#include "parse.h"
#include "malloc.h"
#include "main.h"
#include "error.h"
#include "word.h"
#include "hash.h"


/* For word_dump */
static int indent_level=0;


/*========================================================================
 * Name:	word_string
 * Purpose:	Obtains the string of a Word object. This involves accessing
 *			the Word hash.
 * Args:	Word*.
 * Returns:	String.
 *=======================================================================*/

const char *
word_string (Word *w) {
	CHECK_PARAM_NOT_NULL(w);
	return w->str;
}



/*========================================================================
 * Name:	word_new
 * Purpose:	Instantiates a new Word object.
 * Args:	String.
 * Returns:	Word*.
 *=======================================================================*/

Word * 
word_new (char *str) {
	Word * w;

	w = (Word *) my_malloc(sizeof(Word));
	if (!w)
		error_handler ("out of memory");
	memset ((void*) w, 0, sizeof(Word));
	if (!w) error_handler ("cannot allocate a Word");

	if (str)  w->str = hash_store (str);

	return w;
}

/*========================================================================
 * Name:	word_free
 * Purpose:	Deallocates a Word object. This is only called at the end of
 * 			main(), after everything is processed and output complete.
 * Args:	Word.
 * Returns:	None.
 *=======================================================================*/

void word_free (Word *w) {
	Word *prev;
	Word *w2;

	CHECK_PARAM_NOT_NULL(w);

	while (w) {
		w2 = w->child;
		if (w2)
			word_free(w2);

		prev = w;
		w = w->next;
		my_free((char*) prev);
	}
}





/*========================================================================
 * Name:	print_indentation
 * Purpose:	Prints padding for the word_dump routine.
 * Args:	Identation level.
 * Returns:	None.
 *=======================================================================*/

static void
print_indentation (int level)
{
	int i;

	if (level) {
		for (i=0;i<level;i+=2)
			printf (". ");
	} else {
		printf ("\n-----------------------------------------------------------------------\n\n");
	}
}




/*========================================================================
 * Name:	word_dump
 * Purpose:	Recursive diagnostic routine to print out a tree of words.
 * Args:	Word tree.
 * Returns:	None.
 *=======================================================================*/

void
word_dump (Word *w)
{
	const char *s;

	CHECK_PARAM_NOT_NULL(w);

	printf ("\n");
	indent_level += 2;
	print_indentation (indent_level);

	while (w) {
		s = word_string (w);
		if (s) { 
			printf ("\"%s\" ", s);
		} else { 
			if (w->child) {
				word_dump (w->child); 
				printf ("\n");
				print_indentation (indent_level);
			}
			else
				warning_handler ("Word object has no string and no children");
		}
		w = w->next;
	}

	indent_level -= 2;
}

/*========================================================================
 * Name:	optimize_word
 * Purpose:	Function tries to optimize Word.
 * Args:	Word to optimize.
 * Returns:	Optimized word.
 *=======================================================================*/
Word *
optimize_word(Word *w, int depth)
{
	const char *s, *s1;
	int i = 0, len;
	Collection *c = NULL;
	Tag tags_to_opt[] = OPT_ARRAY;
	Word *root = w, *w2 = 0;

        if (depth > MAX_GROUP_DEPTH) {
		/* Have to be reasonable at some point */
		warning_handler ("Max group depth reached");
		return w;
        }
	for (; w != NULL; w = w->next)
	{

		if ((s = word_string(w)))
		{
			for (i = 0; tags_to_opt[i].name[0] != '\0'; i++)
			{
				if (tags_to_opt[i].has_param)
				{
					len = strlen(tags_to_opt[i].name);
					if (!strncmp(tags_to_opt[i].name, s, len) && (isdigit(s[len]) || s[len] == '-'))
						break;
				}
				else
					if (!strcmp(tags_to_opt[i].name, s))
						break;					
			}

			if (tags_to_opt[i].name[0] != '\0')
			{
				s1 = get_from_collection(c, i);

				if (s != NULL && s1 != NULL && !strcmp(s1, s))
				{
					w2->next = w->next;
					my_free((char *)w);
					w = w2;
				}
				else
					c = add_to_collection(c, i, s);
			}
		}

		if (w->child != NULL)
			w->child = optimize_word(w->child, depth+1);

		w2 = w;
	}

	free_collection(c);

	return root;
}

