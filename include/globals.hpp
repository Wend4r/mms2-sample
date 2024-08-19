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

#ifndef _INCLUDE_METAMOD_SOURCE_GLOBALS_HPP_
#	define _INCLUDE_METAMOD_SOURCE_GLOBALS_HPP_

#	include <stddef.h>

#	define GLOBALS_NAMEOF_ARGUMENTS(var) #var, var

class IVEngineServer2;
class ICvar;
class IFileSystem;
class ISource2Server;

namespace SourceMM
{
	class ISmmAPI;
	class ISmmPlugin;
}; // SourceMM

class CGlobalVars;
class CGameEntitySystem;

#	include <interfaces/interfaces.h>

extern bool InitGlobals(SourceMM::ISmmAPI *ismm, char *error, size_t maxlen);
extern void DebugGlobals(SourceMM::ISmmAPI *ismm, SourceMM::ISmmPlugin *pl);
extern bool DestoryGlobals(char *error, size_t maxlen);

extern CGlobalVars *GetGameGlobals();
// CGameEntitySystem *GameEntitySystem(); // Declared in <entity2/entitysystem.h>

#endif //_INCLUDE_METAMOD_SOURCE_GLOBALS_HPP_
