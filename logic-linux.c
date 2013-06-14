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
void write_config_entry(const proxy_config* config, const int pos, FILE* file)
{
  if (!(*config)[pos])
  {
    printf("%6s -> no proxy\n", TARGET_PROTOCOLS[pos]);
    fprintf(file, "export %s_proxy=\n", TARGET_PROTOCOLS[pos]);
    return;
  }

  printf("%6s -> %s\n", TARGET_PROTOCOLS[pos], (*config)[pos]);
  fprintf(file, "export %s_proxy=%s\n", TARGET_PROTOCOLS[pos], (*config)[pos]);
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
    fprintf(stderr, "Failed to open autoproxy-script.\n");
    return 1;
  }

  printf("Proxy configuration successfully detected:\n");
  fprintf(file, "#!/bin/bash\n");

  for (i = 0; i < PROXY_ARRAY_LEN; i++)
  {
    write_config_entry(config, i, file);
  }

  if (fclose(file))
  {
    fprintf(stderr, "Failed to close autoproxy-script.\n");
    return 2;
  }

  return 0;
}

/**
 * Check if an connection through the given proxy is possible. An if yes, then
 * store the proxy in the config.
 */
int identify_proxy(const proxy_config* config, const int pos, const char* proxy_to_check)
{
  char curlCommand[1024];
  int i = 0;

  sprintf(curlCommand, "curl --max-time 1 -s %s --proxy %s://%s > /tmp/autoproxy-output", PROXY_TEST_URLS[pos], PROXY_PROTOCOLS[pos], proxy_to_check);
  if (system(curlCommand)) return 1;

  (*config)[pos] = malloc(sizeof(char) * (mbstowcs(0, proxy_to_check, 0) + 10));
  sprintf((*config)[pos], "%s://%s", PROXY_PROTOCOLS[pos], proxy_to_check);

  // Since the proxy worked for one protocol, we check it also for the others
  for (i = pos + 1; i < PROXY_ARRAY_LEN; i++)
  {
    identify_proxy(config, i, proxy_to_check);
  } 

  return 0;
}

/**
 * Interpret the WPAD file in the temp directory
 */
int interpret_wpad(const proxy_config* config, const int pos)
{
  FILE*      wpad        = 0;
  char       line[1024];
  regex_t    regex;
  int        regexResult = 0;
  regmatch_t match[2];
  char       proxy[1024];
  int        proxySize   = 0;

  if ((*config)[pos]) return 0;   // Proxy already defined

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

      identify_proxy(config, pos, proxy);
      if ((*config)[pos]) break;   // Proxy detected
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
int detect_proxy_config(proxy_config* config)
{
  int result = 0;
  int i = 0;

  *config = (proxy_config)malloc(PROXY_ARRAY_LEN * sizeof(char*));
  memset(*config, 0, PROXY_ARRAY_LEN * sizeof(char*));

  result = system("curl --proxy \"\" -s http://wpad/wpad.dat > /tmp/autoproxy-wpad");
  if (1536 == result) return 0;   // Direct connection

  for (i = 0; i < PROXY_ARRAY_LEN; i++)
  {
    if (interpret_wpad(config, i)) return 1;
  } 

  return 0;
}
