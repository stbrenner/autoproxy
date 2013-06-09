/**
 * @file
 * @copyright (c) 2013 Stephan Brenner
 * @license   This project is released under the MIT License.
 *
 * This file contains Linux specific implemenation of the autoproxy logic.
 */

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logic.h"

/**
 * Write a single config entry to screen and into the shell file
 */
void write_config_entry(const char* proxy, const char* protocol, FILE* file)
{
  if (!proxy)
  {
    printf("%6s -> no proxy\n", protocol);
    fprintf(file, "export %s_proxy=\n", protocol);
    return;
  }

  printf("%6s -> %s\n", protocol, proxy);
  fprintf(file, "export %s_proxy=%s\n", protocol, proxy);
}

/**
 * Write a batch file containing the statements to set the proxy environment variables.
 */
int write_proxy_config(const proxy_config* config)
{
  FILE* file = NULL;
  int i = 0;

  file = fopen("/tmp/autoproxy-script", "w");
  if (!file)
  {
    fprintf(stderr, "Failed to open file.\n");
    return 1;
  }

  printf("Proxy configuration successfully detected:\n");
  fprintf(file, "#!/bin/bash\n");

  for (i = 0;; i += 2)
  {
    if (!(*config)[i]) break;
    write_config_entry((*config)[i + 1], (*config)[i],  file);
  }

  if (fclose(file))
  {
    fprintf(stderr, "Failed to close file.\n");
    return 2;
  }

  return 0;
}

/**
 * Check if an connection through the given proxy is possible. An if yes, then
 * store the proxy in the config.
 */
int identify_proxy(const proxy_config* config, const int pos, const char* proxy_to_check, const char* test_url, const char* protocol)
{
  char curlCommand[1024];

  if ((*config)[pos]) return 0;   // Proxy for this protocoal already set

  sprintf(curlCommand, "curl --max-time 1 -s %s --proxy %s://%s > /tmp  if ((*config)[pos]) return;   // Proxy for this protocoal already set/autoproxy-output", test_url, protocol, proxy_to_check);
  if (system(curlCommand)) return 1;

  (*config)[pos] = malloc(sizeof(char) * (mbstowcs(0, proxy_to_check, 0) + 10));
  sprintf((*config)[pos], "%s://%s", protocol, proxy_to_check);

  return 0;
}

/**
 * Interpret the WPAD file in the temp directory
 */
int interpret_wpad(const proxy_config* config)
{
  FILE*      wpad        = 0;
  char       line[1024];
  regex_t    regex;
  int        regexResult = 0;
  regmatch_t match[2];
  char       proxy[1024];
  int        proxySize   = 0;

  wpad = fopen("/tmp/autoproxy-wpad", "r");
  if (!wpad)
  {
    fprintf(stderr, "Error: Cannot open WPAD file.");
    return 1;
  }

  regexResult = regcomp(&regex, "PROXY[:space:]*([^;\"']+)", REG_EXTENDED);
  if (regexResult)
  {
    fprintf(stderr, "Error: Regular expression cannot be compiled (%d).\n", regexResult);
    return 2;
  }

  while (fgets(line, 1024, wpad))
  {
    regexResult = regexec(&regex, line, 2, match, 0);
    if (!regexResult)
    {
      proxySize = match[1].rm_eo - match[1].rm_so - 1;
      memcpy(proxy, line + match[1].rm_so + 1, proxySize);
      *(proxy + proxySize) = 0;

      identify_proxy(config, 1, proxy, "http://www.google.com/",  "http");
      identify_proxy(config, 3, proxy, "https://www.google.com/", "http");
      identify_proxy(config, 5, proxy, "ftp://ftp.mozilla.org/",  "ftp");
    }
    else if (REG_NOMATCH != regexResult)
    {
      fprintf(stderr, "Error: Regular expression cannot be executed (%d).\n", regexResult);
      return 3;
    }
  }

  regfree(&regex);
  fclose(wpad);
  return 0;
}

/**
 * Automatically detect proxies for the various protocols
 */
int detect_proxy_config(const proxy_config* config)
{
  int result = 0;

  result = system("curl --proxy \"\" -s http://wpad/wpad.dat > /tmp/autoproxy-wpad");
  if (1536 == result) return 0;   // Direct connection

  if (interpret_wpad(config)) return 1;

  return 0;
}
