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

#include <sample/pathresolver.hpp>

#include <stddef.h>
#include <string.h>

#include <cstddef>

#include <dynlibutils/module.hpp>

#include <tier0/basetypes.h>
#include <tier0/dbg.h>

Sample::PathResolver::PathResolver(const void *pInitModule)
 :  m_pModule(pInitModule)
{
}

bool Sample::PathResolver::Init()
{
	m_sModuleFilename = DynLibUtils::CModule(m_pModule).GetPath();

	return true;
}

void Sample::PathResolver::Clear()
{
	m_sModuleFilename = {};
}

std::string_view Sample::PathResolver::GetAbsoluteModuleFilename()
{
	return m_sModuleFilename;
}

std::string_view Sample::PathResolver::ExtractSubpath(std::string_view sStartMarker, std::string_view sEndMarker)
{
	auto &sFullPath = m_sModuleFilename;

	std::size_t nStartPosition = sFullPath.find(sStartMarker);

	if(nStartPosition != std::string_view::npos)
	{
		std::size_t nEndPosition = sFullPath.find(sEndMarker, nStartPosition);

		if(nEndPosition != std::string_view::npos)
		{
			return sFullPath.substr(nStartPosition, nEndPosition - (nStartPosition + 1));
		}
	}

	return "";
}
