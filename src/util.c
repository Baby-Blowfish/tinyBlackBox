#include "util.h"



// 안전한 free
void safe_free(void **ptr)
{
  if (ptr && *ptr)
  {
    free(*ptr);
    *ptr = NULL;
  }
}