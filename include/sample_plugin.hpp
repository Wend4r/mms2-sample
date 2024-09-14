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

#ifndef _INCLUDE_METAMOD_SOURCE_SAMPLE_PLUGIN_HPP_
#	define _INCLUDE_METAMOD_SOURCE_SAMPLE_PLUGIN_HPP_

#	pragma once

#	include <isample.hpp>
#	include <sample/chat_command_system.hpp>
#	include <sample/provider.hpp>
#	include <concat.hpp>

#	include <logger.hpp>
#	include <translations.hpp>

#	include <ISmmPlugin.h>

#	include <bitvec.h>
#	include <const.h>
#	include <igameevents.h>
#	include <igamesystem.h>
#	include <igamesystemfactory.h>
#	include <iloopmode.h>
#	include <iserver.h>
#	include <playerslot.h>
#	include <tier0/bufferstring.h>
#	include <tier0/strtools.h>
#	include <tier1/convar.h>
#	include <tier1/utlvector.h>

#	define SAMPLE_LOGGINING_COLOR {127, 255, 0, 191} // Green (Chartreuse)

#	define SAMPLE_BASE_DIR "addons" CORRECT_PATH_SEPARATOR_S META_PLUGIN_PREFIX
#	define SAMPLE_GAME_EVENTS_FILES "resource/*.gameevents"
#	define SAMPLE_GAME_TRANSLATIONS_FILES "translations/*.pharses.*"
#	define SAMPLE_BASE_PATHID "GAME"

#	define SAMPLE_EXAMPLE_CHAT_COMMAND "example"

class CBasePlayerController;
class INetworkMessageInternal;

class SamplePlugin final : public ISmmPlugin, public IMetamodListener, public ISample, public CBaseGameSystem, public IGameEventListener2, 
                           public Sample::ChatCommandSystem, public Sample::Provider, virtual public Logger
{
public:
	SamplePlugin();

public: // ISmmPlugin
	bool Load(PluginId id, ISmmAPI *ismm, char *error = nullptr, size_t maxlen = 0, bool late = true) override;
	bool Unload(char *error, size_t maxlen) override;
	bool Pause(char *error, size_t maxlen) override;
	bool Unpause(char *error, size_t maxlen) override;
	void AllPluginsLoaded() override;

	const char *GetAuthor() override;
	const char *GetName() override;
	const char *GetDescription() override;
	const char *GetURL() override;
	const char *GetLicense() override;
	const char *GetVersion() override;
	const char *GetDate() override;
	const char *GetLogTag() override;

public: // IMetamodListener
	void *OnMetamodQuery(const char *iface, int *ret) override;


public: // ISample
	CGameEntitySystem **GetGameEntitySystemPointer() override;
	CBaseGameSystemFactory **GetFirstGameSystemPointer() override;
	IGameEventManager2 **GetGameEventManagerPointer() override;

public: // CBaseGameSystem
	bool Init() override;
	void PostInit() override;
	void Shutdown() override;

	GS_EVENT(GameInit);
	GS_EVENT(GameShutdown);
	GS_EVENT(GamePostInit);
	GS_EVENT(GamePreShutdown);
	GS_EVENT(BuildGameSessionManifest);
	GS_EVENT(GameActivate);
	GS_EVENT(ClientFullySignedOn);
	GS_EVENT(Disconnect);
	GS_EVENT(GameDeactivate);
	GS_EVENT(SpawnGroupPrecache);
	GS_EVENT(SpawnGroupUncache);
	GS_EVENT(PreSpawnGroupLoad);
	GS_EVENT(PostSpawnGroupLoad);
	GS_EVENT(PreSpawnGroupUnload);
	GS_EVENT(PostSpawnGroupUnload);
	GS_EVENT(ActiveSpawnGroupChanged);
	GS_EVENT(ClientPostDataUpdate);
	GS_EVENT(ClientPreRender);
	GS_EVENT(ClientPreEntityThink);
	GS_EVENT(ClientUpdate);
	GS_EVENT(ClientPostRender);
	GS_EVENT(ServerPreEntityThink);
	GS_EVENT(ServerPostEntityThink);
	GS_EVENT(ServerPreClientUpdate);
	GS_EVENT(ServerGamePostSimulate);
	GS_EVENT(ClientGamePostSimulate);
	GS_EVENT(GameFrameBoundary);
	GS_EVENT(OutOfGameFrameBoundary);
	GS_EVENT(SaveGame);
	GS_EVENT(RestoreGame);

public: // IGameEventListener2
	void FireGameEvent(IGameEvent *event) override;

public: // Utils.
	bool InitProvider(char *error = nullptr, size_t maxlen = 0);
	bool LoadProvider(char *error = nullptr, size_t maxlen = 0);
	bool UnloadProvider(char *error = nullptr, size_t maxlen = 0);

public:
	bool LoadTranslations(char *error = nullptr, size_t maxlen = 0);
	bool UnloadTranslations(char *error = nullptr, size_t maxlen = 0);

public: // Game Resource.
	bool RegisterGameResource(char *error = nullptr, size_t maxlen = 0);
	bool UnregisterGameResource(char *error = nullptr, size_t maxlen = 0);

public: // Game Factory.
	bool RegisterGameFactory(char *error = nullptr, size_t maxlen = 0);
	bool UnregisterGameFactory(char *error = nullptr, size_t maxlen = 0);

public: // Source 2 Server.
	bool RegisterSource2Server(char *error = nullptr, size_t maxlen = 0);
	bool UnregisterSource2Server(char *error = nullptr, size_t maxlen = 0);

public: // Network Messages.
	bool RegisterNetMessages(char *error = nullptr, size_t maxlen = 0);
	bool UnregisterNetMessages(char *error = nullptr, size_t maxlen = 0);

public: // Event actions.
	bool ParseGameEvents();
	bool ParseGameEvents(KeyValues3 *pEvents, CUtlVector<CUtlString> &vecMessages); // Parse the structure of events.
	bool ClearGameEvents();

	bool HookGameEvents();
	bool UnhookGameEvents();

private: // Commands.
	CON_COMMAND_MEMBER_F(SamplePlugin, "mm_" META_PLUGIN_PREFIX "_reload_gamedata", OnReloadGameDataCommand, "Reload gamedata configs", FCVAR_LINKED_CONCOMMAND);

private: // ConVars. See the constructor
	ConVar<bool> m_aEnableFrameDetailsConVar;
	ConVar<bool> m_aEnableGameEventsDetaillsConVar;

public: // SourceHooks.
	void OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *);
	void OnDispatchConCommandHook(ConCommandHandle hCommand, const CCommandContext &aContext, const CCommand &aArgs);
	CServerSideClientBase *OnConnectClientHook(const char *pszName, ns_address *pAddr, int socket, CCLCMsg_SplitPlayerConnect_t *pSplitPlayer, const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence);
	void OnDisconectClientHook(ENetworkDisconnectionReason eReason);

public: // Dump ones.
	void DumpProtobufMessage(const ConcatLineString &aConcat, CBufferString &sOutput, const google::protobuf::Message &aMessage);
	void DumpEngineLoopState(const ConcatLineString &aConcat, CBufferString &sOutput, const EngineLoopState_t &aMessage);
	void DumpEntityList(const ConcatLineString &aConcat, CBufferString &sOutput, const CUtlVector<CEntityHandle> &vecEntityList);
	void DumpEventSimulate(const ConcatLineString &aConcat, const ConcatLineString &aConcat2, CBufferString &sOutput, const EventSimulate_t &aMessage);
	void DumpEventFrameBoundary(const ConcatLineString &aConcat, CBufferString &sOutput, const EventFrameBoundary_t &aMessage);
	void DumpServerSideClient(const ConcatLineString &aConcat, CBufferString &sOutput, CServerSideClientBase *pClient);
	void DumpDisconnectReason(const ConcatLineString &aConcat, CBufferString &sOutput, ENetworkDisconnectionReason eReason);

public: // Utils.
	void SendChatMessage(IRecipientFilter *pFilter, int iEntityIndex, bool bIsChat, const char *pszChatMessageFormat, const char *pszParam1 = "", const char *pszParam2 = "", const char *pszParam3 = "", const char *pszParam4 = "");
	void SendTextMessage(IRecipientFilter *pFilter, int iDestination, size_t nParamCount, const char *pszParam, ...);

public: // Handlers.
	void OnStartupServer(CNetworkGameServerBase *pNetServer, const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession);
	void OnChatCommandExample(CPlayerSlot nSlot, CUtlVector<CUtlString> &vecArgs);
	void OnConnectClient(CNetworkGameServerBase *pNetServer, CServerSideClientBase *pClient, const char *pszName, ns_address *pAddr, int socket, CCLCMsg_SplitPlayerConnect_t *pSplitPlayer, const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence);
	void OnDisconectClient(CServerSideClientBase *pClient, ENetworkDisconnectionReason eReason);

protected: // Fields.
	IGameSystemFactory *m_pFactory = NULL;
	INetworkMessageInternal *m_pSayText2Message = NULL;
	INetworkMessageInternal *m_pTextMsgMessage = NULL;
	CUtlVector<CUtlString> m_vecGameEvents;
	CUtlVector<Translations> m_vecTranslations;
}; // SamplePlugin

extern SamplePlugin *g_pSamplePlugin;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_SAMPLE_PLUGIN_HPP_
