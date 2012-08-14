#include "stopclock.h"

static void timer_start(struct application_info *app)
{
	if (app->timer.state == TIMER_DEAD)
	{
		app->timer.gtimer = g_timer_new();
		app->timer.state = TIMER_RUNNING;
	}
	else if (app->timer.state == TIMER_PAUSED)
	{
		g_timer_continue(app->timer.gtimer);
		app->timer.state = TIMER_RUNNING;
	}
}

static void timer_pause(struct application_info *app)
{
	if (app->timer.state == TIMER_RUNNING)
	{
		g_timer_stop(app->timer.gtimer);
		app->timer.state = TIMER_PAUSED;
	}
}

static void timer_reset(struct application_info *app)
{
	if (app->timer.state == TIMER_RUNNING
	    || app->timer.state == TIMER_PAUSED)
	{
		g_timer_destroy(app->timer.gtimer);
		app->timer.state = TIMER_DEAD;
	}
}

static void update_clock(struct application_info *app)
{
	char fmt[6] = "";
	time_t t;
	struct tm *tmp;

	t = time(NULL);
	tmp = localtime(&t);
	if (tmp == NULL)
	{
		perror("update_clock: localtime");
		return;
	}
	if (strftime(fmt, sizeof(fmt), "%R", tmp) == 0)
	{
		fprintf(stderr, "update_clock: strftime failed: 0\n");
		return;
	}
	gtk_label_set_text(GTK_LABEL(app->gui.clock), fmt);
}

static void seconds_to_duration(int elapsed, int *hr, int *min, int *sec)
{
	*sec = elapsed % 60;
	*min = elapsed / 60;
	*hr = *min / 60;
	*min = *min % 60;
}

static void update_timer(struct application_info *app)
{
	int elapsed;
	int sec, min, hr;
	gchar *fmt;

	if (app->timer.state == TIMER_DEAD)
	{
		gtk_label_set_text(GTK_LABEL(app->gui.timer), "00:00:00");
	}
	else
	{
		elapsed = (int)g_timer_elapsed(app->timer.gtimer, NULL);
		seconds_to_duration(elapsed, &hr, &min, &sec);

		fmt = g_strdup_printf("%02d:%02d:%02d", hr, min, sec);
		gtk_label_set_text(GTK_LABEL(app->gui.timer), fmt);
		g_free(fmt);
	}
}

static gboolean update_labels(struct application_info *app)
{
	update_clock(app);
	update_timer(app);
	return TRUE;
}

static void create_gui(struct application_info *app)
{
	PangoFontDescription *font_desc = NULL;

	/* Main window. */
	app->gui.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(app->gui.window), "delete_event",
	                 G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(app->gui.window), "destroy",
	                 G_CALLBACK(gtk_main_quit), NULL);
	gtk_window_set_has_resize_grip(GTK_WINDOW(app->gui.window), FALSE);
	gtk_window_set_title(GTK_WINDOW(app->gui.window), "stopclock");

	/* The layout box. */
	app->gui.box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	/* Create two labels and adjust their font. */
	app->gui.clock = gtk_label_new("00:00");
	app->gui.timer = gtk_label_new("00:00:00");
	update_labels(app);

	font_desc = pango_font_description_from_string(LABEL_FONT);
	gtk_widget_modify_font(app->gui.clock, font_desc);
	gtk_widget_modify_font(app->gui.timer, font_desc);
	pango_font_description_free(font_desc);

	/* Timeout callback. */
	g_timeout_add(500, (GSourceFunc)update_labels, app);

	/* Connect all the containers and show the window. */
	gtk_box_pack_start(GTK_BOX(app->gui.box), app->gui.clock,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(app->gui.box), app->gui.timer,
	                   TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(app->gui.window), app->gui.box);
	gtk_widget_show_all(app->gui.window);
}

static gboolean read_stdin(GIOChannel *source, GIOCondition condition,
                           gpointer data)
{
	/* Unused. */
	(void)source;
	(void)condition;

	GString *buffer = g_string_new("");
	GError *err = NULL;
	gchar **tokens = NULL;
	gchar *line_copy = NULL;
	gint num_tokens = 0;
	struct application_info *app = (struct application_info *)data;

	/* Mixing stdio.h and GIO is probably a bad idea, thus we better use
	 * GIO here to read from stdin. */
	if (g_io_channel_read_line_string(app->gio.stdin_channel, buffer,
	                                  NULL, &err) == G_IO_STATUS_NORMAL)
	{
		/* Copy string from GString into a gchar so we can use
		 * g_strchomp() on it. */
		line_copy = g_strdup(buffer->str);
		g_string_free(buffer, TRUE);

		tokens = g_strsplit(g_strchomp(line_copy), " ", -1);
		for (num_tokens = 0; tokens[num_tokens] != NULL; num_tokens++)
			/* No body, just count the tokens. */ ;

		if (num_tokens >= 1)
		{
			if (g_strcmp0(tokens[0], "start") == 0)
			{
				timer_start(app);
			}
			else if (g_strcmp0(tokens[0], "pause") == 0)
			{
				timer_pause(app);
			}
			else if (g_strcmp0(tokens[0], "reset") == 0)
			{
				timer_reset(app);
			}
		}

		g_strfreev(tokens);
		g_free(line_copy);
	}

	if (err != NULL)
	{
		g_error_free(err);
	}

	/* Do not remove this watch. */
	return TRUE;
}

static void setup_watch_stdin(struct application_info *app)
{
	/* This is specific to Unix. When there's data on stdin,
	 * read_stdin() will get called. */
	app->gio.stdin_channel = g_io_channel_unix_new(fileno(stdin));
	g_io_add_watch(app->gio.stdin_channel, G_IO_IN, read_stdin, app);
}

int main(int argc, char *argv[])
{
	/* We store everything about this application in this struct. It
	 * contains several sub-structs. */
	struct application_info app;
	app.timer.gtimer = NULL;
	app.timer.state = TIMER_DEAD;

	/* Initialize Gtk. Must be done before we can do anything related to
	 * Gtk. */
	gtk_init(&argc, &argv);

	/* This program will read commands from stdin. As we're inside of a
	 * Gtk program, we must use GIO facilities for this job. */
	setup_watch_stdin(&app);

	/* Create all widgets, show them, connect signals, ... */
	create_gui(&app);

	gtk_main();
	exit(EXIT_SUCCESS);
}
