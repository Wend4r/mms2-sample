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
