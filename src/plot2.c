#include "gtkplot.h"
#include <math.h>
#include <stdint.h>
#include <sys/time.h>

#define SAMPLES 100

struct sample {
  float x;
  float y;
};

struct _GtkPlot2 {
  GtkWidget widget;

  int width;
  int height;
  int margin[4]; /* top right bottom left */
  char title[256];
  char label_x[256];
  char label_y[256];

  int rendertime;

  unsigned int read;
  unsigned int write;
  struct sample samples[SAMPLES];
};

static void size_request(GtkWidget* widget, GtkRequisition* requisition){
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_PLOT2(widget));
  g_return_if_fail(requisition != NULL);
  
  requisition->width = 800;
  requisition->height = 600;
}

static void size_allocate(GtkWidget* widget, GtkAllocation* allocation){
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_PLOT2(widget));
  g_return_if_fail(allocation != NULL);
  
  widget->allocation = *allocation;

  GTK_PLOT2(widget)->width = allocation->width;
  GTK_PLOT2(widget)->height = allocation->height;
  
  if (GTK_WIDGET_REALIZED(widget)) {
    gdk_window_move_resize(widget->window,
			   allocation->x, allocation->y,
			   allocation->width, allocation->height
			   );
  }
}

static void realize(GtkWidget* widget){
  GdkWindowAttr attributes;
  guint attributes_mask;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_PLOT2(widget));

  GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;

  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  widget->window = gdk_window_new(
				  gtk_widget_get_parent_window (widget),
				  & attributes, attributes_mask
				  );

  gdk_window_set_user_data(widget->window, widget);

  widget->style = gtk_style_attach(widget->style, widget->window);
  gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

static double calc_y(float value, float scale, float min, float height){
  return height - (value-min) * scale;
}

static void paint(GtkPlot2* plot){
  GtkWidget* widget = GTK_WIDGET(plot);
  cairo_t *cr;

  cr = gdk_cairo_create(widget->window);

  cairo_translate(cr, 0, 7);

  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_paint(cr);

  /* constants */
  const int width = plot->width - plot->margin[1] - plot->margin[3];
  const int height = plot->height - plot->margin[0] - plot->margin[2];

  /* background */
  cairo_set_source_rgba(cr, 1, 1, 1, 1);
  cairo_rectangle(cr,
		  plot->margin[3], plot->margin[0],
		  width, height);
  cairo_fill(cr);

  /* get scale */
  float min = plot->samples[0].y, max = plot->samples[0].y;
  for ( int i = 1; i < SAMPLES; i++ ){
    if ( plot->samples[i].y < min ){
      min = plot->samples[i].y;
    }
    if ( plot->samples[i].y > max ){
      max = plot->samples[i].y;
    }
  }

  min -= fmodf(min, 250.0f);
  max += 250.0f - fmodf(max, 250.0f);

  const float d = max - min;
  const float s = ((float)height) / d;

  /* lines */
  const int offset_y = plot->margin[0];
  const float dx = ((float)width) / (SAMPLES-1);
  int c = plot->read;
  cairo_set_source_rgba(cr, 0,0,0,1);
  cairo_move_to(cr, plot->margin[3], calc_y(plot->samples[c].y, s, min, height) + offset_y);
  for ( int i = 1; i < SAMPLES; i++ ){
    int j = (c+i);
    if ( j >= SAMPLES ){
      j -= SAMPLES;
    }

    const float x = i * dx;
    const float y = calc_y(plot->samples[j].y, s, min, height);
    cairo_line_to(cr, x + plot->margin[3], y + offset_y);
  }
  cairo_stroke (cr);

  /* scales */
  char buf[64];
  cairo_select_font_face (cr, "Sans",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 10.0);

  snprintf(buf, 64, "%04.2f", max);
  cairo_move_to (cr, 5, plot->margin[0] + 10);
  cairo_show_text (cr, buf);

  snprintf(buf, 64, "%04.2f", min);
  cairo_move_to (cr, 5, height - plot->margin[2]+10);
  cairo_show_text (cr, buf);

  cairo_destroy(cr);
}

static gboolean expose(GtkWidget* widget, GdkEventExpose* event){
  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_PLOT2(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  struct timeval a, b;
  gettimeofday(&a, NULL);

  paint(GTK_PLOT2(widget));

  gettimeofday(&b, NULL);
  GTK_PLOT2(widget)->rendertime = (b.tv_sec-a.tv_sec) * 1000000 + (b.tv_usec-a.tv_usec);

  return FALSE;
}

static void destroy(GtkObject* object){
  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_PLOT2(object));

  //GtkPlot2* plot = GTK_PLOT2(object);
  GtkPlot2Class* cls = gtk_type_class(gtk_widget_get_type());

  if (GTK_OBJECT_CLASS(cls)->destroy) {
    (* GTK_OBJECT_CLASS(cls)->destroy) (object);
  }
}

static void class_init(GtkPlot2Class* cls){
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;

  widget_class = (GtkWidgetClass *)cls;
  object_class = (GtkObjectClass *)cls;

  widget_class->realize = realize;
  widget_class->size_request = size_request;
  widget_class->size_allocate = size_allocate;
  widget_class->expose_event = expose;

  object_class->destroy = destroy;
}

static void init(GtkPlot2* plot){
  plot->margin[0] = 5;
  plot->margin[1] = 5;
  plot->margin[2] = 10;
  plot->margin[3] = 50;
  sprintf(plot->title,   "%256s", "Sample plot");
  sprintf(plot->label_x, "%256s", "rendering time (µs)");
  sprintf(plot->label_y, "%256s", "time");

  //gtk_signal_connect(GTK_OBJECT(plot), "size-allocate", size_allocate, plot);
}

GtkType gtk_plot2_get_type(){
  static GtkType type = 0;
  static const GtkTypeInfo info = {
    "GtkPlot2",
    sizeof(GtkPlot2),
    sizeof(GtkPlot2Class),
    (GtkClassInitFunc) class_init,
    (GtkObjectInitFunc) init,
    NULL,
    NULL,
    (GtkClassInitFunc) NULL
  };

  if ( !type ) {
    type = gtk_type_unique(GTK_TYPE_WIDGET, &info);
  }

  return type;
}

GtkWidget* gtk_plot2_new(void){
  return GTK_WIDGET(gtk_type_new(gtk_plot2_get_type()));
}

void gtk_plot2_sample_clear(GtkPlot2* plot);
void gtk_plot2_sample_add(GtkPlot2* plot, float x, float y){
  plot->samples[plot->write].x = x;
  plot->samples[plot->write].y = y;
  plot->write = (plot->write + 1 ) % SAMPLES;
  plot->read = (plot->read + 1 ) % SAMPLES;
}

int gtk_plot2_rendertime(GtkPlot2* plot){
  return plot->rendertime;
}
