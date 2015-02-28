#ifndef SIGNATURES_H
#define SIGNATURES_H

#include "CSigScan.h"

namespace UHook
{
	namespace Signatures
	{
		MemorySignature GObjects = { 
			"\x00\x00\x00\x00\x8B\x04\xB1\x8B\x40\x08",
			"????xxxxxx",
			10
		};

		MemorySignature GNames = { 
			"\x00\x00\x00\x00\x83\x3C\x81\x00\x74\x5C",
			"????xxxxxx",
			10
		};

		MemorySignature ProcessEvent = {
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x50\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\xF1\x89\x75\xEC",
			"xxxxxx????xx????xxxxx????xxxxxxxxxxxxxx????xxxxx",
			48
		};

		MemorySignature CrashHandler = {
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x8B\x7D\x0C",
			"xxxxxx????xx????xxx????x????xxxxxxxxxxxxxx????xx?????xxx",
			56
		};

		MemorySignature GObjHash = {
			"\x00\x00\x00\x00\x8B\x4E\x30\x8B\x46\x2C\x8B\x7E\x28",
			"????xxxxxxxxx",
			13
		};

		MemorySignature CallFunction = {
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\x7D\x10\x8B\x45\x0C",
			"xxxxxx????xx????xxx????x????xxxxxxxxxxxxxx????xxxxxx",
			52
		};

		MemorySignature FrameStep = {
			"\x55\x8B\xEC\x8B\x41\x18\x0F\xB6\x10",
			"xxxxxxxxx",
			9
		};

		MemorySignature GCRCTable = {
			"\x00\x00\x00\x00\x8D\x64\x24\x00\x8B\xD6\xC1\xE2\x18",
			"????xxxxxxxxx",
			13
		};

		MemorySignature NameHash = {
			"\x00\x00\x00\x00\x85\xF6\x74\x56\xFF\x45\xFC\x85\xF6\x74\x4F",
			"????xxxxxxxxxxx",
			15
		};

		MemorySignature StaticConstructor = {
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x10\x53\x56\x57\xA1\x00\x00\x00\x00\x33\xC5\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\x7D\x08\x8A\x87",
			"xxxxxx????xx????xxxxxxxx????xxxxxxxx????xxxxx",
			45
		};

		MemorySignature LoadPackage = {
			"\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x68\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xEC",
			"xxxxxx????xx????xxxxx????xxxxx",
			30
		};

		MemorySignature ByteOrderSerialize = {
			"\x55\x8B\xEC\x56\x8B\xF1\x83\x7E\x4C\x00\x74\x26",
			"xxxxxxxxxxxx",
			12
		};

		MemorySignature TextureFixLocation = {
			"\x8B\x46\x10\x85\xC0\x0F\x84\x00\x00\x00\x00\x8B\x16",
			"xxxxxxx????xx",
			13
		};

		MemorySignature GMalloc = {
			"\x00\x00\x00\x00\xFF\xD7\x83\xC4\x04\x89\x45\xE4",
			"????xxxxxxxx",
			12
		};
	}
}

#endif