#define SAMPLES 100

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

struct Graph {
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
