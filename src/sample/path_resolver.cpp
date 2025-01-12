#include <sample/path_resolver.hpp>

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
	m_sModuleFilename = DynLibUtils::CModule(m_pModule).GetModulePath();

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
