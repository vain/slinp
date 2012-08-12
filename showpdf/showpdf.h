#ifndef SHOWPDF_H
#define SHOWPDF_H

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/poppler.h>

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct gui_info
{
	GtkWidget *window;

	GtkWidget *canvas;
	int canvas_width, canvas_height;
};

struct pdf_info
{
	PopplerDocument *doc;
	int num_pages;
	int page;
};

struct gio_info
{
	GIOChannel *stdin_channel;
};

struct application_info
{
	struct gui_info gui;
	struct pdf_info pdf;
	struct gio_info gio;
};

#endif /* SHOWPDF_H */
