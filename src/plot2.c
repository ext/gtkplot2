#include "gtkplot.h"
#include "graph_int.h"
#include <math.h>
#include <stdint.h>
#include <sys/time.h>

struct _GtkPlot2 {
  GtkWidget widget;
  struct Graph graph;
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

  GTK_PLOT2(widget)->graph.width = allocation->width;
  GTK_PLOT2(widget)->graph.height = allocation->height;
  
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

static gboolean expose(GtkWidget* widget, GdkEventExpose* event){
  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_PLOT2(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  struct timeval a, b;
  gettimeofday(&a, NULL);

  {
    const GtkPlot2* plot = GTK_PLOT2(widget);
    const struct Graph* graph = &plot->graph;
    cairo_t *cr = gdk_cairo_create(widget->window);

    float min, max;
    graph_get_minmax(graph, &min, &max);

    {
      float d = fmodf(min, 250.0f);
      min -= d > 0 ? d : 250.0f + d;
    }
    {
      float d = fmodf(max, 250.0f);
      max += d > 0 ? 250.0 - d : -d;
    }

    graph_render(graph, cr, min, max, 3);
    cairo_destroy(cr);
  }

  gettimeofday(&b, NULL);
  GTK_PLOT2(widget)->graph.rendertime = (b.tv_sec-a.tv_sec) * 1000000 + (b.tv_usec-a.tv_usec);

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

void graph_init(struct Graph* graph){
  graph->write = 0;
  graph->margin[TOP]    = 5;
  graph->margin[RIGHT]  = 5;
  graph->margin[BOTTOM] = 5;
  graph->margin[LEFT]   = 50;
  sprintf(graph->title,   "%256s", "Sample plot");
  sprintf(graph->label_x, "%256s", "rendering time (µs)");
  sprintf(graph->label_y, "%256s", "time");
}

static void init(GtkPlot2* plot){
  graph_init(&plot->graph);
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

int gtk_plot2_rendertime(GtkPlot2* plot){
  return plot->graph.rendertime;
}

Graph* gtk_plot2_get_graph(GtkPlot2* plot){
  return &plot->graph;
}
