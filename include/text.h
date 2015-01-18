/* smb
 * Copyright (c) 2010, Strippato
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <strippato@gmail.com>
 */

/* text.h */
#ifndef TEXT_H
#define TEXT_H

#define TEXT_MAX_ITEM(X)	(sizeof(X)/sizeof(X[0]))
	
#define TEXT_SOMEONEISCHEATING	(text_someoneischeating [rand() % (TEXT_MAX_ITEM(text_someoneischeating))	])
#define TEXT_LIFEUP			(text_lifeup            [rand() % (TEXT_MAX_ITEM(text_lifeup))			])
#define TEXT_AFK			(text_afk               [rand() % (TEXT_MAX_ITEM(text_afk))			])
#define TEXT_CANTPLACEHERE		(text_cantplacehere     [rand() % (TEXT_MAX_ITEM(text_cantplacehere))	])
#define TEXT_DEAD			(text_dead              [rand() % (TEXT_MAX_ITEM(text_dead))			])
#define TEXT_HURRYUP			(text_hurryup           [rand() % (TEXT_MAX_ITEM(text_hurryup))			])

extern char *text_someoneischeating	[5];
extern char *text_lifeup		[5];
extern char *text_afk			[5];
extern char *text_cantplacehere	[5];
extern char *text_dead			[5];
extern char *text_hurryup		[5];

#endif

