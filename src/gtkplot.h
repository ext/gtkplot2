#ifndef GTK_PLOT2_H
#define GTK_PLOT2_H

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
 
G_BEGIN_DECLS

#define GTK_PLOT2(obj) GTK_CHECK_CAST(obj, gtk_plot2_get_type(), GtkPlot2)
#define GTK_PLOT2_CLASS(cls) GTK_CHECK_CLASS_CAST(cls, gtk_plot2_get_type(), GtkPlot2Class)
#define GTK_IS_PLOT2(obj) GTK_CHECK_TYPE(obj, gtk_plot2_get_type())
 
typedef struct _GtkPlot2 GtkPlot2;
typedef struct _GtkPlot2Class GtkPlot2Class;

struct _GtkPlot2Class {
  GtkWidgetClass parent_class;
};

GtkType gtk_plot2_get_type();
GtkWidget* gtk_plot2_new();
void gtk_plot2_sample_clear(GtkPlot2* plot);
void gtk_plot2_sample_add(GtkPlot2* plot, float x, float y);
int gtk_plot2_rendertime(GtkPlot2* plot);

G_END_DECLS

#endif /* GTK_PLOT2_H */
