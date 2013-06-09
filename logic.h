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
 * Array containing the proxy configuration as strings.
 * 
 * The array contains pairs of protocol->proxy. The even array positions
 * contain protocols (e.g. "http") and the odds contain the corresponding
 * proxies (e.g. "http://proxy:8080").
 *
 * The array is null terminated. So the last element must always be null.
 * 
 * @example {"http",  "http://proxy:8080", 
 *           "https", "http://proxy:8080", 0} 
 */
typedef char** proxy_config;

/**
 * Allocates the memory for a new proxy config.
 * 
 * @param config Proxy configuration which is initialized.
 * @note         Memory must be released with free_proxy_config().
 */
void new_proxy_config(proxy_config* config);

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
int detect_proxy_config(const proxy_config* config);

#endif // LOGIC_H
