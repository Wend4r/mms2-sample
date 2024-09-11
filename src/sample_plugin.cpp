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

const ConcatLineString s_aEmbed2Concat =
{
	"\t\t",
	": ",
	"\n",
	"\n\t\t",
};

PLUGIN_EXPOSE(SamplePlugin, s_aSamplePlugin);

SamplePlugin::SamplePlugin()
 :  Logger(GetName(), [](LoggingChannelID_t nTagChannelID)
    {
    	LoggingSystem_AddTagToChannel(nTagChannelID, s_aSamplePlugin.GetLogTag());
    }, 0, LV_DEFAULT, SAMPLE_LOGGINING_COLOR),
    m_aEnableFrameDetailsConVar("mm_" META_PLUGIN_PREFIX "_enable_frame_details", FCVAR_RELEASE | FCVAR_GAMEDLL, "Enable frame detail messages", false, true, false, true, true)
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

	ConVar_Register(FCVAR_RELEASE | FCVAR_GAMEDLL);

	if(!InitProvider(error, maxlen))
	{
		return false;
	}

	if(!LoadProvider(error, maxlen))
	{
		return false;
	}

	if(!RegisterGameFactory(error, maxlen))
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

	if(!UnregisterGameResource(error, maxlen))
	{
		return false;
	}

	if(!UnregisterGameFactory(error, maxlen))
	{
		return false;
	}

	if(!DestoryGlobals(error, maxlen))
	{
		return false;
	}

	ConVar_Unregister();

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

bool SamplePlugin::Init()
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}

	return true;
}

void SamplePlugin::PostInit()
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

void SamplePlugin::Shutdown()
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(SamplePlugin, GameInit)
{
	if(IsChannelEnabled(LS_DETAILED))
	{

		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "Config");
			DumpProtobufMessage(aConcat2, sBuffer, *msg.m_pConfig);
			aConcat.AppendPointerToBuffer(sBuffer, "Registry", msg.m_pRegistry);
			Detailed(sBuffer);
		}
	}

	{
		char sMessage[256];

		if(!RegisterGameResource(sMessage, sizeof(sMessage)))
		{
			WarningFormat("%s\n", sMessage);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, GameShutdown)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(SamplePlugin, GamePostInit)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "Config");
			DumpProtobufMessage(aConcat2, sBuffer, *msg.m_pConfig);
			aConcat.AppendPointerToBuffer(sBuffer, "Registry", msg.m_pRegistry);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, GamePreShutdown)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(SamplePlugin, BuildGameSessionManifest)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendPointerToBuffer(sBuffer, "Config", msg.m_pResourceManifest);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, GameActivate)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat2.AppendToBuffer(sBuffer, "Event loop");
			DumpEngineLoopState(aConcat, sBuffer, *msg.m_pState);
			aConcat2.AppendToBuffer(sBuffer, "Back ground map", msg.m_bBackgroundMap ? "true" : "false");
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ClientFullySignedOn)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(SamplePlugin, Disconnect)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(SamplePlugin, GameDeactivate)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "Event loop");
			DumpEngineLoopState(aConcat2, sBuffer, *msg.m_pState);
			aConcat.AppendToBuffer(sBuffer, "Back ground map", msg.m_bBackgroundMap ? "true" : "false");
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, SpawnGroupPrecache)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "Event loop");
			aConcat2.AppendStringToBuffer(sBuffer, "Spawn group name", msg.m_SpawnGroupName);
			aConcat2.AppendStringToBuffer(sBuffer, "Entity lump name", msg.m_EntityLumpName);
			aConcat2.AppendHandleToBuffer(sBuffer, "Spawn group handle", msg.m_SpawnGroupHandle);
			aConcat2.AppendToBuffer(sBuffer, "Entity count", msg.m_nEntityCount);
			aConcat2.AppendPointerToBuffer(sBuffer, "Entities to spawn", msg.m_pEntitiesToSpawn);
			aConcat2.AppendPointerToBuffer(sBuffer, "Registry", msg.m_pRegistry);
			aConcat2.AppendPointerToBuffer(sBuffer, "Manifest", msg.m_pManifest);
			aConcat2.AppendPointerToBuffer(sBuffer, "Config", msg.m_pConfig);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, SpawnGroupUncache)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat2.AppendStringToBuffer(sBuffer, "Spawn group name", msg.m_SpawnGroupName);
			aConcat2.AppendStringToBuffer(sBuffer, "Entity lump name", msg.m_EntityLumpName);
			aConcat2.AppendHandleToBuffer(sBuffer, "Spawn group handle", msg.m_SpawnGroupHandle);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, PreSpawnGroupLoad)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat2.AppendStringToBuffer(sBuffer, "Spawn group name", msg.m_SpawnGroupName);
			aConcat2.AppendStringToBuffer(sBuffer, "Entity lump name", msg.m_EntityLumpName);
			aConcat2.AppendHandleToBuffer(sBuffer, "Spawn group handle", msg.m_SpawnGroupHandle);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, PostSpawnGroupLoad)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendStringToBuffer(sBuffer, "Spawn group name", msg.m_SpawnGroupName);
			aConcat.AppendStringToBuffer(sBuffer, "Entity lump name", msg.m_EntityLumpName);
			aConcat.AppendHandleToBuffer(sBuffer, "Spawn group handle", msg.m_SpawnGroupHandle);
			aConcat2.AppendToBuffer(sBuffer, "Entity list");
			DumpEntityList(aConcat2, sBuffer, msg.m_EntityList);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, PreSpawnGroupUnload)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendStringToBuffer(sBuffer, "Spawn group name", msg.m_SpawnGroupName);
			aConcat.AppendStringToBuffer(sBuffer, "Entity lump name", msg.m_EntityLumpName);
			aConcat.AppendHandleToBuffer(sBuffer, "Spawn group handle", msg.m_SpawnGroupHandle);
			aConcat.AppendToBuffer(sBuffer, "Entity list");
			DumpEntityList(aConcat2, sBuffer, msg.m_EntityList);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, PostSpawnGroupUnload)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendStringToBuffer(sBuffer, "Spawn group name", msg.m_SpawnGroupName);
			aConcat.AppendStringToBuffer(sBuffer, "Entity lump name", msg.m_EntityLumpName);
			aConcat.AppendHandleToBuffer(sBuffer, "Spawn group handle", msg.m_SpawnGroupHandle);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ActiveSpawnGroupChanged)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendHandleToBuffer(sBuffer, "Spawn group handle", msg.m_SpawnGroupHandle);
			aConcat.AppendStringToBuffer(sBuffer, "Spawn group name", msg.m_SpawnGroupName);
			aConcat.AppendHandleToBuffer(sBuffer, "Previous handle", msg.m_PreviousHandle);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ClientPostDataUpdate)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(SamplePlugin, ClientPreRender)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "Frame time", msg.m_flFrameTime);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ClientPreEntityThink)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "First tick", msg.m_bFirstTick);
			aConcat.AppendToBuffer(sBuffer, "Last tick", msg.m_bLastTick);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ClientUpdate)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "Frame time", msg.m_flFrameTime);
			aConcat.AppendToBuffer(sBuffer, "First tick", msg.m_bFirstTick);
			aConcat.AppendToBuffer(sBuffer, "Last tick", msg.m_bLastTick);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ClientPostRender)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(SamplePlugin, ServerPreEntityThink)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;
			aConcat.AppendToBuffer(sBuffer, "First tick", msg.m_bFirstTick);
			aConcat.AppendToBuffer(sBuffer, "Last tick", msg.m_bLastTick);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ServerPostEntityThink)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "First tick", msg.m_bFirstTick);
			aConcat.AppendToBuffer(sBuffer, "Last tick", msg.m_bLastTick);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ServerPreClientUpdate)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(SamplePlugin, ServerGamePostSimulate)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			DumpEventSimulate(aConcat, aConcat2, sBuffer, msg);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, ClientGamePostSimulate)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			DumpEventSimulate(aConcat, aConcat2, sBuffer, msg);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, GameFrameBoundary)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			DumpEventFrameBoundary(aConcat2, sBuffer, msg);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, OutOfGameFrameBoundary)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			DumpEventFrameBoundary(aConcat2, sBuffer, msg);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, SaveGame)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;
			aConcat.AppendToBuffer(sBuffer, "Entity list");
			DumpEntityList(aConcat2, sBuffer, *msg.m_pEntityList);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, RestoreGame)
{
	if(IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat, 
			           &aConcat2 = s_aEmbed2Concat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendToBuffer(sBuffer, "Entity list");
			DumpEntityList(aConcat2, sBuffer, *msg.m_pEntityList);
			Detailed(sBuffer);
		}
	}
}

bool SamplePlugin::InitProvider(char *error, size_t maxlen)
{
	GameData::CBufferStringVector vecMessages;

	bool bResult = Provider::Init(vecMessages);

	if(vecMessages.Count())
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
	}

	if(!bResult)
	{
		strncpy(error, "Failed to initialize provider. See warnings", maxlen);
	}

	return bResult;
}

bool SamplePlugin::LoadProvider(char *error, size_t maxlen)
{
	GameData::CBufferStringVector vecMessages;

	bool bResult = Provider::Load(SAMPLE_BASE_DIR, SAMPLE_BASE_PATHID, vecMessages);

	if(vecMessages.Count())
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
	}

	if(!bResult)
	{
		strncpy(error, "Failed to load provider. See warnings", maxlen);
	}

	return bResult;
}

bool SamplePlugin::UnloadProvider(char *error, size_t maxlen)
{
	GameData::CBufferStringVector vecMessages;

	bool bResult = Provider::Destroy(vecMessages);

	if(vecMessages.Count())
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
	}

	if(!bResult)
	{
		strncpy(error, "Failed to unload provider. See warnings", maxlen);
	}

	return bResult;
}

bool SamplePlugin::RegisterGameResource(char *error, size_t maxlen)
{
	CGameEntitySystem **pGameEntitySystem = reinterpret_cast<CGameEntitySystem **>((uintptr_t)g_pGameResourceServiceServer + GetGameDataStorage().GetGameResource().GetEntitySystemOffset());

	if(!pGameEntitySystem)
	{
		strncpy(error, "Failed to get a game entity system", maxlen);
	}

	if(!RegisterGameEntitySystem(*pGameEntitySystem))
	{
		strncpy(error, "Failed to register a (game) entity system", maxlen);

		return false;
	}

	return true;
}

bool SamplePlugin::UnregisterGameResource(char *error, size_t maxlen)
{
	if(!UnregisterGameEntitySystem())
	{
		strncpy(error, "Failed to unregister a (game) entity system", maxlen);

		return false;
	}

	return true;
}

bool SamplePlugin::RegisterGameFactory(char *error, size_t maxlen)
{
	if(!RegisterFirstGameSystem(GetGameDataStorage().GetGameSystem().GetFirstGameSystem()))
	{
		strncpy(error, "Failed to register a first game factory", maxlen);

		return false;
	}

	m_pFactory = new CGameSystemStaticFactory<SamplePlugin>(GetName(), this);

	return true;
}

bool SamplePlugin::UnregisterGameFactory(char *error, size_t maxlen)
{
	if(m_pFactory)
	{
		m_pFactory->Shutdown();
	}

	if(!UnregisterFirstGameSystem())
	{
		strncpy(error, "Failed to unregister a first game factory", maxlen);

		return false;
	}

	return true;
}

void SamplePlugin::OnReloadGameDataCommand(const CCommandContext &context, const CCommand &args)
{
	char error[256];

	if(!LoadProvider(error, sizeof(error)))
	{
		META_LOG(this, "%s", error);
	}
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

void SamplePlugin::DumpEngineLoopState(const ConcatLineString &aConcat, CBufferString &sOutput, const EngineLoopState_t &aMessage)
{
	aConcat.AppendHandleToBuffer(sOutput, "Window handle", aMessage.m_hWnd);
	aConcat.AppendHandleToBuffer(sOutput, "Swap chain handle", aMessage.m_hSwapChain);
	aConcat.AppendHandleToBuffer(sOutput, "Input context handle", aMessage.m_hInputContext);
	aConcat.AppendToBuffer(sOutput, "Window width", aMessage.m_nPlatWindowWidth);
	aConcat.AppendToBuffer(sOutput, "Window height", aMessage.m_nPlatWindowHeight);
	aConcat.AppendToBuffer(sOutput, "Render width", aMessage.m_nRenderWidth);
	aConcat.AppendToBuffer(sOutput, "Render height", aMessage.m_nRenderHeight);
}

void SamplePlugin::DumpEntityList(const ConcatLineString &aConcat, CBufferString &sOutput, const CUtlVector<CEntityHandle> &vecEntityList)
{
	for(const auto &it : vecEntityList)
	{
		aConcat.AppendToBuffer(sOutput, it.Get()->GetClassname(), it.GetEntryIndex());
	}
}

void SamplePlugin::DumpEventSimulate(const ConcatLineString &aConcat, const ConcatLineString &aConcat2, CBufferString &sOutput, const EventSimulate_t &aMessage)
{
	aConcat.AppendToBuffer(sOutput, "Loop state");
	DumpEngineLoopState(aConcat2, sOutput, aMessage.m_LoopState);
	aConcat.AppendToBuffer(sOutput, "First tick", aMessage.m_bFirstTick);
	aConcat.AppendToBuffer(sOutput, "Last tick", aMessage.m_bLastTick);
}

void SamplePlugin::DumpEventFrameBoundary(const ConcatLineString &aConcat, CBufferString &sOutput, const EventFrameBoundary_t &aMessage)
{
	aConcat.AppendToBuffer(sOutput, "Frame time", aMessage.m_flFrameTime);
}

void SamplePlugin::DumpServerSideClient(const ConcatLineString &aConcat, CBufferString &sOutput, CServerSideClientBase *pClient)
{
	aConcat.AppendStringToBuffer(sOutput, "Name", pClient->GetClientName());
	aConcat.AppendToBuffer(sOutput, "Player slot", pClient->GetPlayerSlot().Get());
	aConcat.AppendToBuffer(sOutput, "Entity index", pClient->GetEntityIndex().Get());
	aConcat.AppendToBuffer(sOutput, "UserID", pClient->GetUserID().Get());
	aConcat.AppendToBuffer(sOutput, "Signon state", pClient->GetSignonState());
	aConcat.AppendToBuffer(sOutput, "SteamID", pClient->GetClientSteamID().Render());
	aConcat.AppendToBuffer(sOutput, "Is fake", pClient->IsFakeClient());
	aConcat.AppendToBuffer(sOutput, "Address", pClient->GetRemoteAddress()->ToString());
	aConcat.AppendToBuffer(sOutput, "Low violence", pClient->IsLowViolenceClient());
}

void SamplePlugin::DumpDisconnectReason(const ConcatLineString &aConcat, CBufferString &sOutput, ENetworkDisconnectionReason eReason)
{
	aConcat.AppendToBuffer(sOutput, "Disconnect reason", (int)eReason);
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
			aConcat.AppendHandleToBuffer(sMessage, "Socket", (uint32)socket);
		}

		if(pAuthTicket && nAuthTicketLength)
		{
			aConcat.AppendBytesToBuffer(sMessage, "Auth ticket", pAuthTicket, nAuthTicketLength);
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
