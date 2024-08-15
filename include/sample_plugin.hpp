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

#ifndef _INCLUDE_METAMOD_SOURCE_SAMPLE_PLUGIN_HPP_
#define _INCLUDE_METAMOD_SOURCE_SAMPLE_PLUGIN_HPP_

#include <ISmmPlugin.h>

class SamplePlugin final : public ISmmPlugin, public IMetamodListener
{
public: // ISmmPlugin
	const char *GetAuthor() override;
	const char *GetName() override;
	const char *GetDescription() override;
	const char *GetURL() override;
	const char *GetLicense() override;
	const char *GetVersion() override;
	const char *GetDate() override;
	const char *GetLogTag() override;

public: // IMetamodListener
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late) override;
	bool Unload(char *error, size_t maxlen) override;
	bool Pause(char *error, size_t maxlen) override;
	bool Unpause(char *error, size_t maxlen) override;
	void AllPluginsLoaded() override;
}; // SamplePlugin

extern SamplePlugin *g_pSamplePlugin;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_SAMPLE_PLUGIN_HPP_
