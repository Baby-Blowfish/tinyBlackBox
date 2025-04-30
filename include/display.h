#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "console_color.h"
#include "fbDraw.h"

  bool display_init(dev_fb *fb);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_H
