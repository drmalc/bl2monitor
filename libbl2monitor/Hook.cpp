#include "stdafx.h"
#include "Hook.h"

#define PATCH_SIZE	5

Hook::Hook(void* origFunction, void* newFunction)
{
	DWORD	offset = (DWORD)newFunction - (DWORD)origFunction - PATCH_SIZE;

	//Create patch
	newCode[0] = 0xE9;
	*((DWORD*)&newCode[1]) = offset;

	//Allow memory overwriting
	origAddress = (unsigned char*)origFunction;
	DWORD	dwOldProtect;
	VirtualProtect(origAddress, PATCH_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	//Save orignal code
	for (int i = 0; i < PATCH_SIZE; i++)
		origCode[i] = *(origAddress + i);
}

void Hook::Patch()
{
	memcpy(origAddress, newCode, PATCH_SIZE);
}

void Hook::Unpatch()
{
	memcpy(origAddress, origCode, PATCH_SIZE);
}
