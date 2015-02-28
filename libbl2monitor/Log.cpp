#include "stdafx.h"
#include "Log.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

NamedPipe *Log::pipe = NULL;
const char Log::pipeName[] = "\\\\.\\pipe\\bl2monitorpipe";
bool Log::logDebug = false;
