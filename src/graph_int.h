#define SAMPLES_HAVE 500
#define SAMPLES_SHOW 300

enum {
  TOP = 0,
  RIGHT,
  BOTTOM,
  LEFT
};

struct sample {
  float x;
  float y;
};

struct context {
  float width;
  float height;
  
  float x_min;
  float x_max;
  float x_delta;
  float x_scale;
  float y_min;
  float y_max;
  float y_delta;
  float y_scale;
  
  float tick;
  unsigned int lines;
  unsigned int major;
};

struct Graph {
  int width;
  int height;
  int margin[4]; /* top right bottom left */
  char title[256];
  char label_x[256];
  char label_y[256];

  int rendertime;

  unsigned int write;
  struct sample samples[SAMPLES_HAVE];
};
