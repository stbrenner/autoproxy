/**
 * @file
 * @copyright (c) 2013 Stephan Brenner
 * @license   This project is released under the MIT License.
 *
 * This is the main file of the project containing the entry point.
 */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logic.h"

/**
 * Entry point for the executable
 */
int main()
{
  proxy_config config;

#ifdef __linux__
  if (!setlocale(LC_ALL, "en_US.UTF-8"))
  {
    fprintf(stderr, "Error: Unable to configure UTF-8.\n");
    return 3;   
  }
#endif

  printf("Detecting proxy configuration...\n");
  new_proxy_config(&config);

  if (detect_proxy_config(&config))
  {
    fprintf(stderr, "Error: Unable to detect proxy configuration.\n");
    return 1;   
  }

  if (write_proxy_config(&config))
  {
    fprintf(stderr, "Error: Unable to write batch file.\n");
    return 2;   
  }
  
  free_proxy_config(&config);
  return 0;
}
