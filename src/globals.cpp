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

#include <globals.hpp>

#include <ISmmPlugin.h>

#include <iserver.h>
#include <tier0/dbg.h>

bool InitGlobals(SourceMM::ISmmAPI *ismm, char *error, size_t maxlen)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2Server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

	return true;
}

void DebugGlobals(SourceMM::ISmmAPI *ismm, SourceMM::ISmmPlugin *pl)
{
	const char *pszFormat = "[%s] %s = %p\n";

	const char *pszLogTag = pl->GetLogTag();

	ismm->ConPrintf(pszFormat, pszLogTag, "g_pEngineServer", g_pEngineServer);
	ismm->ConPrintf(pszFormat, pszLogTag, "g_pCVar", g_pCVar);
	ismm->ConPrintf(pszFormat, pszLogTag, "g_pFullFileSystem", g_pFullFileSystem);
	ismm->ConPrintf(pszFormat, pszLogTag, "g_pSource2Server", g_pSource2Server);
	ismm->ConPrintf(pszFormat, pszLogTag, "g_pNetworkServerService", g_pNetworkServerService);
}

bool DestoryGlobals(char *error, size_t maxlen)
{
	g_pEngineServer = NULL;
	g_pCVar = NULL;
	g_pFullFileSystem = NULL;
	g_pSource2Server = NULL;
	g_pNetworkServerService = NULL;

	return true;
}

// AMNOTE: Should only be called within the active game loop (i e map should be loaded and active) 
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

	if(!server)
	{
		AssertMsg(server, "Server is not ready");

		return NULL;
	}

	return server->GetGlobals();
}

CGameEntitySystem *GameEntitySystem()
{
	AssertMsgAlways(false, "Not implemented");

	return NULL;
}
