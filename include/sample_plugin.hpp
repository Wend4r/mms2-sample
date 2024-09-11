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

#	include <sample/provider.hpp>
#	include <concat.hpp>

#	include <logger.hpp>

#	include <ISmmPlugin.h>

#	include <igamesystem.h>
#	include <igamesystemfactory.h>
#	include <iloopmode.h>
#	include <iserver.h>
#	include <tier0/bufferstring.h>
#	include <tier0/strtools.h>
#	include <tier1/utlvector.h>

#	define SAMPLE_LOGGINING_COLOR {127, 255, 0, 191} // Green (Chartreuse)
#	define SAMPLE_BASE_DIR "addons" CORRECT_PATH_SEPARATOR_S META_PLUGIN_PREFIX
#	define SAMPLE_BASE_PATHID "GAME"

class SamplePlugin final : public ISmmPlugin, public IMetamodListener, public CBaseGameSystem, 
                           public Sample::Provider, public Logger
{
public:
	SamplePlugin();

public: // ISmmPlugin
	const char *GetAuthor() override;
	const char *GetName() override;
	const char *GetDescription() override;
	const char *GetURL() override;
	const char *GetLicense() override;
	const char *GetVersion() override;
	const char *GetDate() override;
	const char *GetLogTag() override;

public: // IMetamodListener
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late) override;
	bool Unload(char *error, size_t maxlen) override;
	bool Pause(char *error, size_t maxlen) override;
	bool Unpause(char *error, size_t maxlen) override;
	void AllPluginsLoaded() override;

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

public: // Utils.
	bool InitProvider(char *error, size_t maxlen);
	bool LoadProvider(char *error, size_t maxlen);
	bool UnloadProvider(char *error, size_t maxlen);

public:
	bool RegisterGameResource(char *error, size_t maxlen);
	bool UnregisterGameResource(char *error, size_t maxlen);

public:
	bool RegisterGameFactory(char *error, size_t maxlen);
	bool UnregisterGameFactory(char *error, size_t maxlen);

private: // Commands.
	CON_COMMAND_MEMBER_F(SamplePlugin, "mm_" META_PLUGIN_PREFIX "_reload_gamedata", OnReloadGameDataCommand, "Reload gamedata configs", FCVAR_LINKED_CONCOMMAND);

public: // SourceHooks.
	void OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *);
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

public: // Handlers.
	void OnStartupServer(CNetworkGameServerBase *pNetServer, const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession);
	void OnConnectClient(CNetworkGameServerBase *pNetServer, CServerSideClientBase *pClient, const char *pszName, ns_address *pAddr, int socket, CCLCMsg_SplitPlayerConnect_t *pSplitPlayer, const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence);
	void OnDisconectClient(CServerSideClientBase *pClient, ENetworkDisconnectionReason eReason);

protected: // Fields.
	IGameSystemFactory *m_pFactory = NULL;
}; // SamplePlugin

extern SamplePlugin *g_pSamplePlugin;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_SAMPLE_PLUGIN_HPP_
