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

Sample::Provider::GameDataStorage::CSource2Server::CSource2Server()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("&s_GameEventManager"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &aAddress)
		{
			this->m_ppGameEventManager = aAddress.RCast<decltype(this->m_ppGameEventManager)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
}

bool Sample::Provider::GameDataStorage::CSource2Server::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void Sample::Provider::GameDataStorage::CSource2Server::Reset()
{
	this->m_ppGameEventManager = nullptr;
}

CGameEventManager **Sample::Provider::GameDataStorage::CSource2Server::GetGameEventManagerPtr() const
{
	return this->m_ppGameEventManager;
}
