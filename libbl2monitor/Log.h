#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <ctime>
#include <stdio.h>
#include <stdarg.h>
#include "NamedPipe.h"

class Log
{
private:
	static NamedPipe *pipe;
	static bool logDebug;
	static const char pipeName[];

	static void openFile()
	{
		if (!pipe)
			pipe = new NamedPipe(pipeName);
		if (pipe->IsOpen())
			return;
		pipe->Open();
		pipe->Write("--- Session started ---\n", 24);
	}

	static void closeFile(){
		if (pipe)
		{
			pipe->Close();
			delete pipe;
			pipe = NULL;
		}
	}

	static void printText(const char* label, const char* text, va_list args)
	{
		openFile();

		char buffer[1000] = { 0 };
		int bsize = vsprintf_s(buffer, sizeof(buffer), text, args);

		struct tm now;
		__time64_t long_time;
		_time64(&long_time);
		_localtime64_s(&now, &long_time);

		char buffer2[1000] = { 0 };
		bsize = sprintf_s(buffer2, sizeof(buffer2), "%02d:%02d:%02d %s %s\n", now.tm_hour, now.tm_min, now.tm_sec, label, buffer);

		pipe->Write(buffer2, bsize);
	}

public:
	static void debug(const char* text, ...){
		if (logDebug)
		{
			va_list args;
			va_start(args, text);
			printText("DEBG |", text, args);
			va_end(args);
		}
	}

	static void info(const char* text, ...){
		va_list args;
		va_start(args, text);
		printText("INFO |", text, args);
		va_end(args);
	}

	static void error(const char* text, ...){
		va_list args;
		va_start(args, text);
		printText("ERROR|", text, args);
		va_end(args);
	}

	static void warning(const char* text, ...){
		va_list args;
		va_start(args, text);
		printText("WARN |", text, args);
		va_end(args);
	}

	static void lua(const char* text, ...){
		va_list args;
		va_start(args, text);
		printText("LUA  |", text, args);
		va_end(args);
	}

	static void setLogDebug(bool b)
	{
		logDebug = b;
	}
};

#endif
