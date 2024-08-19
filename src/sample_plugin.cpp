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

#include <sample_plugin.hpp>
#include <proto.hpp>
#include <globals.hpp>

#include <sourcehook/sourcehook.h>

#include <iserver.h>
#include <tier0/bufferstring.h>
#include <tier0/commonmacros.h>

SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);

static SamplePlugin s_aSamplePlugin;
SamplePlugin *g_pSamplePlugin = &s_aSamplePlugin;

PLUGIN_EXPOSE(SamplePlugin, s_aSamplePlugin);

const char *SamplePlugin::GetAuthor()        { return META_PLUGIN_AUTHOR; }
const char *SamplePlugin::GetName()          { return META_PLUGIN_NAME; }
const char *SamplePlugin::GetDescription()   { return META_PLUGIN_DESCRIPTION; }
const char *SamplePlugin::GetURL()           { return META_PLUGIN_URL; }
const char *SamplePlugin::GetLicense()       { return META_PLUGIN_LICENSE; }
const char *SamplePlugin::GetVersion()       { return META_PLUGIN_VERSION; }
const char *SamplePlugin::GetDate()          { return META_PLUGIN_DATE; }
const char *SamplePlugin::GetLogTag()        { return META_PLUGIN_LOG_TAG; }

bool SamplePlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	META_CONPRINTF("Starting %s plugin...\n", GetName());

	if(!InitGlobals(ismm, error, maxlen))
	{
		return false;
	}

	DebugGlobals(ismm, this);

	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &SamplePlugin::OnStartupServerHook, true);

	if(late)
	{
		auto *pNetServer = reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer());

		if(pNetServer)
		{
			OnStartupServerHook(pNetServer->m_GameConfig, NULL, pNetServer->GetMapName());
		}
	}

	META_CONPRINTF("%s started!\n", GetName());

	return true;
}

bool SamplePlugin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &SamplePlugin::OnStartupServerHook, true);

	if(!DestoryGlobals(error, maxlen))
	{
		return false;
	}

	// ...

	return true;
}

bool SamplePlugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool SamplePlugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

void SamplePlugin::AllPluginsLoaded()
{
	/**
	 * AMNOTE: This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

void SamplePlugin::OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *)
{
	INetworkGameServer *pNetServer = g_pNetworkServerService->GetIGameServer();

	// Debug a config.
	{
		static const char pszMemberPadding[] = " = ", 
		                  pszEndPadding[] = "\n";

		CBufferStringGrowable<1024, true> sMessage;

		sMessage.Format("[%s] Receive %s message:\n", GetLogTag(), config.GetTypeName().c_str()); 

#define PROTO_CONCAT_PROTO_MEMBER_BASE_LOCAL(member) PROTO_CONCAT_PROTO_MEMBER_BASE(config, member, pszMemberPadding)
#define PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(member) PROTO_CONCAT_PROTO_MEMBER_TO_C(config, pszMemberPadding, member), pszEndPadding
#define PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL(member) PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN(config, pszMemberPadding, member), pszEndPadding
#define PROTO_MEMEBER_TO_STRING_LOCAL(member) PROTO_MEMEBER_TO_STRING(config, member)

		auto min_client_limit = PROTO_MEMEBER_TO_STRING_LOCAL(min_client_limit), 
		     max_client_limit = PROTO_MEMEBER_TO_STRING_LOCAL(max_client_limit), 
		     max_clients = PROTO_MEMEBER_TO_STRING_LOCAL(max_clients), 
		     tick_interval = PROTO_MEMEBER_TO_STRING_LOCAL(tick_interval);

		const char *pszMessageConcat[] =
		{
			PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL(is_multiplayer), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL(is_loadsavegame), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL(is_background_map), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL(is_headless), 
			PROTO_CONCAT_PROTO_MEMBER_BASE_LOCAL(min_client_limit), min_client_limit.c_str(), pszEndPadding, 
			PROTO_CONCAT_PROTO_MEMBER_BASE_LOCAL(max_client_limit), max_client_limit.c_str(), pszEndPadding, 
			PROTO_CONCAT_PROTO_MEMBER_BASE_LOCAL(max_clients), max_clients.c_str(), pszEndPadding, 
			PROTO_CONCAT_PROTO_MEMBER_BASE_LOCAL(tick_interval), tick_interval.c_str(), pszEndPadding, 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(hostname), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(savegamename), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(s1_mapname), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(gamemode), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(server_ip_address), 
			// PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(data), // Binary.
			PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL(is_localonly), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL(no_steam_server), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL(is_transition), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(previouslevel), 
			PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL(landmarkname), 
		};

#undef PROTO_MEMEBER_TO_STRING_LOCAL
#undef PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN_LOCAL
#undef PROTO_CONCAT_PROTO_MEMBER_TO_C_LOCAL
#undef PROTO_CONCAT_PROTO_MEMBER_BASE_LOCAL

		sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);

		META_CONPRINT(sMessage.Get());
	}
}
