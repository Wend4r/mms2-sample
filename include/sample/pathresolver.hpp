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

#ifndef _INCLUDE_METAMOD_SOURCE_SAMPLE_PATHRESOLVER_HPP_
#	define _INCLUDE_METAMOD_SOURCE_SAMPLE_PATHRESOLVER_HPP_

#	define SAMPLE_PATH_RESOLVER_ADDONS_DIR "addons"
#	define SAMPLE_PATH_RESOLVER_BINARY_DIR "bin"

#	include <dynlibutils/module.hpp>

#	include <stddef.h>

#	include <string_view>

namespace Sample
{
	class CPathResolver
	{
	public:
		bool Init(void *pModuleHandle) { m_aModule.InitFromMemory(pModuleHandle); return true; }

	public:
		std::string_view GetAbsoluteModuleFilename() { return m_aModule.GetPath(); }
		std::string_view Extract(std::string_view svStartMarker = SAMPLE_PATH_RESOLVER_ADDONS_DIR, std::string_view svEndMarker = SAMPLE_PATH_RESOLVER_BINARY_DIR)
		{
			auto svFullPath = GetAbsoluteModuleFilename();

			std::size_t nStartPosition = svFullPath.find(svStartMarker);

			if(nStartPosition == std::string_view::npos)
			{
				return "";
			}

			std::size_t nEndPosition = svFullPath.find(svEndMarker, nStartPosition);

			if(nEndPosition == std::string_view::npos)
			{
				return "";
			}

			return svFullPath.substr(nStartPosition, nEndPosition - (nStartPosition + 1));;
		}

	private:
		DynLibUtils::CModule m_aModule;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_SAMPLE_PATHRESOLVER_HPP_
