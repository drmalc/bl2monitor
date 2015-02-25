#ifndef CSIGSCAN_H
#define CSIGSCAN_H

#include <windows.h>

struct MemorySignature
{
	const char* Sig;
	const char* Mask;
	int Length;
};

class CSigScan
{
private:
	HMODULE module;
	bool isReady = false;
	char* m_pModuleBase;
	size_t m_moduleLen;

public:
	CSigScan(HMODULE m);
	void* CSigScan::Scan(const char* sig, const char* mask, int sigLength);
	void* CSigScan::Scan(const char* sig, const char* mask);
	void* CSigScan::Scan(const MemorySignature& sigStruct);
};

#endif
