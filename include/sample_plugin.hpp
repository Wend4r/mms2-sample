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
#	include <sample/chatcommandsystem.hpp>
#	include <sample/pathresolver.hpp>
#	include <sample/provider.hpp>
#	include <concat.hpp>

#	include <string>

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
#	include <netmessages.h>
#	include <network_connection.pb.h>
#	include <playerslot.h>
#	include <tier0/bufferstring.h>
#	include <tier0/strtools.h>
#	include <tier1/convar.h>
#	include <tier1/utlmap.h>
#	include <tier1/utlsymbollarge.h>
#	include <tier1/utlvector.h>

#	define SAMPLE_LOGGINING_COLOR {127, 255, 0, 191} // Green (Chartreuse)

#	define SAMPLE_GAME_BASE_DIR "addons" CORRECT_PATH_SEPARATOR_S META_PLUGIN_PREFIX
#	define SAMPLE_GAME_EVENTS_FILES "resource" CORRECT_PATH_SEPARATOR_S "*.gameevents"
#	define SAMPLE_GAME_TRANSLATIONS_FILES "translations" CORRECT_PATH_SEPARATOR_S "*.phrases.*"
#	define SAMPLE_GAME_LANGUAGES_FILES "configs" CORRECT_PATH_SEPARATOR_S "languages.*"
#	define SAMPLE_BASE_PATHID "GAME"

#	define SAMPLE_CLIENT_CVAR_NAME_LANGUAGE "cl_language"

class CBasePlayerController;
class INetworkMessageInternal;

class Sample_Plugin final : public ISmmPlugin, public IMetamodListener, public ISample, public CBaseGameSystem, public IGameEventListener2, 
                            public Sample::ChatCommandSystem, public Sample::CPathResolver, public Sample::Provider, virtual public CLogger, public Translations
{
public:
	Sample_Plugin();

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
	CGameEntitySystem **GetGameEntitySystemPointer() const override;
	CBaseGameSystemFactory **GetFirstGameSystemPointer() const override;
	CUtlStringMap<IGameSystem::FactoryInfo_t> *GetGameSystemFactoriesPointer() const override;
	CUtlVector<AddedGameSystem_t> *GetGameSystemsPointer() const override;
	CGameSystemEventDispatcher **GetGameSystemEventDispatcherPointer() const override;
	CGameSystemEventDispatcher *GetOutOfGameEventDispatcher() const override;
	IGameEventManager2 **GetGameEventManagerPointer() const override;

	class CLanguage : public ISample::ILanguage
	{
		friend class Sample_Plugin;

	public:
		CLanguage(CUtlSymbolLarge sInitName = CUtlSymbolLarge(), const char *pszInitCountryCode = "en") : m_sName(sInitName), m_sCountryCode(pszInitCountryCode) {}

	public:
		const char *GetName() const override { return m_sName.String(); };
		const char *GetCountryCode() const override { return m_sCountryCode; };

	protected:
		void SetName(const CUtlSymbolLarge &symbolLarge) { m_sName = symbolLarge; };
		void SetCountryCode(const char *psz) { m_sCountryCode = psz; };

	private:
		CUtlSymbolLarge m_sName;
		CUtlString m_sCountryCode;
	}; // Sample_Plugin::CLanguage

	class CPlayerBase : public IPlayerBase
	{
		friend class Sample_Plugin;

	public:
		CPlayerBase();

	public: // ISample::IPlayerLanguageCallbacks
		bool AddLanguageListener(IPlayerLanguageListener *pListener) override;
		bool RemoveLanguageListener(IPlayerLanguageListener *pListener) override;

	public: // ISample::IPlayerLanguage
		const ILanguage *GetLanguage() const override;
		void SetLanguage(const ILanguage *pData) override;

	public: // IMenuSystem::IPlayerBase
		bool IsConnected() const override;
		CServerSideClient *GetServerSideClient() override;

	public:
		virtual void OnConnected(CServerSideClient *pClient);
		virtual void OnDisconnected(CServerSideClient *pClient, ENetworkDisconnectionReason eReason);

	public:
		virtual void OnLanguageChanged(CPlayerSlot aSlot, CLanguage *pData);

	public:
		struct TranslatedPhrase
		{
			const Translations::CPhrase::CFormat *m_pFormat;
			const Translations::CPhrase::CContent *m_pContent;
		};

		void TranslatePhrases(const Translations *pTranslations, const CLanguage &aServerLanguage, CUtlVector<CUtlString> &vecMessages);
		const TranslatedPhrase &GetYourArgumentPhrase() const;

	private:
		CServerSideClient *m_pServerSideClient;

	private:
		const ILanguage *m_pLanguage;
		CUtlVector<IPlayerLanguageListener *> m_vecLanguageCallbacks;

	private:
		TranslatedPhrase m_aYourArgumentPhrase;
	}; // Sample_Plugin::CPlayerBase

	const ISample::ILanguage *GetServerLanguage() const override;
	const ISample::ILanguage *GetLanguageByName(const char *psz) const override;
	IPlayerBase *GetPlayerBase(const CPlayerSlot &aSlot) override;
	CPlayerBase &GetPlayerData(const CPlayerSlot &aSlot);

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

public: // Path resolver.
	bool InitPathResolver(char *error = nullptr, size_t maxlen = 0);
	bool ClearPathResolver(char *error = nullptr, size_t maxlen = 0);

private:
	std::string m_sBaseGameDirectory = SAMPLE_GAME_BASE_DIR;

public: // Utils.
	bool InitProvider(char *error = nullptr, size_t maxlen = 0);
	bool LoadProvider(char *error = nullptr, size_t maxlen = 0);
	bool UnloadProvider(char *error = nullptr, size_t maxlen = 0);

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

public: // Languages.
	bool ParseLanguages(char *error = nullptr, size_t maxlen = 0);
	bool ParseLanguages(KeyValues3 *pRoot, CUtlVector<CUtlString> &vecMessages);
	bool ClearLanguages(char *error = nullptr, size_t maxlen = 0);

public: // Translations.
	bool ParseTranslations(char *error = nullptr, size_t maxlen = 0);
	bool ClearTranslations(char *error = nullptr, size_t maxlen = 0);

public: // Event actions.
	bool ParseGameEvents();
	bool ParseGameEvents(KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages); // Parse the structure of events.
	bool ClearGameEvents();

	bool HookGameEvents();
	bool UnhookGameEvents();

private: // Commands.
	CON_COMMAND_MEMBER_F(Sample_Plugin, "mm_" META_PLUGIN_PREFIX "_reload_gamedata", OnReloadGameDataCommand, "Reload gamedata configs", FCVAR_LINKED_CONCOMMAND);

private: // ConVars. See the constructor
	CConVar<bool> m_aEnableFrameDetailsConVar;
	CConVar<bool> m_aEnableGameEventsDetaillsConVar;

public: // SourceHooks.
	void OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *);
	void OnDispatchConCommandHook(ConCommandRef hCommand, const CCommandContext &aContext, const CCommand &aArgs);
	CServerSideClientBase *OnConnectClientHook(const char *pszName, ns_address *pAddr, void *pNetInfo, C2S_CONNECT_Message *pConnectMsg, const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence);
	bool OnProcessRespondCvarValueHook(const CCLCMsg_RespondCvarValue_t &aMessage);
	void OnDisconectClientHook(ENetworkDisconnectionReason eReason);

public: // Dump ones.
	static CBufferStringN<1024> DumpProtobufMessage(const ConcatLineString &aConcat, const google::protobuf::Message &aMessage);
	static void DumpEngineLoopState(const ConcatLineString &aConcat, CBufferString &sOutput, const EngineLoopState_t &aMessage);
	static void DumpEntityList(const ConcatLineString &aConcat, CBufferString &sOutput, const CUtlVector<CEntityHandle> &vecEntityList);
	static void DumpEventSimulate(const ConcatLineString &aConcat, const ConcatLineString &aConcat2, CBufferString &sOutput, const EventSimulate_t &aMessage);
	static void DumpEventFrameBoundary(const ConcatLineString &aConcat, CBufferString &sOutput, const EventFrameBoundary_t &aMessage);
	static void DumpServerSideClient(const ConcatLineString &aConcat, CBufferString &sOutput, CServerSideClientBase *pClient);
	static void DumpDisconnectReason(const ConcatLineString &aConcat, CBufferString &sOutput, ENetworkDisconnectionReason eReason);

public: // Utils.
	void SendCvarValueQuery(IRecipientFilter *pFilter, const char *pszName, int iCookie);
	void SendChatMessage(IRecipientFilter *pFilter, int iEntityIndex, bool bIsChat, const char *pszChatMessageFormat, const char *pszParam1 = "", const char *pszParam2 = "", const char *pszParam3 = "", const char *pszParam4 = "");
	void SendTextMessage(IRecipientFilter *pFilter, int iDestination, size_t nParamCount, const char *pszParam, ...);

protected: // Handlers.
	void OnStartupServer(CNetworkGameServerBase *pNetServer, const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession);
	void OnConnectClient(CNetworkGameServerBase *pNetServer, CServerSideClientBase *pClient, const char *pszName, ns_address *pAddr, void *pNetInfo, C2S_CONNECT_Message *pConnectMsg, const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence);
	bool OnProcessRespondCvarValue(CServerSideClientBase *pClient, const CCLCMsg_RespondCvarValue_t &aMessage);
	void OnDisconectClient(CServerSideClientBase *pClient, ENetworkDisconnectionReason eReason);

protected: // ConVar symbols.
	CUtlSymbolLarge GetConVarSymbol(const char *pszName);
	CUtlSymbolLarge FindConVarSymbol(const char *pszName) const;

private: // ConVar (hash)map.
	CUtlSymbolTableLarge_CI m_tableConVars;
	CUtlMap<CUtlSymbolLarge, int> m_mapConVarCookies;

protected: // Language symbols.
	CUtlSymbolLarge GetLanguageSymbol(const char *pszName);
	CUtlSymbolLarge FindLanguageSymbol(const char *pszName) const;

private: // Language (hash)map.
	CUtlSymbolTableLarge_CI m_tableLanguages;
	CUtlMap<CUtlSymbolLarge, CLanguage> m_mapLanguages;

private: // Fields.
	CGameSystemStaticFactory<Sample_Plugin> *m_pFactory;

	INetworkMessageInternal *m_pGetCvarValueMessage;
	INetworkMessageInternal *m_pSayText2Message;
	INetworkMessageInternal *m_pTextMsgMessage;

	CUtlVector<CUtlString> m_vecGameEvents;

	CLanguage m_aServerLanguage;
	CUtlVector<CLanguage> m_vecLanguages;

	CPlayerBase m_aPlayers[ABSOLUTE_PLAYER_LIMIT];
}; // Sample_Plugin

extern Sample_Plugin *g_pSamplePlugin;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_SAMPLE_PLUGIN_HPP_
