#include "stdafx.h"
#include "CHookManager.h"
#include "Log.h"

void CHookManager::AddVirtualHook(const std::string& funcName, const tFuncNameHookPair& hookPair)
{
	tiVirtualHooks iHooks = VirtualHooks.find(funcName);
	if (iHooks != VirtualHooks.end())
	{
		// Otherwise it's fine, add it into the existing table
		iHooks->second.insert(hookPair);
	}
	else
	{
		// There are no other virtual hooks so we need to create the table for it
		tHookMap newMap;
		newMap.insert(hookPair);
		VirtualHooks.emplace(funcName, newMap);
	}

	Log::info("(%s) Hook \"%s\" added as virtual hook for \"%s\".", this->DebugName.c_str(), hookPair.first.c_str(), funcName.c_str());
}

void CHookManager::AddStaticHook(UFunction* function, const tFuncNameHookPair& hookPair)
{
	tiStaticHooks iHooks = StaticHooks.find(function);
	if (iHooks != StaticHooks.end())
	{
		// Otherwise it's fine, add it into the existing table
		iHooks->second.insert(hookPair);
	}
	else
	{
		// There are no other hooks so we need to create the table for it
		tHookMap newMap;
		newMap.insert(hookPair);
		StaticHooks.emplace(function, newMap);
	}

	Log::info("(%s) Hook \"%s\" added as static hook for \"%s\".", this->DebugName.c_str(), hookPair.first.c_str(), function->GetFullName().c_str());
}

bool CHookManager::RemoveFromTable(tHookMap& hookTable, const std::string& funcName, const std::string& hookName)
{
	// Remove it and ensure that it actually got removed
	int removed = hookTable.erase(hookName);

	if (removed == 0)
	{
		Log::warning("(%s) Failed to remove hook \"%s\" for function \"%s\".", this->DebugName.c_str(), hookName.c_str(), funcName.c_str());
		return false;
	}
	else
	{
		Log::info("(%s) Hook \"%s\" removed for function \"%s\" successfully.", this->DebugName.c_str(), hookName.c_str(), funcName.c_str());
		return true;
	}
}

void CHookManager::Register(const std::string& funcName, const std::string& hookName, void* funcHook)
{
	// Create pair to insert
	tFuncNameHookPair hookPair = std::make_pair(hookName, funcHook);

	// Find func
	UFunction* function = UObject::FindObject<UFunction>(funcName);
	if (function == nullptr)
	{
		// The function was not found, so we need to create a virtual hook for it
		AddVirtualHook(funcName, hookPair);
	}
	else
	{
		// The function WAS found, so we can just hook it straight away
		AddStaticHook(function, hookPair);
	}
}

bool CHookManager::Remove(const std::string& funcName, const std::string& hookName)
{
	if (!RemoveVirtualHook(funcName, hookName)){
		Log::warning("(%s) Failed to remove virtual hook \"%s\" for \"%s\". Attempting to remove static hook instead.", this->DebugName.c_str(), hookName.c_str(), funcName);
		UFunction* function = UObject::FindObject<UFunction>(funcName);
		if (function != nullptr)
			return RemoveStaticHook(function, hookName);
	}
	else{
		return true;
	}
	return false;
}

bool CHookManager::RemoveVirtualHook(const std::string& funcName, const std::string& hookName)
{
	tiVirtualHooks iHooks = VirtualHooks.find(funcName);
	if (iHooks == VirtualHooks.end())
	{
		return false;
	}

	return RemoveFromTable(iHooks->second, funcName, hookName);
}

bool CHookManager::RemoveStaticHook(UFunction* function, const std::string& hookName)
{
	// Since we are getting a UFunction pointer, we don't need to check virtual hooks
	tiStaticHooks iHooks = StaticHooks.find(function);
	if (iHooks == StaticHooks.end())
	{
		Log::error("(%s) Failed to remove static hook \"%s\" for \"%s\".", this->DebugName.c_str(), hookName.c_str(), function->GetFullName().c_str());
		return false;
	}

	return RemoveFromTable(iHooks->second, function->GetFullName(), hookName);
}

void CHookManager::ResolveVirtualHooks(UFunction* function)
{
	// Resolve any virtual hooks into static hooks
	if (VirtualHooks.size() > 0)
	{
		//std::string funcName = GetFuncName(pFunction); TODO: Use this instead of the ugly other thing
		std::string funcName = function->GetFullName();

		tiVirtualHooks iVHooks = VirtualHooks.find(funcName);
		if (iVHooks != VirtualHooks.end())
		{
			// Insert this map into the static hooks map
			int size = iVHooks->second.size();
			StaticHooks.emplace(function, iVHooks->second);
			VirtualHooks.erase(iVHooks);

			Log::info("(%s) Function pointer found for \"%s\", added map with %i elements to static hooks map.", this->DebugName.c_str(), funcName.c_str(), size);
		}
	}
}
