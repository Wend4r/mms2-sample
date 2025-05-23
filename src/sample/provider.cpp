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

#include <sample/provider.hpp>
#include <globals.hpp>

#include <filesystem.h>
#include <tier0/commonmacros.h>
#include <tier0/keyvalues3.h>
#include <tier0/strtools.h>

#include <any_config.hpp>

Sample::Provider::Provider()
 :  m_mapLibraries(DefLessFunc(const CUtlSymbolLarge))
{
}

bool Sample::Provider::Init(GameData::CBufferStringVector &vecMessages)
{
	// Enigne 2.
	{
		const char szEngineModuleName[] = "engine2";

		if(!m_aEngine2Library.InitFromMemory(g_pEngineServer))
		{
			static const char *s_pszMessageConcat[] = {"Failed to ", "get \"", szEngineModuleName, "\" module"};

			vecMessages.AddToTail({s_pszMessageConcat});
		}

		m_mapLibraries.Insert(GetSymbol(szEngineModuleName), &m_aEngine2Library);
	}

	// File System.
	{
		const char szFileSystemSTDIOModuleName[] = "filesystem_stdio";

		if(!m_aFileSystemSTDIOLibrary.InitFromMemory(g_pFullFileSystem))
		{
			static const char *s_pszMessageConcat[] = {"Failed to ", "get \"", szFileSystemSTDIOModuleName, "\" module"};

			vecMessages.AddToTail({s_pszMessageConcat});
		}

		m_mapLibraries.Insert(GetSymbol(szFileSystemSTDIOModuleName), &m_aFileSystemSTDIOLibrary);
	}

	// Server.
	{
		const char szServerModuleName[] = "server";

		if(!m_aServerLibrary.InitFromMemory(g_pSource2Server))
		{
			static const char *s_pszMessageConcat[] = {"Failed to ", "get \"", szServerModuleName, "\" module"};

			vecMessages.AddToTail({s_pszMessageConcat});
		}

		m_mapLibraries.Insert(GetSymbol(szServerModuleName), &m_aServerLibrary);
	}

	return true;
}

bool Sample::Provider::Load(const char *pszBaseDir, const char *pszPathID, GameData::CBufferStringVector &vecMessages)
{
	if(!LoadGameData(pszBaseDir, pszPathID, vecMessages))
	{
		return false;
	}

	// ...

	return true;
}

bool Sample::Provider::Destroy(GameData::CBufferStringVector &vecMessages)
{
	m_mapLibraries.Purge();

	return true;
}

const DynLibUtils::CModule *Sample::Provider::FindLibrary(const char *pszName) const
{
	auto iFound = m_mapLibraries.Find(FindSymbol(pszName));

	Assert(m_mapLibraries.IsValidIndex(iFound));

	return m_mapLibraries.Element(iFound);
}

CUtlSymbolLarge Sample::Provider::GetSymbol(const char *pszText)
{
	return m_aSymbolTable.AddString(pszText);
}

CUtlSymbolLarge Sample::Provider::FindSymbol(const char *pszText) const
{
	return m_aSymbolTable.Find(pszText);
}

bool Sample::Provider::LoadGameData(const char *pszBaseGameDir, const char *pszPathID, GameData::CBufferStringVector &vecMessages)
{
	return m_aStorage.Load(this, pszBaseGameDir, pszPathID, vecMessages);
}

bool Sample::Provider::GameDataStorage::Load(IGameData *pRoot, const char *pszBaseGameDir, const char *pszPathID, GameData::CBufferStringVector &vecMessages)
{
	const struct
	{
		const char *pszFilename;
		bool (Sample::Provider::GameDataStorage::*pfnLoad)(IGameData *, KeyValues3 *, GameData::CBufferStringVector &);
	} aConfigs[] =
	{
		{
			SAMPLE_PROVIDER_GAMERESOURCE_FILENAME,
			&GameDataStorage::LoadGameResource
		},
		{
			SAMPLE_PROVIDER_GAMESYSTEM_FILENAME,
			&GameDataStorage::LoadGameSystem
		},
		{
			SAMPLE_PROVIDER_SOURCE2SERVER_FILENAME,
			&GameDataStorage::LoadSource2Server
		}
	};

	CBufferStringN<MAX_PATH> sConfigFile;

	CUtlVector<CUtlString> vecConfigFiles;

	CUtlString sError;

	AnyConfig::CLoadFromFile_General aLoadPresets({{&sError, NULL, pszPathID}, g_KV3Format_Generic});

	for(const auto &aConfig : aConfigs)
	{
		AnyConfig::Anyone aGameConfig;

		sConfigFile.Clear();
		sConfigFile.Insert(0, pszBaseGameDir);
		sConfigFile.Insert(sConfigFile.Length(), CORRECT_PATH_SEPARATOR_S);
		sConfigFile.Insert(sConfigFile.Length(), aConfig.pszFilename);

		const char *pszConfigFile = sConfigFile.Get();

		g_pFullFileSystem->FindFileAbsoluteList(vecConfigFiles, pszConfigFile, pszPathID);

		if(vecConfigFiles.Count() < 1)
		{
			const char *pszMessageConcat[] = {"Failed to ", "find \"", pszConfigFile, "\" file"};

			vecMessages.AddToTail({pszMessageConcat});

			continue;
		}

		aLoadPresets.m_pszFilename = vecConfigFiles[0].Get();

		if(!aGameConfig.Load(aLoadPresets)) // Hot.
		{
			const char *pszMessageConcat[] = {"Failed to ", "load \"", pszConfigFile, "\" file", ": ", sError.Get()};

			vecMessages.AddToTail({pszMessageConcat});

			continue;
		}

		if(!(this->*(aConfig.pfnLoad))(pRoot, aGameConfig.Get(), vecMessages))
		{
			const char *pszMessageConcat[] = {"Failed to ", "parse \"", pszConfigFile, "\" file", ": ", sError.Get()};

			vecMessages.AddToTail({pszMessageConcat});

			continue;
		}

		// ...
	}

	return true;
}

bool Sample::Provider::GameDataStorage::LoadGameResource(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aGameResource.Load(pRoot, pGameConfig, vecMessages);
}

bool Sample::Provider::GameDataStorage::LoadGameSystem(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aGameSystem.Load(pRoot, pGameConfig, vecMessages);
}

bool Sample::Provider::GameDataStorage::LoadSource2Server(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aSource2Server.Load(pRoot, pGameConfig, vecMessages);
}

const Sample::Provider::GameDataStorage::CGameResource &Sample::Provider::GameDataStorage::GetGameResource() const
{
	return m_aGameResource;
}

const Sample::Provider::GameDataStorage::CGameSystem &Sample::Provider::GameDataStorage::GetGameSystem() const
{
	return m_aGameSystem;
}

const Sample::Provider::GameDataStorage::CSource2Server &Sample::Provider::GameDataStorage::GetSource2Server() const
{
	return m_aSource2Server;
}

const Sample::Provider::GameDataStorage &Sample::Provider::GetGameDataStorage() const
{
	return m_aStorage;
}
