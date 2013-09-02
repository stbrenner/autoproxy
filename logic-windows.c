/**
 * @file
 * @copyright (c) 2013 Stephan Brenner
 * @license   This project is released under the MIT License.
 *
 * This file contains Windows specific implemenation of the autoproxy logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Winhttp.h>
#include "logic.h"

/**
 * Writes a single proxy setting to the batch file
 */
void write_proxy_to_batch_file(FILE* file, const proxy_config* config, const network_protocol protocol)
{
  wchar_t* protocolW = 0;
  wchar_t* proxyW    = 0;
  int      size      = 0;

  if (!(*config)[protocol])
  {
    printf("%6s -> no proxy\n", TARGET_PROTOCOLS[protocol]);
    fprintf(file, "set %s_proxy=\n", TARGET_PROTOCOLS[protocol]);
    return;
  }

  size = MultiByteToWideChar(CP_UTF8, 0, (*config)[protocol], -1, 0, 0);
  proxyW = (wchar_t*)malloc(size * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, (*config)[protocol], -1, proxyW, size);

  size = MultiByteToWideChar(CP_UTF8, 0, TARGET_PROTOCOLS[protocol], -1, 0, 0);
  protocolW = (wchar_t*)malloc(size * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, TARGET_PROTOCOLS[protocol], -1, protocolW, size);

  wprintf(L"%6s -> %s\n", protocolW, proxyW);
  fwprintf(file, L"set %s_proxy=%s\n", protocolW, proxyW);

  free(protocolW);
  free(proxyW);
}

/**
 * Write a batch file containing the statements to set the proxy environment variables.
 */
int write_proxy_config(const proxy_config* config)
{
  int     i    = 0;
  FILE*   file = NULL;
  wchar_t filePath[1024];
  wchar_t tempDir[1024];
  size_t  requiredSize;
  
  _wgetenv_s(&requiredSize, tempDir, 1024, L"TEMP");

  wsprintf(filePath, L"%s\\autoproxy-script.cmd", tempDir);
  if (_wfopen_s(&file, filePath, L"w"))
  {
    fprintf(stderr, "Failed to open file.\n");
    return 1;
  }

  wprintf(L"Proxy configuration successfully detected:\n");
  fwprintf(file, L"@echo off\n");

  for (i = 0; i < PROXY_ARRAY_LEN; i++)
  {
    write_proxy_to_batch_file(file, config, (network_protocol)i);
  }

  if (fclose(file))
  {
    fprintf(stderr, "Failed to close file.\n");
    return 2;
  }

  return 0;
}

/**
 * Detect the proxy for a specific protocol
 */
void detect_proxy(HINTERNET http_session, proxy_config* config, network_protocol protocol)
{
  wchar_t*                  firstProxy        = 0;
  wchar_t*                  nextProxy         = 0;
  WINHTTP_PROXY_INFO        proxyInfo;
  WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
  int                       size              = 0;
  wchar_t*                  proxyUrlW         = 0;
  wchar_t*                  testUrlW          = 0;
  wchar_t*                  protocolW         = 0;

  ZeroMemory(&proxyInfo, sizeof(proxyInfo));
  ZeroMemory(&autoProxyOptions, sizeof(autoProxyOptions));

  autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
  autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
  autoProxyOptions.fAutoLogonIfChallenged = TRUE;

  size = MultiByteToWideChar(CP_UTF8, 0, PROXY_TEST_URLS[protocol], -1, 0, 0);
  testUrlW = (wchar_t*)malloc(size * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, PROXY_TEST_URLS[protocol], -1, testUrlW, size);

  if(!WinHttpGetProxyForUrl(http_session, testUrlW, &autoProxyOptions, &proxyInfo) || 
     !proxyInfo.lpszProxy)
  {
    free(testUrlW);
    return;   // No proxy detected
  }

  size = MultiByteToWideChar(CP_UTF8, 0, PROXY_PROTOCOLS[protocol], -1, 0, 0);
  protocolW = (wchar_t*)malloc(size * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, PROXY_PROTOCOLS[protocol], -1, protocolW, size);

  firstProxy = wcstok_s(proxyInfo.lpszProxy, L";", &nextProxy);
  proxyUrlW = (wchar_t*)malloc((wcslen(firstProxy) + 10) * sizeof(wchar_t));
  wsprintf(proxyUrlW, L"%s://%s", protocolW, firstProxy); 

  size = WideCharToMultiByte(CP_UTF8, 0, proxyUrlW, -1, 0, 0, 0, 0);
  (*config)[protocol] = (char*)malloc(size * sizeof(char));
  WideCharToMultiByte(CP_UTF8, 0, proxyUrlW, -1, (*config)[protocol], size, 0, 0);

  free(testUrlW);
  free(proxyUrlW);
  free(protocolW);
}

/**
 * Automatically detect proxies for the various protocols
 */
int detect_proxy_config(proxy_config* config)
{
  HINTERNET httpSession = NULL;
  int       i           = 0;

  *config = (proxy_config)malloc(PROXY_ARRAY_LEN * sizeof(char*));
  memset(*config, 0, PROXY_ARRAY_LEN * sizeof(char*));

  httpSession = WinHttpOpen(L"autoproxy/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if(!httpSession)
  {
    fprintf(stderr, "Failed to open HTTP session.\n");
    return 1;
  }

  for (i = 0; i < PROXY_ARRAY_LEN; i++)
  {
    detect_proxy(httpSession, config, (network_protocol)i);
  } 

  WinHttpCloseHandle(httpSession);
  return 0;
}