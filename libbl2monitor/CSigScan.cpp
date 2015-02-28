#include "stdafx.h"
#include <windows.h>
#include "CSigScan.h"

CSigScan::CSigScan(HMODULE m):
module(m)
{
	void* pAddr = module;
	MEMORY_BASIC_INFORMATION mem;

	if (VirtualQuery(pAddr, &mem, sizeof(mem)))
	{
		m_pModuleBase = (char*)mem.AllocationBase;
		if (m_pModuleBase)
		{
			IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)mem.AllocationBase;
			IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)((unsigned long)dos + (unsigned long)dos->e_lfanew);
			if (pe->Signature == IMAGE_NT_SIGNATURE)
			{
				m_moduleLen = (size_t)pe->OptionalHeader.SizeOfImage;
				isReady = true;
			}
		}
	}
}

void* CSigScan::Scan(const char* sig, const char* mask, int sigLength)
{
	if (!isReady)
		return NULL;

	char* pData = m_pModuleBase;
	char* pEnd = m_pModuleBase + m_moduleLen;

	while (pData < pEnd)
	{
		int i;
		for (i = 0; i < sigLength; i++)
		{
			if (mask[i] != '?' && sig[i] != pData[i])
				break;
		}

		if (i == sigLength){
			return (void*)pData;
		}
		pData++;
	}
	return NULL;
}

void* CSigScan::Scan(const char* sig, const char* mask)
{
	int sigLength = strlen(mask);
	return Scan(sig, mask, sigLength);
}

void* CSigScan::Scan(const MemorySignature& sigStruct)
{
	return Scan(sigStruct.Sig, sigStruct.Mask, sigStruct.Length);
}
