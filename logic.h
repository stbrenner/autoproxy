/**
 * @file
 * @copyright (c) 2013 Stephan Brenner
 * @license   This project is released under the MIT License.
 *
 * This file defines a generic interface to the internal logic. The
 * implementation of the logic itself is platform dependent.
 *
 * @note All strings in this API are UTF-8 encoded.
 */

#ifndef LOGIC_H
#define LOGIC_H

/**
 * The following constants point to the respective element in the
 * TARGET_PROTOCOLS array (and related).
 */
typedef enum {
  HTTP_PROXY,
  HTTPS_PROXY,
  FTP_PROXY
} network_protocol;

/**
 * Defines the length of the TARGET_PROTOCOLS (and all related)
 */
static const int PROXY_ARRAY_LEN = 3;

/**
 * Array containing protocols used to reach targets in the internet.
 */
static const char* TARGET_PROTOCOLS[] = {"http", "https", "ftp"};

/**
 * Array containing protocols to access the proxy.
 * The order matches the TARGET_PROTOCOLS.
 */
static const char* PROXY_PROTOCOLS[] = {"http", "http", "ftp"};

/**
 * Array containing the URLs to test aginst.
 * The order matches the TARGET_PROTOCOLS.
 */
static const char* PROXY_TEST_URLS[] = {
  "http://www.google.com/",
  "https://www.google.com/",
  "ftp://ftp.mozilla.org/"};

/**
 * Array containing the proxy configuration as strings.
 *
 * The order must match to the TARGET_PROTOCOLS.
 * 
 * @example {"http://proxy:8080", "http://proxy:8080", "ftp://proxy:8080"} 
 */
typedef char** proxy_config;

/**
 * Free the memory allocated by detect_proxy_config()
 * 
 * @param config Pointer to the proxy_config.
 */
void free_proxy_config(proxy_config* config);

/**
 * Write proxy configuration to screen and script file.
 *
 * @param config Proxy configuration to be written to screen and file. 
 * @return       0 if function succeeded, otherwise error number
 */
int write_proxy_config(const proxy_config* config);

/**
 * Automatically detect proxies for the various protocols
 * 
 * @param config Proxy configuration where the result is written to.
 *               Memory must be released with free_proxy_config().
 * @return       0 if function succeeded, otherwise error number
 */
int detect_proxy_config(proxy_config* config);

#endif // LOGIC_H
