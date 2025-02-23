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

#ifndef _INCLUDE_METAMOD_SOURCE_SAMPLE_PROVIDER_HPP_
#	define _INCLUDE_METAMOD_SOURCE_SAMPLE_PROVIDER_HPP_

#	pragma once

#	include <stddef.h>
#	include <stdint.h>

#	include <tier0/dbg.h>
#	include <tier0/platform.h>
#	include <tier0/utlscratchmemory.h>
#	include <tier1/utldelegateimpl.h>
#	include <tier1/utlmap.h>
#	include <tier1/utlstringmap.h>
#	include <entity2/entitykeyvalues.h>
#	include <igamesystem.h>

#	include <gamedata.hpp> // GameData

#	define SAMPLE_PROVIDER_BASE_DIR "gamedata"
#	define SAMPLE_PROVIDER_GAMERESOURCE_FILENAME SAMPLE_PROVIDER_BASE_DIR CORRECT_PATH_SEPARATOR_S "gameresource.games.*"
#	define SAMPLE_PROVIDER_GAMESYSTEM_FILENAME SAMPLE_PROVIDER_BASE_DIR CORRECT_PATH_SEPARATOR_S "gamesystem.games.*"
#	define SAMPLE_PROVIDER_SOURCE2SERVER_FILENAME SAMPLE_PROVIDER_BASE_DIR CORRECT_PATH_SEPARATOR_S "source2server.games.*"

class CBaseGameSystemFactory;
class CGameEventManager;
class CGameSystemEventDispatcher;
struct AddedGameSystem_t;

namespace Sample
{
	class Provider : public IGameData
	{
	public:
		Provider();

	public:
		bool Init(GameData::CBufferStringVector &vecMessages);
		bool Load(const char *pszBaseDir, const char *pszPathID, GameData::CBufferStringVector &vecMessages);
		bool Destroy(GameData::CBufferStringVector &vecMessages);

	protected:
		CUtlSymbolLarge GetSymbol(const char *pszText);
		CUtlSymbolLarge FindSymbol(const char *pszText) const;

	public: // IGameData
		const DynLibUtils::CModule *FindLibrary(const char *pszName) const;

	protected:
		bool LoadGameData(const char *pszBaseGameDir, const char *pszPathID, GameData::CBufferStringVector &vecMessages);

	public:
		class GameDataStorage
		{
		public:
			bool Load(IGameData *pRoot, const char *pszBaseGameDir, const char *pszPathID, GameData::CBufferStringVector &vecMessages);

		protected:
			bool LoadGameResource(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages);
			bool LoadGameSystem(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages);
			bool LoadSource2Server(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages);

		public:
			class CGameResource
			{
			public:
				CGameResource();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages);
				void Reset();

			public:
				ptrdiff_t GetEntitySystemOffset() const;

			private:
				GameData::Config::Offsets::ListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Offsets.
				ptrdiff_t m_nEntitySystemOffset = -1;
			}; // Sample::Provider::GameDataStorage::CGameResource

			class CGameSystem
			{
			public:
				CGameSystem();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages);
				void Reset();

			public:
				CBaseGameSystemFactory **GetFirstPointer() const;
				CUtlStringMap<IGameSystem::FactoryInfo_t> *GetFactories() const;
				CUtlVector<AddedGameSystem_t> *GetList() const;
				CGameSystemEventDispatcher **GetEventDispatcher() const;

			private:
				GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				CBaseGameSystemFactory **m_ppFirst = nullptr;
				CUtlStringMap<IGameSystem::FactoryInfo_t> *m_pGameSystemFactories = nullptr;
				CUtlVector<AddedGameSystem_t> *m_pGameSystems = nullptr;
				CGameSystemEventDispatcher **m_ppEventDispatcher = nullptr;
			}; // Sample::Provider::CGameSystem

			class CSource2Server
			{
			public:
				CSource2Server();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages);
				void Reset();

			public:
				CGameEventManager **GetGameEventManagerPointer() const;

			private:
				GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				CGameEventManager **m_ppGameEventManager = nullptr;
			}; // Sample::Provider::GameDataStorage::CSource2Server

			const CGameResource &GetGameResource() const;
			const CGameSystem &GetGameSystem() const;
			const CSource2Server &GetSource2Server() const;

		private:
			CGameResource m_aGameResource;
			CGameSystem m_aGameSystem;
			CSource2Server m_aSource2Server;
		}; // Sample::Provider::GameDataStorage

		const GameDataStorage &GetGameDataStorage() const;

	private:
		CUtlSymbolTableLarge_CI m_aSymbolTable;
		CUtlMap<CUtlSymbolLarge, DynLibUtils::CModule *> m_mapLibraries;

	private:
		GameDataStorage m_aStorage;

	private:
		DynLibUtils::CModule m_aEngine2Library, 
		                     m_aFileSystemSTDIOLibrary, 
		                     m_aServerLibrary;
	}; // Provider
}; // Sample

#endif // _INCLUDE_METAMOD_SOURCE_SAMPLE_PROVIDER_HPP_
