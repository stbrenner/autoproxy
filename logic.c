/**
 * @file
 * @copyright (c) 2013 Stephan Brenner
 * @license   This project is released under the MIT License.
 *
 * This file contains platform independent implementation of logic.
 */

#include <stdlib.h>
#include <string.h>
#include "logic.h"

/**
 * Duplicates a string of a given size.
 *
 * @param size Number of bytes including terminating null character
 */
char* strdup_s(const char* source, const size_t size)
{
  char* result = (char*)malloc(size);
  memcpy(result, source, size);
  return result;
}

/**
 * Allocates the memory for a new proxy config.
 */
void new_proxy_config(proxy_config* config)
{
  *config = (proxy_config)malloc(7 * sizeof(char*));
  memset(*config, 0, 7 * sizeof(char*));

  (*config)[0] = strdup_s("http",  5);
  (*config)[2] = strdup_s("https", 6);
  (*config)[4] = strdup_s("ftp",   4);
}

/**
 * Free the proxy_config array
 */
void free_proxy_config(proxy_config* config)
{
  int i = 0;

  if (!config || !(*config)) return;

  for (i = 0;; i += 2)
  {
    if (!(*config)[i]) break;
    free((*config)[i]);
    free((*config)[i + 1]);
  }

  free(*config);
  *config = 0;
}
