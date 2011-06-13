#include "gtkplot.h"
#include "gtk/gtk.h"
#include <math.h>

static gint active = 0;

static gboolean update(GtkPlot2* plot) {
  if ( !active ){
    return FALSE;
  }

  gtk_widget_queue_draw(GTK_WIDGET(plot));
  return TRUE;
}

static gboolean expose(GtkPlot2* plot) {
  static int n = 0;
  static float v = 0.0f;

  Graph* graph = gtk_plot2_get_graph(GTK_PLOT2(plot));

  //float value = gtk_plot2_rendertime(plot);
  float value = sin(v * 150) * 25 + sin(v * 75) * 25 + sin(v * 2)* 150;

  graph_sample_add(graph, n, value);

  n++;
  v += 0.003;
  return TRUE;
}

static void quit(){
  active = 0;
  gtk_main_quit();
}

int main(int argc, char* argv[]){
  gtk_init( &argc, &argv );
 
  GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(win, "gtkplot2");
 
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(win), vbox);
  gtk_widget_show( vbox );
 
  gtk_signal_connect(GTK_OBJECT(win), "destroy", quit, NULL);
  
  GtkWidget* plot = gtk_plot2_new();
  gtk_signal_connect_after(GTK_OBJECT(plot), "expose-event", G_CALLBACK(expose), plot);

  gtk_box_pack_start(GTK_BOX(vbox), plot, TRUE, TRUE, 0);
  gtk_widget_show_all(win);
 
  active = 1;
  g_timeout_add(5, (GSourceFunc)update, plot);
 
  gtk_main();
}
