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
void write_config_entry(const proxy_config* config, const network_protocol protocol, FILE* file)
{
  if (!(*config)[protocol])
  {
    printf("%6s -> no proxy\n", TARGET_PROTOCOLS[protocol]);
    fprintf(file, "export %s_proxy=\n", TARGET_PROTOCOLS[protocol]);
    return;
  }

  printf("%6s -> %s\n", TARGET_PROTOCOLS[protocol], (*config)[protocol]);
  fprintf(file, "export %s_proxy=%s\n", TARGET_PROTOCOLS[protocol], (*config)[protocol]);
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
int identify_proxy(const proxy_config* config, const network_protocol protocol, const char* proxy_to_check)
{
  char curlCommand[1024];
  int i = 0;

  sprintf(curlCommand, "curl --max-time 1 -s %s --proxy %s://%s > /tmp/autoproxy-output", PROXY_TEST_URLS[protocol], PROXY_PROTOCOLS[protocol], proxy_to_check);
  if (system(curlCommand)) return 1;

  (*config)[protocol] = malloc(sizeof(char) * (mbstowcs(0, proxy_to_check, 0) + 10));
  sprintf((*config)[protocol], "%s://%s", PROXY_PROTOCOLS[protocol], proxy_to_check);

  // Since the proxy worked for one protocol, we check it also for the others
  for (i = protocol + 1; i < PROXY_ARRAY_LEN; i++)
  {
    identify_proxy(config, i, proxy_to_check);
  } 

  return 0;
}

/**
 * Interpret the WPAD file in the temp directory
 */
int interpret_wpad(const proxy_config* config, const network_protocol protocol)
{
  FILE*      wpad        = 0;
  char       line[1024];
  regex_t    regex;
  int        regexResult = 0;
  regmatch_t match[2];
  char       proxy[1024];
  int        proxySize   = 0;

  if ((*config)[protocol]) return 0;   // Proxy already defined

  wpad = fopen("/tmp/autoproxy-wpad", "r");
  if (!wpad)
  {
    fprintf(stderr, "Error: Cannot open WPAD file.\n");
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

      identify_proxy(config, protocol, proxy);
      if ((*config)[protocol]) break;   // Proxy detected
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
 * Checks if a direct connection (without proxy) is possible
 *
 * @return 1 if direct connection possible, otherwise 0
 */
int direct_connection_possible(const network_protocol protocol)
{
  char curlCommand[1024];

  sprintf(curlCommand, "curl --proxy \"\" --max-time 1 -s %s > /tmp/autoproxy-output", PROXY_TEST_URLS[protocol]);
  return !system(curlCommand);
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

  result = system("curl --proxy \"\" --max-time 1 -s http://wpad/wpad.dat > /tmp/autoproxy-wpad");
  if (result) return 0;   // Direct connection

  for (i = 0; i < PROXY_ARRAY_LEN; i++)
  {
    if (direct_connection_possible(i)) continue;
    if (interpret_wpad(config, i)) return 1;
  } 

  return 0;
}
