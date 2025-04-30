#include "display.h"


bool display_init(dev_fb *fb)
{
  fb =(dev_fb*) malloc(sizeof(dev_fb));
  if (fb == NULL)
  {
    fprintf(stderr, "Failed to allocate memory for framebuffer device\n");
    goto FAIL;
  }
  if (fb_init(fb) != 0)
  {
    fprintf(stderr, "Failed to initialize framebuffer device\n");
    goto FAIL;
  }

  return true;

FAIL:
  free(fb);
  return false;
}
