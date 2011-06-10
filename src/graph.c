#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "gtkplot2/graph.h"
#include "graph_int.h"
#include <stdio.h>

void graph_get_minmax(const Graph* graph, float* min, float* max){
  *min = graph->samples[0].y;
  *max = graph->samples[0].y;

  for ( int i = 1; i < SAMPLES; i++ ){
    if ( graph->samples[i].y < *min ){
      *min = graph->samples[i].y;
    }
    if ( graph->samples[i].y > *max ){
      *max = graph->samples[i].y;
    }
  }
}

void graph_sample_add(struct Graph* graph, float x, float y){
  graph->samples[graph->write].x = x;
  graph->samples[graph->write].y = y;
  graph->write = (graph->write + 1 ) % SAMPLES;
  graph->read = (graph->read + 1 ) % SAMPLES;
}

static double calc_y(float value, float scale, float min, float height){
  return height - (value-min) * scale;
}

void graph_render(const Graph* graph, cairo_t* cr, float min, float max, unsigned int lines){
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_paint(cr);

  /* constants */
  const int width  = graph->width  - graph->margin[RIGHT] - graph->margin[LEFT];
  const int height = graph->height - graph->margin[TOP]   - graph->margin[BOTTOM];

  /* background */
  cairo_set_source_rgba(cr, 1, 1, 1, 1);
  cairo_rectangle(cr,
		  graph->margin[LEFT], graph->margin[TOP],
		  width, height);
  cairo_fill(cr);

  /* calc scale */
  const float d = max - min;
  const float s = ((float)height) / d;

  /* actual graph */
  const int offset_y = graph->margin[TOP];
  const float dx = ((float)width) / (SAMPLES-1);
  int c = graph->read;
  cairo_set_source_rgba(cr, 0,0,0,1);
  cairo_move_to(cr, graph->margin[LEFT], calc_y(graph->samples[c].y, s, min, height) + offset_y);
  for ( int i = 1; i < SAMPLES; i++ ){
    int j = (c+i);
    if ( j >= SAMPLES ){
      j -= SAMPLES;
    }

    const float x = i * dx;
    const float y = calc_y(graph->samples[j].y, s, min, height);
    cairo_line_to(cr, x + graph->margin[LEFT], y + offset_y);
  }
  cairo_stroke (cr);

  /* scales */
  char buf[64];
  cairo_select_font_face (cr, "Sans",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 10.0);

  snprintf(buf, 64, "%04.2f", max);
  cairo_move_to (cr, 5, graph->margin[TOP] + 10);
  cairo_show_text (cr, buf);

  snprintf(buf, 64, "%04.2f", min);
  cairo_move_to (cr, 5, height - graph->margin[BOTTOM] + 10);
  cairo_show_text (cr, buf);
}
