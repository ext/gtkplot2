#ifndef GTK_PLOT2_GRAPH_H
#define GTK_PLOT2_GRAPH_H

#include <cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct Graph Graph;

  /**
   * Removes all samples from the graph.
   * (all samples are reset to zero)
   */
  void graph_sample_clear(Graph* graph);

  /**
   * Add a new sample to the graph.
   * Assumes x is incremental.
   */
  void graph_sample_add(Graph* graph, float x, float y);

  /**
   * Get minimum and maximum sample (in the current range).
   * If no samples is available it return zeroes.
   */
  void graph_get_minmax(const Graph* graph, float* min, float* max);

  /**
   * Render graph onto surface.
   * @param min Lower y value.
   * @param max Upper y value.
   * @param lines Number of helper lines to render (also puts a mark on the
   *              scale indicating its value). Lines are placed with equal
   *              spacing. Use zero to disable lines.
   */
  void graph_render(const Graph* graph, cairo_t* cr, float min, float max, unsigned int lines);

#ifdef __cplusplus
}
#endif

#endif /* GTK_PLOT2_GRAPH_H */
