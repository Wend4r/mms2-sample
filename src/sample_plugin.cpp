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
#include <globals.hpp>

#include <stdint.h>

#include <string>

#include <sourcehook/sourcehook.h>

#include <serversideclient.h>
#include <tier0/commonmacros.h>

SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
SH_DECL_HOOK8(CNetworkGameServerBase, ConnectClient, SH_NOATTRIB, 0, CServerSideClientBase *, const char *, ns_address *, int, CCLCMsg_SplitPlayerConnect_t *, const char *, const byte *, int, bool);
SH_DECL_HOOK1_void(CServerSideClientBase, PerformDisconnection, SH_NOATTRIB, 0, ENetworkDisconnectionReason);

static SamplePlugin s_aSamplePlugin;
SamplePlugin *g_pSamplePlugin = &s_aSamplePlugin;

const ConcatLineString s_aEmbedConcat =
{
	"\t", // Start message.
	": ", // Padding of key & value.
	"\n", // End.
	"\n\t", // End and next line.
};

PLUGIN_EXPOSE(SamplePlugin, s_aSamplePlugin);

SamplePlugin::SamplePlugin()
 :  Logger(GetName(), [](LoggingChannelID_t nTagChannelID)
    {
    	LoggingSystem_AddTagToChannel(nTagChannelID, s_aSamplePlugin.GetLogTag());
    }, 0, LV_DEFAULT, SAMPLE_LOGGINING_COLOR)
{
}

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

	MessageFormat("Starting %s plugin...\n", GetName());

	if(!InitGlobals(ismm, error, maxlen))
	{
		return false;
	}

	if(IsChannelEnabled(LS_DETAILED))
	{
		CBufferStringGrowable<1024> sMessage;

		DumpGlobals(s_aEmbedConcat, sMessage);
		Detailed(sMessage);
	}

	if(!InitProvider(error, maxlen))
	{
		return false;
	}

	if(!LoadProvider(error, maxlen))
	{
		return false;
	}

	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &SamplePlugin::OnStartupServerHook, true);

	if(late)
	{
		auto *pNetServer = reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer());

		if(pNetServer)
		{
			OnStartupServer(pNetServer, pNetServer->m_GameConfig, NULL);

			{
				auto &vecClients = pNetServer->m_Clients;

				FOR_EACH_VEC(vecClients, i)
				{
					auto *pClient = vecClients[i];

					if(pClient->IsConnected() && !pClient->IsFakeClient())
					{
						OnConnectClient(pNetServer, pClient, pClient->GetClientName(), &pClient->m_nAddr, -1, NULL, NULL, NULL, 0, pClient->m_bLowViolence);
					}
				}
			}
		}
	}

	MessageFormat("%s started!\n", GetName());

	return true;
}

bool SamplePlugin::Unload(char *error, size_t maxlen)
{
	{
		auto *pNetServer = reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer());

		if(pNetServer)
		{
			SH_REMOVE_HOOK_MEMFUNC(CNetworkGameServerBase, ConnectClient, pNetServer, this, &SamplePlugin::OnConnectClientHook, true);
		}
	}

	SH_REMOVE_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &SamplePlugin::OnStartupServerHook, true);

	if(!UnloadProvider(error, maxlen))
	{
		return false;
	}

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

bool SamplePlugin::InitProvider(char *error, size_t maxlen)
{
	GameData::CBufferStringVector vecMessages;

	if(!Provider::Init(vecMessages))
	{
		if(IsChannelEnabled(LS_WARNING))
		{
			auto aWarnings = CreateWarningsScope();

			FOR_EACH_VEC(vecMessages, i)
			{
				auto &aMessage = vecMessages[i];

				aWarnings.Push(aMessage.Get());
			}

			aWarnings.SendColor([=](Color rgba, const CUtlString &sContext)
			{
				Warning(rgba, sContext);
			});
		}

		strncpy(error, "Failed to load provider. See warnings", maxlen);

		return false;
	}

	return true;
}

bool SamplePlugin::LoadProvider(char *error, size_t maxlen)
{
	GameData::CBufferStringVector vecMessages;

	if(!Provider::Load(SAMPLE_BASE_DIR, SAMPLE_BASE_PATHID, vecMessages))
	{
		if(IsChannelEnabled(LS_WARNING))
		{
			auto aWarnings = CreateWarningsScope();

			FOR_EACH_VEC(vecMessages, i)
			{
				auto &aMessage = vecMessages[i];

				aWarnings.Push(aMessage.Get());
			}

			aWarnings.SendColor([=](Color rgba, const CUtlString &sContext)
			{
				Warning(rgba, sContext);
			});
		}

		strncpy(error, "Failed to load provider. See warnings", maxlen);

		return false;
	}

	return true;
}

void SamplePlugin::OnReloadGameDataCommand(const CCommandContext &context, const CCommand &args)
{
	char error[256];

	if(!this->LoadProvider(error, sizeof(error)))
	{
		META_LOG(this, "%s", error);
	}
}

bool SamplePlugin::UnloadProvider(char *error, size_t maxlen)
{
	GameData::CBufferStringVector vecMessages;

	if(!Provider::Destroy(vecMessages))
	{
		if(IsChannelEnabled(LS_WARNING))
		{
			auto aWarnings = CreateWarningsScope();

			FOR_EACH_VEC(vecMessages, i)
			{
				auto &aMessage = vecMessages[i];

				aWarnings.Push(aMessage.Get());
			}

			aWarnings.SendColor([=](Color rgba, const CUtlString &sContext)
			{
				Warning(rgba, sContext);
			});
		}

		strncpy(error, "Failed to unload provider. See warnings", maxlen);

		return false;
	}

	return true;
}

void SamplePlugin::OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *)
{
	auto *pNetServer = reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer());

	OnStartupServer(pNetServer, config, pWorldSession);

	RETURN_META(MRES_IGNORED);
}

CServerSideClientBase *SamplePlugin::OnConnectClientHook(const char *pszName, ns_address *pAddr, int socket, CCLCMsg_SplitPlayerConnect_t *pSplitPlayer, 
                                                         const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence)
{
	auto *pNetServer = META_IFACEPTR(CNetworkGameServerBase);

	auto *pClient = META_RESULT_ORIG_RET(CServerSideClientBase *);

	OnConnectClient(pNetServer, pClient, pszName, pAddr, socket, pSplitPlayer, pszChallenge, pAuthTicket, nAuthTicketLength, bIsLowViolence);

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

void SamplePlugin::OnDisconectClientHook(ENetworkDisconnectionReason eReason)
{
	auto *pClient = META_IFACEPTR(CServerSideClientBase);

	OnDisconectClient(pClient, eReason);

	RETURN_META(MRES_IGNORED);
}

void SamplePlugin::DumpProtobufMessage(const ConcatLineString &aConcat, CBufferString &sOutput, const google::protobuf::Message &aMessage)
{
	CBufferStringGrowable<1024> sProtoOutput;

	sProtoOutput.Insert(0, aMessage.DebugString().c_str());
	sProtoOutput.Replace("\n", aConcat.m_aEndAndNextLine);
	
	const char *pszProtoConcat[] = {aConcat.m_aStartWith, sProtoOutput.Get()};

	sOutput.AppendConcat(ARRAYSIZE(pszProtoConcat), pszProtoConcat, NULL);
}

void SamplePlugin::DumpServerSideClient(const ConcatLineString &aConcat, CBufferString &sOutput, CServerSideClientBase *pClient)
{
	aConcat.AppendStringToBuffer(sOutput, "Name", pClient->GetClientName());
	aConcat.AppendToBuffer(sOutput, "Player slot", std::to_string(pClient->GetPlayerSlot().Get()).c_str());
	aConcat.AppendToBuffer(sOutput, "Entity index", std::to_string(pClient->GetEntityIndex().Get()).c_str());
	aConcat.AppendToBuffer(sOutput, "UserID", std::to_string(pClient->GetUserID().Get()).c_str());
	aConcat.AppendToBuffer(sOutput, "Signon state", std::to_string(pClient->GetSignonState()).c_str());
	aConcat.AppendToBuffer(sOutput, "SteamID", pClient->GetClientSteamID().Render());
	aConcat.AppendToBuffer(sOutput, "Is fake", pClient->IsFakeClient() ? "true" : "false");
	aConcat.AppendToBuffer(sOutput, "Address", pClient->GetRemoteAddress()->ToString());
	aConcat.AppendToBuffer(sOutput, "Low violence", pClient->IsLowViolenceClient() ? "true" : "false");
}

void SamplePlugin::DumpDisconnectReason(const ConcatLineString &aConcat, CBufferString &sOutput, ENetworkDisconnectionReason eReason)
{
	aConcat.AppendToBuffer(sOutput, "Disconnect reason", std::to_string((int)eReason).c_str());
}

void SamplePlugin::OnStartupServer(CNetworkGameServerBase *pNetServer, const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession)
{
	SH_ADD_HOOK_MEMFUNC(CNetworkGameServerBase, ConnectClient, pNetServer, this, &SamplePlugin::OnConnectClientHook, true);

	if(IsChannelEnabled(LS_DETAILED))
	{
		const auto &aConcat = s_aEmbedConcat;

		CBufferStringGrowable<1024, true> sMessage;

		sMessage.Format("Receive %s message:\n", config.GetTypeName().c_str());
		DumpProtobufMessage(aConcat, sMessage, config);

		Detailed(sMessage);
	}
}

void SamplePlugin::OnConnectClient(CNetworkGameServerBase *pNetServer, CServerSideClientBase *pClient, const char *pszName, ns_address *pAddr, int socket, CCLCMsg_SplitPlayerConnect_t *pSplitPlayer, const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence)
{
	SH_ADD_HOOK_MEMFUNC(CServerSideClientBase, PerformDisconnection, pClient, this, &SamplePlugin::OnDisconectClientHook, false);

	if(IsChannelEnabled(LS_DETAILED))
	{
		const auto &aConcat = s_aEmbedConcat;

		CBufferStringGrowable<1024> sMessage;

		sMessage.Insert(0, "Connect a client:\n");

		this->DumpServerSideClient(aConcat, sMessage, pClient);

		if(socket)
		{
			aConcat.AppendToBuffer(sMessage, "Socket", std::to_string(socket).c_str());
		}

		if(pAuthTicket && nAuthTicketLength)
		{
			aConcat.AppendToBuffer(sMessage, "Auth ticket", "has");
		}

		Detailed(sMessage);
	}
}

void SamplePlugin::OnDisconectClient(CServerSideClientBase *pClient, ENetworkDisconnectionReason eReason)
{
	SH_REMOVE_HOOK_MEMFUNC(CServerSideClientBase, PerformDisconnection, pClient, this, &SamplePlugin::OnDisconectClientHook, true);

	if(IsChannelEnabled(LS_DETAILED))
	{
		CBufferStringGrowable<1024> sMessage;

		const auto &aConcat = s_aEmbedConcat;

		sMessage.Insert(0, "Disconnect a client:\n");
		DumpServerSideClient(aConcat, sMessage, pClient);
		DumpDisconnectReason(aConcat, sMessage, eReason);

		Detailed(sMessage);
	}
}
