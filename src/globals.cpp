/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source {project}
 * Written by {name of author} ({fullname}).
 * ======================================================

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iserver.h>
#include <tier0/dbg.h>

class CGlobalVars;
class CGameEntitySystem;

// AMNOTE: Should only be called within the active game loop (i e map should be loaded and active) 
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

	if(!server)
	{
		AssertMsg(server, "Server is not ready");

		return nullptr;
	}

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

CGameEntitySystem *GameEntitySystem()
{
	AssertMsgAlways(false, "Not implemented");

	return nullptr;
}
