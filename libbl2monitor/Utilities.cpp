#include "stdafx.h"
#include "Utilities.h"
#include "NamedPipe.h"
#include "Log.h"


namespace Utilities
{
	static bool isInit = false;
	static NamedPipe *pipe = NULL;
	static const char pipeName[] = "\\\\.\\pipe\\bl2monitorpipeutils";
	static char serverPath[MAX_PATH] = { 0 };
	static char mainLuaPath[MAX_PATH] = { 0 };
	static char imagesPath[MAX_PATH] = { 0 }; //deprecated
	static char layoutPath[MAX_PATH] = { 0 };

	void SendRequest(const char*req, unsigned len, char* buffer, unsigned *buflen)
	{//Clean me up
		if (!isInit || !buffer)
			return;
		pipe->Flush();
		if (pipe->Write(req, len) != len)
		{
			*buflen = 0;
			*buffer = 0;
			Log::error("Failed to send request.");
			return;
		}
		*buffer = 0;
		unsigned total = 0;
		bool endCharFound = false;
		for (unsigned i = 0; 3 > i; i++){
			total += pipe->ReadImmediate(buffer + total, *buflen - total);
			for (unsigned j = 0; total > j; j++)
			{
				if (buffer[j] == '\n'){
					endCharFound = true;
					buffer[j] = 0;
					break;
				}
			}
			if (endCharFound)
				break;
		}
		if (endCharFound)
			*buflen = total;
		else{
			*buflen = 0;
			*buffer = 0;
		}
	}

	void Initialize()
	{
		if (isInit)
			return;
		isInit = true;

		pipe = new NamedPipe(pipeName);
		pipe->Open();
	}

	void CleanUp()
	{
		if (pipe)
		{
			pipe->Close();
			delete pipe;
			pipe = NULL;
		}
	}

	const char *ServerPath()
	{
		const char req[] = "PATH\n";
		if (*serverPath)
			return serverPath;
		unsigned l = sizeof(serverPath);
		SendRequest(req, sizeof(req)-1, serverPath, &l);
		return serverPath;
	}

	const char *MainLuaPath()
	{
		const char req[] = "LUAMAIN\n";
		if (*mainLuaPath)
			return mainLuaPath;
		unsigned l = sizeof(mainLuaPath);
		SendRequest(req, sizeof(req)-1, mainLuaPath, &l);
		return mainLuaPath;
	}

	const char *ImagesPath()
	{
		const char req[] = "IMAGES\n";
		if (*imagesPath)
			return imagesPath;
		unsigned l = sizeof(imagesPath);
		SendRequest(req, sizeof(req) - 1, imagesPath, &l);
		return imagesPath;
	}

	const char *LayoutPath()
	{
		const char req[] = "LAYOUT\n";
		if (*layoutPath)
			return layoutPath;
		unsigned l = sizeof(layoutPath);
		SendRequest(req, sizeof(req) - 1, layoutPath, &l);
		return layoutPath;
	}
}
