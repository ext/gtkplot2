#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "gtkplot2/graph.h"
#include "graph_int.h"
#include <stdio.h>

void graph_get_minmax(const Graph* graph, float* min, float* max){
  *min = graph->samples[0].y;
  *max = graph->samples[0].y;

  for ( int i = 1; i < SAMPLES_HAVE; i++ ){
    if ( graph->samples[i].y < *min ){
      *min = graph->samples[i].y;
    }
    if ( graph->samples[i].y > *max ){
      *max = graph->samples[i].y;
    }
  }
}

void graph_sample_add(struct Graph* graph, float x, float y){
  graph->write = (graph->write + 1 ) % SAMPLES_HAVE;
  graph->samples[graph->write].x = x;
  graph->samples[graph->write].y = y;
}

static double scale_y(float value, float scale, float min, float height){
  return height - (value-min) * scale;
}

static void render_series(const Graph* graph, cairo_t* cr, float scale, float min, float max){
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0,0,0,1);

  /* constants */
  const int width  = graph->width  - graph->margin[RIGHT] - graph->margin[LEFT];
  const int height = graph->height - graph->margin[TOP]   - graph->margin[BOTTOM];
  const int offset_y = graph->margin[TOP];
  const float dx = ((float)width) / (SAMPLES_SHOW-1);

  int c = graph->write; /* position of current sample */
  float xref = graph->samples[c].x; /* x-value of (previous) sample */
  float x = width; /* current position (in graph) */
  int n = 1; /* sample counter */

  /* move to position of latest sample */
  cairo_move_to(cr, width + graph->margin[LEFT], scale_y(graph->samples[c].y, scale, min, height) + offset_y);

  /* Render as many samples as possible, until it runs out of screen-space or 
   * there is no more samples left. Rendering is done backwards, starting at the
   * latest sample. */
  while ( x > 0.0f ){

    /* position of sample in circular array */
    int i = c - n++;
    if ( i < 0 ){
      i += SAMPLES_HAVE;
    }

    /* detect if counter wrapped (to few samples) */
    if ( i == graph->write ){
      break;
    }

    /* current sample value */
    const float ox = graph->samples[i].x;
    const float oy = graph->samples[i].y;

    /* calculate the new position */
    x -= (xref - ox) * dx;
    x = x > 0.0f ? x : 0.0f; /** @bug This clipping will alter the angle of the line, must calculate what the new y-value will be */
    xref = ox; /* store reference */
    const float y = scale_y(oy, scale, min, height); /* scaled y-position */

    /* render line */
    cairo_line_to(cr, x + graph->margin[LEFT], y + offset_y);
  }
  cairo_stroke (cr);

  cairo_restore(cr);
}

static void render_scale(const Graph* graph, cairo_t* cr, unsigned int lines, float min, float max){
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0, 0, 0, 1);

  /* constants */
  const int height = graph->height - graph->margin[TOP]   - graph->margin[BOTTOM];
  const float delta = max - min;

  char buf[64];
  cairo_select_font_face (cr, "Sans",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 10.0);
    
  float dy = (float)(height-10) / (lines+1);
  for ( unsigned int i = 0; i <= lines+1; i++ ){
    float s = 1.0 - (float)i / (lines+1);
    snprintf(buf, 64, "%9.2f", min + delta * s);
    cairo_move_to (cr, 5, graph->margin[TOP] + 10 + i * dy);
    cairo_show_text (cr, buf);
  }

  cairo_restore(cr);
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
  const float delta = max - min;
  const float scale = ((float)height) / delta;

  /* help lines */
  cairo_save(cr);
  {
    cairo_set_line_width(cr, 0.5);
    cairo_set_source_rgba(cr, 0.5, 0.5, 0.8, 1.0);
    float dy = (float)height / (lines+1);
    for ( unsigned int i = 1; i <= lines; i++ ){
      cairo_move_to(cr, graph->margin[LEFT]        , graph->margin[TOP] + i * dy);
      cairo_line_to(cr, graph->margin[LEFT] + width, graph->margin[TOP] + i * dy);
      cairo_stroke(cr);
    }
  }
  cairo_restore(cr);

  /* actual graph */
  render_series(graph, cr, scale, min, max);

  /* scales */
  render_scale(graph, cr, lines, min, max);
}
