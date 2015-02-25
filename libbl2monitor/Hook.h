#pragma once

//Used as a replacement for Detour.
class Hook
{
private:
	unsigned char origCode[5];
	unsigned char newCode[5];
	unsigned char *origAddress;
	void* copyOfOrig;

public:
	Hook(void* origFunction, void* newFunction);
	void Patch();
	void Unpatch();
	void* OrignalAddress(){ return (void*)origAddress; }
};
