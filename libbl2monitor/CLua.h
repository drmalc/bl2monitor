#pragma once

#define FFI_EXPORT extern "C" __declspec(dllexport)

namespace CLua
{
	void Initialize();
}

