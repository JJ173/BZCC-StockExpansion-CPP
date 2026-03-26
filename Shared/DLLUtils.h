#ifndef DLLUTILS_H__
#define DLLUTILS_H__
#include <string>

namespace dll_utils
{
	bool is_recycler(const Handle handle);
	
	int count_allied_players(int team);
	
	std::string get_checked_network_svar(size_t svar, NETWORK_LIST_TYPE list_type);
}

#endif
