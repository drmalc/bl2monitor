#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <ctime>
#include <stdio.h>
#include <stdarg.h>

#define USEFILE 0 //log to file, not to pipe. Deprecated, use server-side setting instead.

class Log
{
private:
#if USEFILE
	static const char filePath[];
	static std::ofstream logFile;
#else
	static const char Log::pipeName[];
	static HANDLE hpipe;
	static bool pipeIsOpen;
#endif
	static bool logDebug;

	static void openFile(){
#if USEFILE
		if (!logFile.is_open()){
			logFile.open(filePath, std::ofstream::out);
			logFile.clear();
			logFile << "-- Session started --" << std::endl;
		}
#else
		if (pipeIsOpen)
			return;
		hpipe = CreateFileA(pipeName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		pipeIsOpen = true;
		DWORD bytesWritten;
		WriteFile(hpipe, "--- Session started ---\n", 24, &bytesWritten, NULL);
#endif
	}

	static void closeFile(){
#if USEFILE
		if (logFile.is_open()){
			logFile.close();
		}
#else
		if (pipeIsOpen)
			CloseHandle(hpipe);
		pipeIsOpen = false;
#endif
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

#if USEFILE
		(void)bsize;
		logfile << buffer;
		logFile.flush();
#else
		DWORD bytesWritten;
		WriteFile(hpipe, buffer2, bsize, &bytesWritten, NULL);
#endif
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
