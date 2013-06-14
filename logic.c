/**
 * @file
 * @copyright (c) 2013 Stephan Brenner
 * @license   This project is released under the MIT License.
 *
 * This file contains platform independent implementation of logic.
 */

#include <stdlib.h>
#include "logic.h"

/**
 * Free the proxy_config array
 */
void free_proxy_config(proxy_config* config)
{
  int i = 0;

  if (!config || !(*config)) return;

  for (i = 0; i < PROXY_ARRAY_LEN; i++)
  {
    free((*config)[i]);
  }

  free(*config);
  *config = 0;
}
