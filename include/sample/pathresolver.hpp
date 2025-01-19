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

#ifndef _INCLUDE_METAMOD_SOURCE_SAMPLE_PATH_RESOLVER_HPP_
#	define _INCLUDE_METAMOD_SOURCE_SAMPLE_PATH_RESOLVER_HPP_

#	define SAMPLE_PATH_RESOLVER_ADDONS_DIR "addons"
#	define SAMPLE_PATH_RESOLVER_BINARY_DIR "bin"

#	include <stddef.h>

#	include <string_view>

namespace Sample
{
	class PathResolver
	{
	public:
		PathResolver(const void *pInitModule);

	public:
		bool Init();
		void Clear();

	public:
		std::string_view GetAbsoluteModuleFilename();
		std::string_view ExtractSubpath(std::string_view sStartMarker = SAMPLE_PATH_RESOLVER_ADDONS_DIR, std::string_view sEndMarker = SAMPLE_PATH_RESOLVER_BINARY_DIR);

	private:
		const void *m_pModule;
		std::string_view m_sModuleFilename;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_SAMPLE_PATH_RESOLVER_HPP_
