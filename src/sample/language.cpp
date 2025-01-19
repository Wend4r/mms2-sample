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

#include <sample_plugin.hpp>

Sample_Plugin::CLanguage::CLanguage(const CUtlSymbolLarge &sInitName, const char *pszInitCountryCode)
 :  m_sName(sInitName), 
    m_sCountryCode(pszInitCountryCode)
{
}

const char *Sample_Plugin::CLanguage::GetName() const
{
	return m_sName.String();
}

void Sample_Plugin::CLanguage::SetName(const CUtlSymbolLarge &s)
{
	m_sName = s;
}

const char *Sample_Plugin::CLanguage::GetCountryCode() const
{
	return m_sCountryCode;
}

void Sample_Plugin::CLanguage::SetCountryCode(const char *psz)
{
	m_sCountryCode = psz;
}
