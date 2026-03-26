#include "../Shared/DLLBase.h"
#include "DLLUtils.h"

bool dll_utils::is_recycler(const Handle handle)
{
	char object_class[64]{};
	if (!GetObjInfo(handle, Get_GOClass, object_class))
	{
		return false;
	}

	static constexpr const char* recycler_classes[] =
	{
		"CLASS_RECYCLERVEHICLE",
		"CLASS_RECYCLER",
		"CLASS_RECYCLERVEHICLEH"
	};

	for (const char* recycler_class : recycler_classes)
	{
		if (std::strcmp(object_class, recycler_class) == 0)
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
	static constexpr auto k_svar_prefix = "network.session.svar";
	
	const std::string svar_key = std::string(k_svar_prefix) + std::to_string(svar);
	const std::string svar_value = GetVarItemStr(svar_key.c_str());
	
	if (svar_value.empty())
	{
		return {};
	}

	const auto list_count = GetNetworkListCount(list_type);
	
	for (size_t i = 0; i < list_count; ++i)
	{
		const std::string list_item = GetNetworkListItem(list_type, i);
		if (list_item == svar_value)
		{
			return list_item;
		}
	}
	
	return {};
}