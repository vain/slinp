#ifndef STOPCLOCK_H
#define STOPCLOCK_H

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>

/* Font for the two labels. */
#define LABEL_FONT "Serif 64"

struct gui_info
{
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *clock;
	GtkWidget *timer;
};

struct gio_info
{
	GIOChannel *stdin_channel;
};

enum timer_mode
{
	TIMER_DEAD,
	TIMER_PAUSED,
	TIMER_RUNNING
};

struct timer_info
{
	enum timer_mode state;
	GTimer *gtimer;
};

struct application_info
{
	struct gui_info gui;
	struct timer_info timer;
	struct gio_info gio;
};

#endif /* STOPCLOCK_H */
