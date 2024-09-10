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
#define _INCLUDE_METAMOD_SOURCE_SAMPLE_PROVIDER_HPP_

#	include <stddef.h>
#	include <stdint.h>

#	include <tier0/dbg.h>
#	include <tier0/platform.h>
#	include <tier0/utlscratchmemory.h>
#	include <tier1/utldelegateimpl.h>
#	include <tier1/utlmap.h>
#	include <entity2/entitykeyvalues.h>

#	include <gamedata.hpp> // GameData

#	define SAMPLE_GAMECONFIG_FOLDER_DIR "gamedata"
#	define SAMPLE_GAMECONFIG_SOURCE2SERVER_FILENAME "source2server.games.*"

class CGameEventManager;

namespace Sample
{
	class Provider : public IGameData
	{
	public:
		Provider();

	public:
		bool Init(GameData::CBufferStringVector &vecMessages);
		bool Load(const char *pszBaseDir, GameData::CBufferStringVector &vecMessages);
		bool Destroy(GameData::CBufferStringVector &vecMessages);

	public:
		CUtlSymbolLarge GetSymbol(const char *pszText);
		CUtlSymbolLarge FindSymbol(const char *pszText) const;

	public: // IGameData
		const DynLibUtils::CModule *FindLibrary(const char *pszName) const;

	protected:
		bool LoadGameData(const char *pszBaseDir, GameData::CBufferStringVector &vecMessages);

	public:
		class GameDataStorage
		{
		public:
			bool Load(IGameData *pRoot, const char *pszBaseConfigDir, GameData::CBufferStringVector &vecMessages);

		protected:
			bool LoadSource2Server(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages);

		public:
			class CSource2Server
			{
			public:
				CSource2Server();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages);
				void Reset();

			public:
				CGameEventManager **GetGameEventManagerPtr() const;

			private:
				GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config m_aGameConfig;

			private: // Signatures.
				CGameEventManager **m_ppGameEventManager = nullptr;
			}; // CSource2Server

			const CSource2Server &GetSource2Server() const;

		private:
			CSource2Server m_aSource2Server;
		}; // GameDataStorage

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
