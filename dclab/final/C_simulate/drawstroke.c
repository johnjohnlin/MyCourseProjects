#include <gtk/gtk.h>
#include <stdio.h>
#include <math.h>
//#include <float.h>
#include <assert.h>

#define NUM_PT 32
#define SENSITIVE_DIST 32.0
#define MAX_FILENAME_NOEXT 10

typedef struct {
	double x, y;
} Point;

typedef struct {
	int n;
	double maxX, minX, maxY, minY;
	Point p0;
	Point pprev;
	Point move[NUM_PT];
} Stroke;

static Stroke ss;
static GtkWidget *window, *area, *name, *button, *table;

static gboolean button_pressed(GtkWidget*, GdkEventButton*, gpointer*);
static gboolean motion_notify(GtkWidget*, GdkEventMotion*, gpointer*);
static gboolean key_pressed(GtkWidget*, GdkEventKey*, gpointer*);
static gboolean expose_event(GtkWidget*, GdkEventExpose*, gpointer*);
static void redraw(GtkWidget *area);
static gboolean save(GtkWidget*, GdkEvent*, gpointer*);

int main (int argc, char *argv[])
{
	ss.n = 0;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	table = gtk_table_new(2, 1, FALSE);
	button = gtk_button_new_with_label("Save!");
	name = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(name), "Enter the filename");
	gtk_entry_set_width_chars(GTK_ENTRY(name), MAX_FILENAME_NOEXT);
	area = gtk_drawing_area_new();
	gtk_widget_set_size_request(area, 512, 512);
	gtk_table_attach(GTK_TABLE(table), area     , 0, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(table), name     , 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(table), button   , 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

	gtk_window_set_title(GTK_WINDOW (window), "Drawing Areas");
	g_signal_connect (G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect (G_OBJECT(button), "clicked", G_CALLBACK(save), NULL);

	gtk_widget_set_app_paintable(area, TRUE);
	gtk_widget_add_events(area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_MOTION_MASK | GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(area), "button_press_event", G_CALLBACK(button_pressed), NULL);
	g_signal_connect(G_OBJECT(area), "motion_notify_event", G_CALLBACK(motion_notify), NULL);
	g_signal_connect(G_OBJECT(area), "key_press_event", G_CALLBACK(key_pressed), NULL);
	g_signal_connect(G_OBJECT(area), "expose_event", G_CALLBACK(expose_event), NULL);
	gtk_container_add(GTK_CONTAINER(window), table);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}

static gboolean expose_event(GtkWidget *area, GdkEventExpose *event, gpointer *pointer)
{
	int i;
	redraw(area);
	return TRUE;
}

static gboolean button_pressed(GtkWidget *area, GdkEventButton *event, gpointer *pointer)
{
	printf("button_pressed, n = %d\n", ss.n);
	ss.p0.x = ss.pprev.x = ss.maxX = ss.minX = event->x;
	ss.p0.y = ss.pprev.y = ss.maxY = ss.minY = event->y;
	ss.n = 0;
	return FALSE;
}

static inline void update_maxmin(double x, double y)
{
	if (x > ss.maxX) {
	   ss.maxX = x;
	} else if (x < ss.minX) {
		ss.minX = x;
	}
	if (y > ss.maxY) {
		ss.maxY = y;
	} else if (y < ss.minY) {
		ss.minY = y;
	}
}

static gboolean motion_notify(GtkWidget *area, GdkEventMotion *event, gpointer *pointer)
{
	printf("motion_notify, n = %d\n", ss.n);
	double movex;
	double movey;
	if (ss.n == NUM_PT) {
		goto ret;
	}
	update_maxmin(event->x, event->y);
	movex = event->x - ss.pprev.x;
	movey = event->y - ss.pprev.y;
	printf("(%lf, %lf)\n", movex, movey);
	if (movex*movex + movey*movey < SENSITIVE_DIST*SENSITIVE_DIST) {
		goto ret;
	}
	printf("update prev\n");
	ss.move[ss.n].x = movex;
	ss.move[ss.n].y = movey;
	ss.pprev.x = event->x;
	ss.pprev.y = event->y;
	++ss.n;
	redraw(area);
ret:
	return FALSE;
}

static gboolean key_pressed(GtkWidget *area, GdkEventKey *event, gpointer *pointer)
{
	if (event->keyval == GDK_DELETE) {
		gdk_window_clear(area->window);
	}
	return FALSE;
}

static void redraw(GtkWidget *area)
{
	cairo_t *c = gdk_cairo_create(area->window);
	cairo_set_source_rgba(c, 1.0, 1.0, 1.0, 1.0);
	cairo_paint(c);
	cairo_set_source_rgba(c, 1.0, 0.0, 0.0, 1.0);
	cairo_set_line_width(c, 3.0);
	cairo_new_path(c);
	cairo_move_to(c, ss.p0.x, ss.p0.y);
	int i;
	for (i = 0; i < ss.n; ++i) {
		cairo_rel_line_to(c, ss.move[i].x, ss.move[i].y);
	}
	cairo_stroke(c);
	cairo_destroy(c);
}

inline int my_atan2(int y, int x)
{
	int ret = 0;
	if (y < 0) {
		ret += 8;
		x = -x;
		y = -y;
	}
	if (x < 0) {
		ret += 4;
		int xtmp = x;
		x = y;
		y = -xtmp;
	}
	assert(x >= 0 && y >= 0);
	if (2*y > 5*x) {
		++ret;
	}
	if (y > x) {
		++ret;
	}
	if (5*y > 2*x) {
		++ret;
	}
	return ret;
}

static gboolean save(GtkWidget *button, GdkEvent *e, gpointer *p)
{
	static int counter = 0;
	char buf[32];
	snprintf(buf, 32, "%s.%d.txt", gtk_entry_get_text(GTK_ENTRY(name)), counter);
	++counter;
	FILE *fp = fopen(buf, "w");
	printf("save to %s\n", buf);
	// double normX = 128.0 / (ss.maxX - ss.minX);
	// double normY = 128.0 / (ss.maxY - ss.minY);
	double normX = 2.0;
	double normY = 2.0;

	int i;
	int dx_prev, dy_prev;
	int dx, dy;
	for (i = 0; i < ss.n; ++i) {
		dx = (int)(ss.move[i].x * normX);
		dy = (int)(ss.move[i].y * normY);
		fprintf(fp, "%d %d %d\n", dx, dy, my_atan2(dy, dx));
	}
	fclose(fp);
	return FALSE;
}
