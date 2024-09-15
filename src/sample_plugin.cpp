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
#include <exception>

#include <any_config.hpp>

#include <sourcehook/sourcehook.h>

#include <filesystem.h>
#include <igameeventsystem.h>
#include <inetchannel.h>
#include <networksystem/inetworkmessages.h>
#include <networksystem/inetworkserializer.h>
#include <recipientfilter.h>
#include <serversideclient.h>
#include <shareddefs.h>
#include <tier0/commonmacros.h>
#include <usermessages.pb.h>

SH_DECL_HOOK3_void(ICvar, DispatchConCommand, SH_NOATTRIB, 0, ConCommandHandle, const CCommandContext &, const CCommand &);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
SH_DECL_HOOK8(CNetworkGameServerBase, ConnectClient, SH_NOATTRIB, 0, CServerSideClientBase *, const char *, ns_address *, int, CCLCMsg_SplitPlayerConnect_t *, const char *, const byte *, int, bool);
SH_DECL_HOOK1_void(CServerSideClientBase, PerformDisconnection, SH_NOATTRIB, 0, ENetworkDisconnectionReason);

static SamplePlugin s_aSamplePlugin;
SamplePlugin *g_pSamplePlugin = &s_aSamplePlugin;

const ConcatLineString s_aEmbedConcat =
{
	{
		"\t", // Start message.
		": ", // Padding of key & value.
		"\n", // End.
		"\n\t", // End and next line.
	}
};

const ConcatLineString s_aEmbed2Concat =
{
	{
		"\t\t",
		": ",
		"\n",
		"\n\t\t",
	}
};

PLUGIN_EXPOSE(SamplePlugin, s_aSamplePlugin);

SamplePlugin::SamplePlugin()
 :  Logger(GetName(), [](LoggingChannelID_t nTagChannelID)
    {
    	LoggingSystem_AddTagToChannel(nTagChannelID, s_aSamplePlugin.GetLogTag());
    }, 0, LV_DETAILED, SAMPLE_LOGGINING_COLOR),
    m_aEnableFrameDetailsConVar("mm_" META_PLUGIN_PREFIX "_enable_frame_details", FCVAR_RELEASE | FCVAR_GAMEDLL, "Enable detail messages of frames", false, true, false, true, true), 
    m_aEnableGameEventsDetaillsConVar("mm_" META_PLUGIN_PREFIX "_enable_game_events_details", FCVAR_RELEASE | FCVAR_GAMEDLL, "Enable detail messages of game events", false, true, false, true, true)
{
}

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

	if(!LoadTranslations(error, maxlen))
	{
		return false;
	}

	if(!RegisterGameFactory(error, maxlen))
	{
		return false;
	}

	Assert(ParseGameEvents());

	SH_ADD_HOOK(ICvar, DispatchConCommand, g_pCVar, SH_MEMBER(this, &SamplePlugin::OnDispatchConCommandHook), false);
	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &SamplePlugin::OnStartupServerHook, true);

	// Register chat commands.
	Sample::ChatCommandSystem::Register("sample", [&](CPlayerSlot aSlot, bool bIsSilent, const CUtlVector<CUtlString> &vecArguments)
	{
		Warning("Executing sample command\n");

		static const char s_pszYourArgumentPhrase[] = "Your argument", 
		                  s_pszDefaultContryCode[] = "en";

		CSingleRecipientFilter aFilter(aSlot);

		Translations::CPhrase::CContent aContent;
		Translations::CPhrase::CFormat aFormat;

		int iFound {};

		const auto &aTranslations = m_aTranslations;

		{
			if(aTranslations.FindPhrase(s_pszYourArgumentPhrase, iFound))
			{
				const auto &aPhrase = aTranslations.GetPhrase(iFound);

				if(!aPhrase.Find(s_pszDefaultContryCode, aContent))
				{
					WarningFormat("Not found \"%s\" country code for \"%s\" phrase\n", s_pszDefaultContryCode, s_pszYourArgumentPhrase);
				}

				aFormat = aPhrase.GetFormat();
			}
			else
			{
				WarningFormat("Not found \"%s\" phrase\n", s_pszYourArgumentPhrase);
			}
		}

		{
			CUtlString sOutput;

			if(!aContent.IsEmpty())
			{
				for(const auto &sArgument : vecArguments)
				{
					sOutput = aContent.Format(aFormat, 1, sArgument.Get());

					if(!sOutput.IsEmpty())
					{
						SendTextMessage(&aFilter, HUD_PRINTTALK, 1, sOutput.Get());
					}
				}
			}
		}
	});

	if(late)
	{
		auto *pNetServer = reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer());

		if(pNetServer)
		{
			OnStartupServer(pNetServer, pNetServer->m_GameConfig, NULL);

			for(const auto &pClient : pNetServer->m_Clients)
			{
				if(pClient->IsConnected() && !pClient->IsFakeClient())
				{
					OnConnectClient(pNetServer, pClient, pClient->GetClientName(), &pClient->m_nAddr, -1, NULL, NULL, NULL, 0, pClient->m_bLowViolence);
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

	Assert(UnhookGameEvents());

	if(!UnloadTranslations(error, maxlen))
	{
		return false;
	}

	if(!UnloadProvider(error, maxlen))
	{
		return false;
	}

	if(!UnregisterNetMessages(error, maxlen))
	{
		return false;
	}

	if(!UnregisterSource2Server(error, maxlen))
	{
		return false;
	}

	if(!UnregisterGameFactory(error, maxlen))
	{
		return false;
	}

	if(!UnregisterGameResource(error, maxlen))
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

const char *SamplePlugin::GetAuthor()        { return META_PLUGIN_AUTHOR; }
const char *SamplePlugin::GetName()          { return META_PLUGIN_NAME; }
const char *SamplePlugin::GetDescription()   { return META_PLUGIN_DESCRIPTION; }
const char *SamplePlugin::GetURL()           { return META_PLUGIN_URL; }
const char *SamplePlugin::GetLicense()       { return META_PLUGIN_LICENSE; }
const char *SamplePlugin::GetVersion()       { return META_PLUGIN_VERSION; }
const char *SamplePlugin::GetDate()          { return META_PLUGIN_DATE; }
const char *SamplePlugin::GetLogTag()        { return META_PLUGIN_LOG_TAG; }

void *SamplePlugin::OnMetamodQuery(const char *iface, int *ret)
{
	if(!strcmp(iface, SAMPLE_INTERFACE_NAME))
	{
		if(ret)
		{
			*ret = META_IFACE_OK;
		}

		return this;
	}

	if(ret)
	{
		*ret = META_IFACE_FAILED;
	}

	return nullptr;
}

CGameEntitySystem **SamplePlugin::GetGameEntitySystemPointer()
{
	return reinterpret_cast<CGameEntitySystem **>((uintptr_t)g_pGameResourceServiceServer + GetGameDataStorage().GetGameResource().GetEntitySystemOffset());
}

CBaseGameSystemFactory **SamplePlugin::GetFirstGameSystemPointer()
{
	return GetGameDataStorage().GetGameSystem().GetFirstGameSystemPointer();
}

IGameEventManager2 **SamplePlugin::GetGameEventManagerPointer()
{
	return reinterpret_cast<IGameEventManager2 **>(GetGameDataStorage().GetSource2Server().GetGameEventManagerPointer());
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

#ifndef _WIN32
			try
			{
				aConcat.AppendToBuffer(sBuffer, "Config");
				DumpProtobufMessage(aConcat2, sBuffer, *msg.m_pConfig);
			}
			catch(const std::exception &aError)
			{
				sBuffer.Format("Config: %s\n", aError.what());
			}

#endif
			aConcat.AppendPointerToBuffer(sBuffer, "Registry", msg.m_pRegistry);
			Detailed(sBuffer);
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

#ifndef _WIN32
			try
			{
				aConcat.AppendToBuffer(sBuffer, "Config");
				DumpProtobufMessage(aConcat2, sBuffer, *msg.m_pConfig);
			}
			catch(const std::exception &aError)
			{
				sBuffer.Format("Config: %s\n", aError.what());
			}

#endif
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

			aConcat.AppendToBuffer(sBuffer, "Event loop");
			DumpEngineLoopState(aConcat2, sBuffer, *msg.m_pState);
			aConcat.AppendToBuffer(sBuffer, "Back ground map", msg.m_bBackgroundMap ? "true" : "false");
			Detailed(sBuffer);
		}
	}

	// Initialize a game resource.
	{
		char sMessage[256];

		if(!RegisterGameResource(sMessage, sizeof(sMessage)))
		{
			WarningFormat("%s\n", sMessage);
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
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			aConcat.AppendStringToBuffer(sBuffer, "Spawn group name", msg.m_SpawnGroupName);
			aConcat.AppendStringToBuffer(sBuffer, "Entity lump name", msg.m_EntityLumpName);
			aConcat.AppendHandleToBuffer(sBuffer, "Spawn group handle", msg.m_SpawnGroupHandle);
			aConcat.AppendToBuffer(sBuffer, "Entity count", msg.m_nEntityCount);
			aConcat.AppendPointerToBuffer(sBuffer, "Entities to spawn", msg.m_pEntitiesToSpawn);
			aConcat.AppendPointerToBuffer(sBuffer, "Registry", msg.m_pRegistry);
			aConcat.AppendPointerToBuffer(sBuffer, "Manifest", msg.m_pManifest);
			aConcat.AppendPointerToBuffer(sBuffer, "Config", msg.m_pConfig);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, SpawnGroupUncache)
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

GS_EVENT_MEMBER(SamplePlugin, PreSpawnGroupLoad)
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
			aConcat.AppendToBuffer(sBuffer, "Entity list");
			DumpEntityList(aConcat2, sBuffer, msg.m_EntityList);
			Detailed(sBuffer);
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
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			DumpEventFrameBoundary(aConcat, sBuffer, msg);
			Detailed(sBuffer);
		}
	}
}

GS_EVENT_MEMBER(SamplePlugin, OutOfGameFrameBoundary)
{
	if(m_aEnableFrameDetailsConVar.GetValue() && IsChannelEnabled(LS_DETAILED))
	{
		DetailedFormat("%s:\n", __FUNCTION__);

		{
			const auto &aConcat = s_aEmbedConcat;

			CBufferStringGrowable<1024> sBuffer;

			DumpEventFrameBoundary(aConcat, sBuffer, msg);
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

void SamplePlugin::FireGameEvent(IGameEvent *event)
{
	if(!m_aEnableGameEventsDetaillsConVar.GetValue())
	{
		return;
	}

	KeyValues3 *pEventDataKeys = event->GetDataKeys();

	if(!pEventDataKeys)
	{
		WarningFormat("Data keys is empty at \"%s\" event", event->GetName());

		return;
	}

	if(IsChannelEnabled(LS_DETAILED))
	{
		int iMemberCount = pEventDataKeys->GetMemberCount();

		if(!iMemberCount)
		{
			WarningFormat("No members at \"%s\" event", event->GetName());

			return;
		}

		{
			auto aDetails = CreateDetailsScope();

			aDetails.PushFormat("\"%s\":", event->GetName());
			aDetails.Push("{");

			for(KV3MemberId_t id = 0; id < iMemberCount; id++)
			{
				const char *pEventMemberName = pEventDataKeys->GetMemberName(id);

				KeyValues3 *pEventMember = pEventDataKeys->GetMember(id);

				CBufferStringGrowable<128> sEventMember;

				pEventMember->ToString(sEventMember, KV3_TO_STRING_DONT_CLEAR_BUFF);
				aDetails.PushFormat("\t\"%s\":\t%s", pEventMemberName, sEventMember.Get());
			}

			aDetails.Push("}");
			aDetails.Send([&](const CUtlString &sMessage)
			{
				Detailed(sMessage);
			});
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
		if(error && maxlen)
		{
			strncpy(error, "Failed to initialize provider. See warnings", maxlen);
		}
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
		if(error && maxlen)
		{
			strncpy(error, "Failed to load provider. See warnings", maxlen);
		}
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
		if(error && maxlen)
		{
			strncpy(error, "Failed to unload provider. See warnings", maxlen);
		}
	}

	return bResult;
}

bool SamplePlugin::LoadTranslations(char *error, size_t maxlen)
{
	const char *pszPathID = SAMPLE_BASE_PATHID, 
	           *pszTranslationsFiles = SAMPLE_GAME_TRANSLATIONS_PATH_FILES;

	CUtlVector<CUtlString> vecTranslations;

	Translations::CBufferStringVector vecSubmessages;

	CUtlString sMessage;

	auto aWarnings = CreateWarningsScope();

	AnyConfig::LoadFromFile_Generic_t aLoadPresets({{&sMessage, NULL, pszPathID}, g_KV3Format_Generic});

	CBufferStringGrowable<1024> sWarningMessage;

	g_pFullFileSystem->FindFileAbsoluteList(vecTranslations, pszTranslationsFiles, pszPathID);

	if(!vecTranslations.Count())
	{
		snprintf(error, maxlen, "No found translations by \"%s\" path", pszTranslationsFiles);

		return false;
	}

	for(const auto &sFile : vecTranslations)
	{
		const char *pszFilename = sFile.Get();

		AnyConfig::Anyone aTranslationsConfig;

		aLoadPresets.m_pszFilename = pszFilename;

		if(!aTranslationsConfig.Load(aLoadPresets))
		{
			aWarnings.PushFormat("\"%s\": %s", pszFilename, sMessage.Get());

			continue;
		}

		if(!m_aTranslations.Parse(aTranslationsConfig.Get(), vecSubmessages))
		{
			aWarnings.PushFormat("\"%s\"", pszFilename);

			for(const auto &sSubmessage : vecSubmessages)
			{
				aWarnings.PushFormat("\t%s", sSubmessage.Get());
			}

			continue;
		}

		MessageFormat("Add to tail %s file", pszFilename);
	}

	if(aWarnings.Count())
	{
		aWarnings.Send([&](const CUtlString &sMessage)
		{
			Warning(sMessage);
		});
	}

	return true;
}

bool SamplePlugin::UnloadTranslations(char *error, size_t maxlen)
{
	m_aTranslations.Purge();

	return true;
}

bool SamplePlugin::RegisterGameResource(char *error, size_t maxlen)
{
	CGameEntitySystem **pGameEntitySystem = GetGameEntitySystemPointer();

	if(!pGameEntitySystem)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to get a game entity system", maxlen);
		}
	}

	DetailedFormat("RegisterGameEntitySystem\n");

	if(!RegisterGameEntitySystem(*pGameEntitySystem))
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to register a (game) entity system", maxlen);
		}

		return false;
	}

	return true;
}

bool SamplePlugin::UnregisterGameResource(char *error, size_t maxlen)
{
	if(!UnregisterGameEntitySystem())
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to unregister a (game) entity system", maxlen);
		}

		return false;
	}

	return true;
}

bool SamplePlugin::RegisterGameFactory(char *error, size_t maxlen)
{
	CBaseGameSystemFactory **ppFactory = GetGameDataStorage().GetGameSystem().GetFirstGameSystemPointer();

	if(!ppFactory)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to get a first game system factory", maxlen);
		}

		return false;
	}

	if(!RegisterFirstGameSystem(ppFactory))
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to register a first game factory", maxlen);
		}

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
		m_pFactory->DestroyGameSystem(this);
	}

	if(!UnregisterFirstGameSystem())
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to unregister a first game factory", maxlen);
		}

		return false;
	}

	return true;
}

bool SamplePlugin::RegisterSource2Server(char *error, size_t maxlen)
{
	IGameEventManager2 **ppGameEventManager = GetGameEventManagerPointer();

	if(!ppGameEventManager)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to get a game event manager", maxlen);
		}

		return false;
	}

	if(!RegisterGameEventManager(*ppGameEventManager))
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to register a game event manager", maxlen);
		}

		return false;
	}

	return true;
}

bool SamplePlugin::UnregisterSource2Server(char *error, size_t maxlen)
{
	if(!UnregisterGameEventManager())
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to register a game event manager", maxlen);
		}

		return false;
	}

	return true;
}

bool SamplePlugin::RegisterNetMessages(char *error, size_t maxlen)
{
	struct
	{
		const char *pszName;
		INetworkMessageInternal **ppInternal;
	} aMessageInitializers[] =
	{
		{
			"CUserMessageSayText2",
			&m_pSayText2Message,
		},
		{
			"CUserMessageTextMsg",
			&m_pTextMsgMessage,
		},
	};

	for(size_t n = 0, nSize = ARRAYSIZE(aMessageInitializers); n < nSize; n++)
	{
		auto &aMessageInitializer = aMessageInitializers[n];

		const char *pszMessageName = aMessageInitializer.pszName;

		INetworkMessageInternal *pMessage = g_pNetworkMessages->FindNetworkMessagePartial(pszMessageName);

		if(!pMessage)
		{
			if(error && maxlen)
			{
				snprintf(error, maxlen, "Failed to get \"%s\" message", pszMessageName);
			}

			return false;
		}

		*aMessageInitializer.ppInternal = pMessage;
	}

	return true;
}

bool SamplePlugin::UnregisterNetMessages(char *error, size_t maxlen)
{
	m_pSayText2Message = NULL;

	return true;
}

bool SamplePlugin::ParseGameEvents()
{
	const char *pszPathID = SAMPLE_BASE_PATHID;

	CUtlVector<CUtlString> vecGameEventFiles;

	CUtlVector<CUtlString> vecSubmessages;

	CUtlString sMessage;

	auto aWarnings = CreateWarningsScope();

	AnyConfig::LoadFromFile_Generic_t aLoadPresets({{&sMessage, NULL, pszPathID}, g_KV3Format_Generic});

	CBufferStringGrowable<1024> sWarningMessage;

	g_pFullFileSystem->FindFileAbsoluteList(vecGameEventFiles, SAMPLE_GAME_EVENTS_FILES, pszPathID);

	for(const auto &sFile : vecGameEventFiles)
	{
		const char *pszFilename = sFile.Get();

		AnyConfig::Anyone aGameEventConfig;

		aLoadPresets.m_pszFilename = pszFilename;

		if(!aGameEventConfig.Load(aLoadPresets))
		{
			aWarnings.PushFormat("\"%s\": %s", pszFilename, sMessage.Get());

			continue;
		}

		if(!ParseGameEvents(aGameEventConfig.Get(), vecSubmessages))
		{
			aWarnings.PushFormat("\"%s\":", pszFilename);

			for(const auto &sSubmessage : vecSubmessages)
			{
				aWarnings.PushFormat("\t%s", sSubmessage.Get());
			}

			continue;
		}

		// ...
	}

	if(aWarnings.Count())
	{
		aWarnings.Send([&](const CUtlString &sMessage)
		{
			Warning(sMessage);
		});
	}

	return true;
}

bool SamplePlugin::ParseGameEvents(KeyValues3 *pEvents, CUtlVector<CUtlString> &vecMessages)
{
	int iMemberCount = pEvents->GetMemberCount();

	if(!iMemberCount)
	{
		vecMessages.AddToTail("No members");

		return false;
	}

	CUtlString sMessage;

	for(KV3MemberId_t n = 0; n < iMemberCount; n++)
	{
		const char *pszEvent = pEvents->GetMemberName(n);

		if(!pszEvent)
		{
			sMessage.Format("No member name at #%d", n);
			vecMessages.AddToTail(sMessage);

			continue;
		}

		m_vecGameEvents.AddToTail(pszEvent);
	}

	return iMemberCount;
}

bool SamplePlugin::ClearGameEvents()
{
	m_vecGameEvents.Purge();

	return true;
}

bool SamplePlugin::HookGameEvents()
{
	auto aWarnings = CreateWarningsScope();

	static const char *pszWarningFormat = "Failed to hook \"%s\" event";

	for(const auto &sEvent : m_vecGameEvents)
	{
		const char *pszEvent = sEvent.Get();

		if(g_pGameEventManager->AddListener(this, pszEvent, true) == -1)
		{
			aWarnings.PushFormat(pszWarningFormat, pszEvent);

			continue;
		}

#ifdef DEBUG
		DetailedFormat("Hooked \"%s\" event\n", pszEvent);
#endif
	}

	if(aWarnings.Count())
	{
		aWarnings.Send([&](const CUtlString &sMessage)
		{
			Warning(sMessage);
		});
	}

	return true;
}

bool SamplePlugin::UnhookGameEvents()
{
	g_pGameEventManager->RemoveListener(this);

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

void SamplePlugin::OnDispatchConCommandHook(ConCommandHandle hCommand, const CCommandContext &aContext, const CCommand &aArgs)
{
	if(IsChannelEnabled(LV_DETAILED))
	{
		DetailedFormat("%s(%d, %d, %s)\n", __FUNCTION__, hCommand.GetIndex(), aContext.GetPlayerSlot().Get(), aArgs.GetCommandString());
	}

	auto aPlayerSlot = aContext.GetPlayerSlot();

	const char *pszArg0 = aArgs.Arg(0);

	static const char szSayCommand[] = "say";

	size_t nSayNullTerminated = sizeof(szSayCommand) - 1;

	if(!V_strncmp(pszArg0, (const char *)szSayCommand, nSayNullTerminated))
	{
		if(!pszArg0[nSayNullTerminated] || !V_strcmp(&pszArg0[nSayNullTerminated], "_team"))
		{
			const char *pszArg1 = aArgs.Arg(1);

			// Skip spaces.
			while(*pszArg1 == ' ')
			{
				pszArg1++;
			}

			bool bIsSilent = *pszArg1 == Sample::ChatCommandSystem::GetSilentTrigger();

			if(bIsSilent || *pszArg1 == Sample::ChatCommandSystem::GetPublicTrigger())
			{
				pszArg1++; // Skip a command character.

				// Print a chat message before.
				if(!bIsSilent && g_pCVar)
				{
					SH_CALL(g_pCVar, &ICvar::DispatchConCommand)(hCommand, aContext, aArgs);
				}

				// Call the handler.
				{
					size_t nArg1Length = 0;

					// Get a length to a first space.
					while(pszArg1[nArg1Length] && pszArg1[nArg1Length] != ' ')
					{
						nArg1Length++;
					}

					CUtlVector<CUtlString> vecArgs;

					V_SplitString(pszArg1, " ", vecArgs);

					for(auto &sArg : vecArgs)
					{
						sArg.Trim(' ');
					}

					if(IsChannelEnabled(LV_DETAILED))
					{
						const auto &aConcat = s_aEmbedConcat, 
						           &aConcat2 = s_aEmbed2Concat;

						CBufferStringGrowable<1024> sBuffer;

						sBuffer.Format("Handle a chat command:\n");
						aConcat.AppendToBuffer(sBuffer, "Player slot", aPlayerSlot.Get());
						aConcat.AppendToBuffer(sBuffer, "Is silent", bIsSilent);
						aConcat.AppendToBuffer(sBuffer, "Arguments");

						for(const auto &sArg : vecArgs)
						{
							const char *pszMessageConcat[] = {aConcat2.m_aStartWith, "\"", sArg.Get(), "\"", aConcat2.m_aEnd};

							sBuffer.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);
						}

						Detailed(sBuffer);
					}

					Sample::ChatCommandSystem::Handle(aPlayerSlot, bIsSilent, vecArgs);
				}

				RETURN_META(MRES_SUPERCEDE);
			}
		}
	}

	RETURN_META(MRES_IGNORED);
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
	sProtoOutput.SetLength(sProtoOutput.GetTotalNumber() - (V_strlen(aConcat.m_aEndAndNextLine) - 1)); // Strip the last next line, leaving the end.

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

void SamplePlugin::SendChatMessage(IRecipientFilter *pFilter, int iEntityIndex, bool bIsChat, const char *pszChatMessageFormat, const char *pszParam1, const char *pszParam2, const char *pszParam3, const char *pszParam4)
{
	auto *pSayText2Message = m_pSayText2Message;

	if(IsChannelEnabled(LV_DETAILED))
	{
		const auto &aConcat = s_aEmbedConcat;

		CBufferStringGrowable<1024> sBuffer;

		sBuffer.Format("Send chat message (%s):\n", pSayText2Message->GetUnscopedName());
		aConcat.AppendToBuffer(sBuffer, "Entity index", iEntityIndex);
		aConcat.AppendToBuffer(sBuffer, "Is chat", bIsChat);
		aConcat.AppendStringToBuffer(sBuffer, "Chat message", pszChatMessageFormat);

		if(pszParam1 && *pszParam1)
		{
			aConcat.AppendStringToBuffer(sBuffer, "Parameter #1", pszParam1);
		}

		if(pszParam2 && *pszParam2)
		{
			aConcat.AppendStringToBuffer(sBuffer, "Parameter #2", pszParam2);
		}

		if(pszParam3 && *pszParam3)
		{
			aConcat.AppendStringToBuffer(sBuffer, "Parameter #3", pszParam3);
		}

		if(pszParam4 && *pszParam4)
		{
			aConcat.AppendStringToBuffer(sBuffer, "Parameter #4", pszParam4);
		}

		Detailed(sBuffer);
	}

	auto *pMessage = pSayText2Message->AllocateMessage()->ToPB<CUserMessageSayText2>();

	pMessage->set_entityindex(iEntityIndex);
	pMessage->set_chat(bIsChat);
	pMessage->set_messagename(pszChatMessageFormat);
	pMessage->set_param1(pszParam1);
	pMessage->set_param2(pszParam2);
	pMessage->set_param3(pszParam3);
	pMessage->set_param4(pszParam4);

	g_pGameEventSystem->PostEventAbstract(-1, false, pFilter, pSayText2Message, pMessage, 0);

	delete pMessage;
}

void SamplePlugin::SendTextMessage(IRecipientFilter *pFilter, int iDestination, size_t nParamCount, const char *pszParam, ...)
{
	auto *pTextMsg = m_pTextMsgMessage;

	if(IsChannelEnabled(LV_DETAILED))
	{
		const auto &aConcat = s_aEmbedConcat;

		CBufferStringGrowable<1024> sBuffer;

		sBuffer.Format("Send message (%s):\n", pTextMsg->GetUnscopedName());
		aConcat.AppendToBuffer(sBuffer, "Destination", iDestination);
		aConcat.AppendToBuffer(sBuffer, "Parameter", pszParam);
		Detailed(sBuffer);
	}

	auto *pMessage = pTextMsg->AllocateMessage()->ToPB<CUserMessageTextMsg>();

	pMessage->set_dest(iDestination);
	pMessage->add_param(pszParam);
	nParamCount--;

	// Parse incoming parameters.
	if(nParamCount)
	{
		va_list aParams;

		va_start(aParams, pszParam);

		for(size_t n = 0; n < nParamCount; n++)
		{
			pMessage->add_param(va_arg(aParams, const char *));
		}

		va_end(aParams);
	}

	g_pGameEventSystem->PostEventAbstract(-1, false, pFilter, pTextMsg, pMessage, 0);

	delete pMessage;
}

void SamplePlugin::OnStartupServer(CNetworkGameServerBase *pNetServer, const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession)
{
	SH_ADD_HOOK_MEMFUNC(CNetworkGameServerBase, ConnectClient, pNetServer, this, &SamplePlugin::OnConnectClientHook, true);

	// Initialize & hook game evetns.
	// Initialize network messages.
	{
		char sMessage[256];

		if(RegisterSource2Server(sMessage, sizeof(sMessage)))
		{
			Assert(HookGameEvents());
		}
		else
		{
			WarningFormat("%s\n", sMessage);
		}

		if(!RegisterNetMessages(sMessage, sizeof(sMessage)))
		{
			WarningFormat("%s\n", sMessage);
		}
	}

	if(IsChannelEnabled(LS_DETAILED))
	{
		const auto &aConcat = s_aEmbedConcat;

		CBufferStringGrowable<1024> sMessage;

#ifndef _WIN32
		try
		{
			sMessage.Format("Receive %s message:\n", config.GetTypeName().c_str());
			DumpProtobufMessage(aConcat, sMessage, config);
		}
		catch(const std::exception &aError)
		{
			sMessage.Format("Receive a proto message: %s\n", aError.what());
		}

#endif
		sMessage.AppendFormat("Register globals:\n");
		DumpRegisterGlobals(aConcat, sMessage);

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
		DumpServerSideClient(aConcat, sMessage, pClient);

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
