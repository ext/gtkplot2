#include "gtkplot.h"
#include "gtk/gtk.h"

static gboolean update(GtkPlot2* plot) {
  gtk_widget_queue_draw(GTK_WIDGET(plot));
  return TRUE;
}

static gboolean expose(GtkPlot2* plot) {
  static int n = 0;
  Graph* graph = gtk_plot2_get_graph(GTK_PLOT2(plot));

  float value = gtk_plot2_rendertime(plot);
  printf("t: %.1f\n", value);
  graph_sample_add(graph, n++, value);
  return TRUE;
}

int main(int argc, char* argv[]){
  gtk_init( &argc, &argv );
 
  GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(win, "gtkplot2");
 
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(win), vbox);
  gtk_widget_show( vbox );
 
  gtk_signal_connect(GTK_OBJECT(win), "destroy", gtk_main_quit, NULL);
  
  GtkWidget* plot = gtk_plot2_new();
  gtk_signal_connect_after(GTK_OBJECT(plot), "expose-event", G_CALLBACK(expose), plot);

  gtk_box_pack_start(GTK_BOX(vbox), plot, TRUE, TRUE, 0);
  gtk_widget_show_all(win);
 
  g_timeout_add(25, (GSourceFunc)update, plot);
 
  gtk_main();
}
