#include "..\Shared\DLLBase.h"
#include "DLLUtils.h"

bool dll_utils::is_recycler(Handle handle)
{
	char objectClass[64]{};
	if (!GetObjInfo(handle, Get_GOClass, objectClass))
	{
		return false;
	}

	static constexpr const char* recyclerClasses[] =
	{
		"CLASS_RECYCLERVEHICLE",
		"CLASS_RECYCLER",
		"CLASS_RECYCLERVEHICLEH"
	};

	for (const char* recyclerClass : recyclerClasses)
	{
		if (std::strcmp(objectClass, recyclerClass) == 0)
		{
			return true;
		}
	}

	return false;
}

int dll_utils::count_allied_players(const int team)
{
	int count = 0;
	
	for (int i = 1; i < MAX_TEAMS; ++i)
	{
		if (IsTeamAllied(i, team))
		{
			const Handle handle = GetPlayerHandle(i);
			if (handle != 0)
			{
				++count;
			}
		}
	}

	return count;
}

std::string dll_utils::get_checked_network_svar(const size_t svar, const NETWORK_LIST_TYPE list_type)
{
	static constexpr auto kSvarPrefix = "network.session.svar";
	
	const std::string svarKey = std::string(kSvarPrefix) + std::to_string(svar);
	const std::string svarValue = GetVarItemStr(svarKey.c_str());
	
	if (svarValue.empty())
	{
		return {};
	}

	const auto listCount = GetNetworkListCount(list_type);
	
	for (size_t i = 0; i < listCount; ++i)
	{
		const std::string listItem = GetNetworkListItem(list_type, i);
		if (listItem == svarValue)
		{
			return listItem;
		}
	}
	
	return {};
}
