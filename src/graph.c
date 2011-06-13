#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "gtkplot2/graph.h"
#include "graph_int.h"
#include <stdio.h>
#include <math.h>

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

static void render_series(const Graph* graph, const struct context* c, cairo_t* cr){
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0,0,0,1);

  /* constants */
  const int offset_y = graph->margin[TOP];
  const float dx = c->width / c->x_delta;

  int ref = graph->write; /* position of current sample */
  float xref = graph->samples[ref].x; /* x-value of (previous) sample */
  float x = c->width; /* current position (in graph) */
  int n = 1; /* sample counter */

  /* move to position of latest sample */
  cairo_move_to(cr, c->width + graph->margin[LEFT], scale_y(graph->samples[ref].y, c->y_scale, c->y_min, c->height) + offset_y);

  /* Render as many samples as possible, until it runs out of screen-space or 
   * there is no more samples left. Rendering is done backwards, starting at the
   * latest sample. */
  while ( x > 0.0f ){

    /* position of sample in circular array */
    int i = ref - n++;
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
    const float y = scale_y(oy, c->y_scale, c->y_min, c->height); /* scaled y-position */

    /* render line */
    cairo_line_to(cr, x + graph->margin[LEFT], y + offset_y);
  }
  cairo_stroke (cr);

  cairo_restore(cr);
}

static void render_lines_x(const Graph* graph, const struct context* c, cairo_t* cr){
  cairo_save(cr);

  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgba(cr, 0.5, 0.5, 0.8, 1.0);
  const float dy = c->height / c->lines;
  for ( unsigned int i = 1; i <= c->lines; i++ ){
    cairo_move_to(cr, graph->margin[LEFT]           , graph->margin[TOP] + i * dy);
    cairo_line_to(cr, graph->margin[LEFT] + c->width, graph->margin[TOP] + i * dy);
    cairo_stroke(cr);
  }

  cairo_restore(cr);
}

/**
 * Render vertical lines.
 * @param tick Distance between lines.
 * @param major How often major lines should occur.
 */
static void render_lines_y(const Graph* graph, const struct context* c, cairo_t* cr){
  cairo_save(cr);
  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgba(cr, 0.5, 0.5, 0.8, 1.0);

  /* constants */
  const float dx = c->width / c->x_delta;

  const int ref = graph->write; /* position of current sample */
  const float xref = graph->samples[ref].x; /* x-value of (previous) sample */
  float x = c->width - fmod(xref, c->tick) * dx; /* current position (in graph) */
  unsigned int n = floor(xref / c->tick);

  while ( x >= 0.0 ){
    if ( n-- % c->major == 0 ){
      cairo_set_line_width(cr, 1.5);
    } else {
      cairo_set_line_width(cr, 0.5);
    }

    cairo_move_to(cr, graph->margin[LEFT] + x, graph->margin[TOP]);
    cairo_line_to(cr, graph->margin[LEFT] + x, graph->margin[TOP] + c->height);
    cairo_stroke(cr);

    x -= c->tick * dx;
  }

  cairo_restore(cr);
}

static void render_scale(const Graph* graph, const struct context* c, cairo_t* cr){
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0, 0, 0, 1);

  char buf[64];
  cairo_select_font_face (cr, "Sans",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 10.0);
    
  float dy = (float)(c->height-10) / c->lines;
  for ( unsigned int i = 0; i <= c->lines; i++ ){
    float s = 1.0 - (float)i / c->lines;
    snprintf(buf, 64, "%9.2f", c->y_min + c->y_delta * s);
    cairo_move_to (cr, 5, graph->margin[TOP] + 10 + i * dy);
    cairo_show_text (cr, buf);
  }

  cairo_restore(cr);
}

void graph_render(const Graph* graph, cairo_t* cr, float min, float max, unsigned int lines){
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_paint(cr);

  /* constants */
  struct context c;
  c.width  = graph->width  - graph->margin[RIGHT] - graph->margin[LEFT];
  c.height = graph->height - graph->margin[TOP]	 - graph->margin[BOTTOM];
  c.x_min = graph->samples[graph->write].x - (SAMPLES_SHOW-1);
  c.x_max = graph->samples[graph->write].x;
  c.y_min = min;
  c.y_max = max;
  c.x_delta = c.x_max - c.x_min;
  c.y_delta = c.y_max - c.y_min;
  c.x_scale = c.width  / c.x_delta;
  c.y_scale = c.height / c.y_delta;
  c.tick = 25.0f;
  c.major = 4;
  c.lines = lines + 1; /* always a line a max */

  /* background */
  cairo_set_source_rgba(cr, 1, 1, 1, 1);
  cairo_rectangle(cr,
		  graph->margin[LEFT], graph->margin[TOP],
		  c.width, c.height);
  cairo_fill(cr);

  /* help lines */
  render_lines_x(graph, &c, cr);
  render_lines_y(graph, &c, cr);

  /* actual graph */
  render_series(graph, &c, cr);

  /* scales */
  render_scale(graph, &c, cr);
}
