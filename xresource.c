/*----------------------------------------------------------------------------*/
/*  xresource.c                                                               */
/*----------------------------------------------------------------------------*/
/*  copyright         : (C) 2002 by Dao Hai Lam                               */
/*                      VISC Software & Security Consultant Company           */
/*                      Hall 3, Quang Trung Software City                     */
/*                      Tan Chanh Hiep Ward, District 12,                     */
/*                      Ho Chi Minh city, VIETNAM                             */
/*  website           : http://www.visc-network.com                           */
/*  email             : lam@visc-network.com                                  */
/*  last modify       : Thu, 18 Apr 2002 22:00:33 +0700                       */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*   This program is free software; you can redistribute it and/or modify     */
/*   it under the terms of the GNU General Public License as published by     */
/*   the Free Software Foundation; either version 2 of the License, or        */
/*   (at your option) any later version.                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include "data.h"
#include <locale.h>
/*----------------------------------------------------------------------------*/
void VKLocaleInit()
{
	char *lang = getenv("LANG");
	lang = lang ? strdup(lang) : strdup("en_US");
	if( !strstr(lang, ".UTF-8") ) {
		char *p = strchr(lang, '.');
		if( p ) *p = 0;
		p = (char *)malloc(strlen(lang)+8);
		sprintf(p, "%s%s", lang, ".UTF-8");
		setenv("LANG", p, 1);
		free(lang);
		lang = p;
	}
	if( !setlocale(LC_ALL, "") )
		error("Error: Cannot set locale %s!\n", lang);
	free(lang);
}
/*----------------------------------------------------------------------------*/
#ifndef USE_XFT
char *VKXftFont2XFont(char *name)
{
	static char fn[128], *sep = ":";

	char s[128], *p, *face, *style = 0;
	int bold = 0, mode = 'r';

	strcpy(s, name);
	face = strtok(s, sep);
	p = strtok(NULL, sep);
	while( p ) {
		if( !strncasecmp(p, "style=", 6) )
			style = p+6;
		p = strtok(NULL, sep);
	}
	if( style ) {
		p = strtok(style, " ");
		while( p ) {
			if( !strcasecmp(p, "bold") )
				bold = 1;
			else
			if( !strcasecmp(p, "italic") ||
				!strcasecmp(p, "oblique") )
				mode = 'i';
			p = strtok(NULL, " ");
		}
	}

	snprintf(fn, sizeof(fn), "*-%s-%s-%c-normal--*",
								face, bold ? "bold" : "medium",	mode);
	return fn;
}
/*----------------------------------------------------------------------------*/
void VKUseDefaultFont()
{
	if( vk_font_name ) free(vk_font_name);
	vk_font_name = vk_interface==VK_ENGLISH ?
		strdup(screen_width<=800 ? VK_FONT_12 : VK_FONT_14) :
		strdup(VK_VN_FONT);
}
/*----------------------------------------------------------------------------*/
void VKFontSetInit()
{
	int missing_count;
	char **missing_list;
	char *def_string;
	char *backup = NULL;

	vk_fontset = XCreateFontSet(display, vk_font_name, &missing_list,
								&missing_count, &def_string);
	if( !vk_fontset ) {
		char *p = strchr(vk_font_name, ':');
		if( p ) {
			TRACE("It seems XFT font name,"
				" strip extended info and try to reload fontset.\n");
			backup = vk_font_name;
			vk_font_name = strdup(VKXftFont2XFont(vk_font_name));
		}
		else {
			TRACE("Unknown fontname style.\n"
				"Try to load fontset with default font.\n");
			VKUseDefaultFont();
		}
		vk_fontset = XCreateFontSet(display, vk_font_name, &missing_list,
									&missing_count, &def_string);
	}
	if( !vk_fontset ) {
		if( backup ) {
			VKUseDefaultFont();
			vk_fontset = XCreateFontSet(display, vk_font_name, &missing_list,
									&missing_count, &def_string);
		}
		if( !vk_fontset ) error("Error: cannot load font set.\n");
	}
	if( backup ) {
		free(vk_font_name);
		vk_font_name = backup;
	}
}
#endif
/*----------------------------------------------------------------------------*/
void VKLoadPalette()
{
#ifdef USE_XFT
	XftColorAllocName(display, visual, colormap, "red", &clRed );
	XftColorAllocName(display, visual, colormap, "yellow", &clYellow );
	XftColorAllocName(display, visual, colormap, "white", &clWhite );
	XftColorAllocName(display, visual, colormap, "black", &clBlack );
	XftColorAllocName(display, visual, colormap, "blue", &clBlue );
	XftColorAllocName(display, visual, colormap, "gray", &clGray );
	XftColorAllocName(display, visual, colormap, "SteelBlue", &clMenuBar );
#else
	XColor color;

	XParseColor(display, colormap, "red", &color);
	XAllocColor(display, colormap, &color);
	clRed = color.pixel;
	XParseColor(display, colormap, "yellow", &color);
	XAllocColor(display, colormap, &color);
	clYellow = color.pixel;
	XParseColor(display, colormap, "white", &color);
	XAllocColor(display, colormap, &color);
	clWhite = color.pixel;
	XParseColor(display, colormap, "black", &color);
	XAllocColor(display, colormap, &color);
	clBlack = color.pixel;
	XParseColor(display, colormap, "blue", &color);
	XAllocColor(display, colormap, &color);
	clBlue = color.pixel;
	XParseColor(display, colormap, "gray", &color);
	XAllocColor(display, colormap, &color);
	clGray = color.pixel;
	XParseColor(display, colormap, "SteelBlue", &color);
	XAllocColor(display, colormap, &color);
	clMenuBar = color.pixel;
#endif
}
/*----------------------------------------------------------------------------*/
void VKLoadXResource()
{
	VKLocaleInit();
	VKLoadPalette();

	if( !vk_font_name )
		vk_font_name = strdup(screen_width<=800 ? VK_FONT_12 : VK_FONT_14);

	if( !vk_font_name ) error("Error: not enough memory!");
#ifdef USE_XFT
	vk_font = XftFontOpenName(display, 0, vk_font_name);
	if( !vk_font )
		vk_font = XftFontOpenXlfd(display, 0, vk_font_name);
#else
	vk_font = XLoadQueryFont(display, vk_font_name);
	if( !vk_font )
		vk_font = XLoadQueryFont(display, screen_width<=800 ? VK_FONT_12 : VK_FONT_14);
	VKFontSetInit();
#endif
	if( !vk_font ) error("Cannot load font %s\n", vk_font_name);

	vk_hand = XCreateFontCursor(display, XC_hand2);
	vk_cursor = XCreateFontCursor(display, XC_left_ptr);
	vk_attrib.override_redirect = True;
#ifdef USE_XFT
	vk_text_height = vk_font->height;
	vk_text_ascent = vk_font->ascent;
#else
	{
		XFontSetExtents *fse = XExtentsOfFontSet(vk_fontset);
		vk_text_height = fse->max_logical_extent.height + 4;
		vk_text_ascent = vk_font->ascent;
	}
#endif
}
/*----------------------------------------------------------------------------*/
void VKFreeXResource()
{
	XFreeCursor(display, vk_hand);
	XFreeCursor(display, vk_cursor);
#ifdef USE_XFT
	XftColorFree(display, visual, colormap, &clRed);
	XftColorFree(display, visual, colormap, &clYellow);
	XftColorFree(display, visual, colormap, &clWhite);
	XftColorFree(display, visual, colormap, &clBlack);
	XftColorFree(display, visual, colormap, &clBlue);
	XftColorFree(display, visual, colormap, &clGray);
	XftColorFree(display, visual, colormap, &clMenuBar);
	XftFontClose(display, vk_font);
#else
	XFreeFontSet(display, vk_fontset);
	XFreeFont(display, vk_font);
#endif
}
/*----------------------------------------------------------------------------*/
