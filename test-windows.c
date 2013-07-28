/**
 * @file
 * @copyright (c) 2013 Stephan Brenner
 * @license   This project is released under the MIT License.
 *
 * This file defines some tests for the Windows platform - currently mainly 
 * checking for memory leaks.
 */

#include <crtdbg.h>
#include "logic.h"

int main()
{
  proxy_config config;
  if (detect_proxy_config(&config)) return 1;
  if (write_proxy_config(&config)) return 2;
  free_proxy_config(&config);

  _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
  _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
  _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
  _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
  _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );

  return _CrtDumpMemoryLeaks();
}
