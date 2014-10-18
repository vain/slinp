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

static gboolean load_pdf(struct application_info *app, char *path)
{
	gchar *absolute, *uri;
	GError *err = NULL;

	/* Build a URI from the path name. */
	if (g_path_is_absolute(path))
	{
		absolute = g_strdup(path);
	}
	else
	{
		gchar *dir = g_get_current_dir();
		absolute = g_build_filename(dir, path, NULL);
		g_free(dir);
	}

	uri = g_filename_to_uri(absolute, NULL, &err);
	g_free(absolute);

	if (uri == NULL)
	{
		printf("load_pdf: %s\n", err->message);
		return 1;
	}

	/* Load the file via its URI. */
	app->pdf.doc = poppler_document_new_from_file(uri, NULL, &err);
	if (app->pdf.doc == NULL)
	{
		fprintf(stderr, "load_pdf: %s\n", err->message);
		g_error_free(err);
		return FALSE;
	}

	app->pdf.num_pages = poppler_document_get_n_pages(app->pdf.doc);
	if (app->pdf.num_pages <= 0)
	{
		fprintf(stderr, "load_pdf: PDF has <= 0 pages.\n");
		g_object_unref(app->pdf.doc);
		return FALSE;
	}

	/* We will start with a blank window. */
	app->pdf.page = -1;

	return TRUE;
}

static void update_window_title(struct application_info *app)
{
	gchar *doc_title = poppler_document_get_title(app->pdf.doc);
	gchar *window_title = NULL;

	if (doc_title == NULL)
		doc_title = g_strdup("<Untitled>");

	if (app->pdf.page < 0 || app->pdf.page >= app->pdf.num_pages)
	{
		window_title = g_strdup_printf("showpdf: %s", doc_title);
	}
	else
	{
		window_title = g_strdup_printf("showpdf: %s [%d/%d]", doc_title,
		                               app->pdf.page + 1,
		                               app->pdf.num_pages);
	}

	gtk_window_set_title(GTK_WINDOW(app->gui.window), window_title);

	g_free(doc_title);
	g_free(window_title);
}

static gboolean on_canvas_draw(GtkWidget *widget, cairo_t *cr,
                               struct application_info *app)
{
	/* Unused. */
	(void)widget;

	gdouble popwidth, popheight;
	gdouble screen_ratio, page_ratio, scale;
	gdouble tx, ty, w, h;
	PopplerPage *page = NULL;

	/* Display nothing? */
	if (app->pdf.page < 0 || app->pdf.page >= app->pdf.num_pages)
	{
		cairo_save(cr);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_rectangle(cr, 0, 0,
		                app->gui.canvas_width, app->gui.canvas_height);
		cairo_fill(cr);
		cairo_restore(cr);
		return TRUE;
	}

	/* Get the page and its size. */
	page = poppler_document_get_page(app->pdf.doc, app->pdf.page);
	poppler_page_get_size(page, &popwidth, &popheight);

	/* That's it: Compare screen and page ratio. This will cover all 4
	 * cases that could happen. */
	page_ratio = popwidth / popheight;
	screen_ratio = (double)app->gui.canvas_width /
	               (double)app->gui.canvas_height;

	if (screen_ratio > page_ratio)
	{
		/* Fit size. */
		h = app->gui.canvas_height;
		w = h * page_ratio;
		scale = h / popheight;
		/* Center page. */
		tx = (app->gui.canvas_width - popwidth * scale) * 0.5;
		ty = 0;
	}
	else
	{
		w = app->gui.canvas_width;
		h = w / page_ratio;
		scale = w / popwidth;
		tx = 0;
		ty = (app->gui.canvas_height - popheight * scale) * 0.5;
	}

	/* A black background. Push and pop cairo contexts, so we have a
	 * clean state afterwards. */
	cairo_save(cr);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_rectangle(cr, 0, 0,
	                app->gui.canvas_width, app->gui.canvas_height);
	cairo_fill(cr);
	cairo_restore(cr);

	/* A white background, i.e. "paper color", only below the PDF. */
	cairo_save(cr);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, tx, ty, w, h);
	cairo_fill(cr);
	cairo_restore(cr);

	/* Render the page centered and scaled. */
	cairo_translate(cr, tx, ty);
	cairo_scale(cr, scale, scale);
	poppler_page_render(page, cr);

	/* We no longer need that page. */
	g_object_unref(G_OBJECT(page));

	/* Nobody else draws to this widget. */
	return TRUE;
}

static void on_canvas_size_allocate(GtkWidget *widget,
                                    GtkAllocation *al,
                                    struct application_info *app)
{
	/* Unused. */
	(void)widget;

	app->gui.canvas_width = al->width;
	app->gui.canvas_height = al->height;
}

static void create_gui(struct application_info *app)
{
	/* Main window. */
	app->gui.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(app->gui.window), "delete_event",
	                 G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(app->gui.window), "destroy",
	                 G_CALLBACK(gtk_main_quit), NULL);
	update_window_title(app);

	/* PDF drawing area. */
	app->gui.canvas = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(app->gui.canvas), "draw",
	                 G_CALLBACK(on_canvas_draw), app);
	g_signal_connect(G_OBJECT(app->gui.canvas), "size_allocate",
	                 G_CALLBACK(on_canvas_size_allocate), app);

	/* Connect both widgets and show them. */
	gtk_container_add(GTK_CONTAINER(app->gui.window), app->gui.canvas);
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
	int new_page = 0;
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

		if (num_tokens >= 2)
		{
			if (g_strcmp0(tokens[0], "go_page") == 0)
			{
				/* We expect 1-based page numbers here. Internally,
				 * though, we use 0-based page numbers (poppler wants
				 * them anyway). */
				new_page = atoi(tokens[1]);
				if (new_page <= 0 || new_page > app->pdf.num_pages)
					app->pdf.page = -1;
				else
					app->pdf.page = new_page - 1;

				update_window_title(app);
				gtk_widget_queue_draw(app->gui.canvas);
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

	/* Initialize Gtk. Must be done before we can do anything related to
	 * GLib, Poppler or Gtk. */
	gtk_init(&argc, &argv);

	/* Load the PDF specified as argv[1]. */
	if (argc < 2)
	{
		fprintf(stderr, "main: Path to PDF file required.\n");
		exit(EXIT_FAILURE);
	}

	if (!load_pdf(&app, argv[1]))
	{
		fprintf(stderr, "main: Cannot load PDF, giving up.\n");
		exit(EXIT_FAILURE);
	}

	/* This program will read commands from stdin. As we're inside of a
	 * Gtk program, we must use GIO facilities for this job. */
	setup_watch_stdin(&app);

	/* Create all widgets, show them, connect signals, ... */
	create_gui(&app);

	gtk_main();
	exit(EXIT_SUCCESS);
}
