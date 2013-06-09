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
void write_proxy_to_batch_file(FILE* file, const char* protocol, const char* proxy)
{
  wchar_t* protocolW;
  wchar_t* proxyW;
  int size;

  if (!proxy)
  {
    wprintf(L"%6s -> no proxy\n", protocol);
    fwprintf(file, L"set %s_proxy=\n", protocol);
    return;
  }

  size = MultiByteToWideChar(CP_UTF8, 0, proxy, -1, 0, 0);
  proxyW = (wchar_t*)malloc(size * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, proxy, -1, proxyW, size);

  size = MultiByteToWideChar(CP_UTF8, 0, protocol, -1, 0, 0);
  protocolW = (wchar_t*)malloc(size * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, protocol, -1, protocolW, size);

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

  for (i = 0;; i += 2)
  {
    if (!(*config)[i]) break;
    write_proxy_to_batch_file(file, (*config)[i], (*config)[i + 1]);
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
void detect_proxy(HINTERNET http_session, const wchar_t* test_url, char** proxy_url, const wchar_t* protocol)
{
  wchar_t*                  firstProxy        = 0;
  wchar_t*                  nextProxy         = 0;
  WINHTTP_PROXY_INFO        proxyInfo;
  WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
  int                       convertedSize     = 0;
  wchar_t*                  proxyUrlW          = 0;

  ZeroMemory(&proxyInfo, sizeof(proxyInfo));
  ZeroMemory(&autoProxyOptions, sizeof(autoProxyOptions));

  autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
  autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
  autoProxyOptions.fAutoLogonIfChallenged = TRUE;

  if(!WinHttpGetProxyForUrl(http_session, test_url, &autoProxyOptions, &proxyInfo))
  {
    *proxy_url = 0;   // No proxy detected
    return;
  }

  firstProxy = wcstok_s(proxyInfo.lpszProxy, L";", &nextProxy);
  proxyUrlW = (wchar_t*)malloc((wcslen(firstProxy) + 10) * sizeof(wchar_t));
  wsprintf(proxyUrlW, L"%s://%s", protocol, firstProxy); 

  convertedSize = WideCharToMultiByte(CP_UTF8, 0, proxyUrlW, -1, 0, 0, 0, 0);
  *proxy_url = (char*)malloc(convertedSize * sizeof(char));
  WideCharToMultiByte(CP_UTF8, 0, proxyUrlW, -1, *proxy_url, convertedSize, 0, 0);
}

/**
 * Automatically detect proxies for the various protocols
 */
int detect_proxy_config(const proxy_config* config)
{
  HINTERNET httpSession = NULL;

  httpSession = WinHttpOpen(L"autoproxy/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if(!httpSession)
  {
    fprintf(stderr, "Failed to open HTTP session.\n");
    return 1;
  }

  detect_proxy(httpSession, L"http://www.google.com/",  &(*config)[1], L"http");
  detect_proxy(httpSession, L"https://www.google.com/", &(*config)[3], L"http");
  detect_proxy(httpSession, L"ftp://ftp.mozilla.org/",  &(*config)[5], L"ftp");

  WinHttpCloseHandle(httpSession);
  return 0;
}
