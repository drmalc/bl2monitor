#include "stdafx.h"
#include "Log.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#if USEFILE
const char Log::filePath[] = "C:\\temp\\bl2monitor.log";
ofstream Log::logFile;
#else
bool Log::pipeIsOpen = false;
HANDLE Log::hpipe;
const char Log::pipeName[] = "\\\\.\\pipe\\bl2monitorpipe";
#endif
bool Log::logDebug = false;
