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

SamplePlugin::CPlayerData::CPlayerData()
 :  m_pLanguage(nullptr), 
    m_aYourArgumentPhrase({nullptr, nullptr})
{
}

void SamplePlugin::CPlayerData::Init()
{
	m_pLanguage = nullptr;
	m_aYourArgumentPhrase = {nullptr, nullptr};
}

void SamplePlugin::CPlayerData::Destroy()
{
	Init();
}

const ISample::ILanguage *SamplePlugin::CPlayerData::GetLanguage() const
{
	return m_pLanguage;
}

void SamplePlugin::CPlayerData::SetLanguage(const ILanguage *pData)
{
	m_pLanguage = pData;
}

bool SamplePlugin::CPlayerData::AddLanguageListener(const LanguageHandleCallback_t *pfnCallback)
{
	// Check on exists.
	{
		int iFound = m_vecLanguageCallbacks.Find(pfnCallback);

		Assert(iFound != m_vecLanguageCallbacks.InvalidIndex());
	}

	m_vecLanguageCallbacks.AddToTail(pfnCallback);

	return true;
}

bool SamplePlugin::CPlayerData::RemoveLanguageListener(const LanguageHandleCallback_t *pfnCallback)
{
	return m_vecLanguageCallbacks.FindAndRemove(pfnCallback);
}

void SamplePlugin::CPlayerData::OnLanguageReceived(CPlayerSlot aSlot, CLanguage *pData)
{
	SetLanguage(pData);

	for(const auto &it : m_vecLanguageCallbacks)
	{
		(*it)(aSlot, pData);
	}
}

void SamplePlugin::CPlayerData::TranslatePhrases(const Translations *pTranslations, const CLanguage &aServerLanguage, CUtlVector<CUtlString> &vecMessages)
{
	const struct
	{
		const char *pszName;
		TranslatedPhrase *pTranslated;
	} aPhrases[] =
	{
		{
			"Your argument",
			&m_aYourArgumentPhrase,
		}
	};

	const Translations::CPhrase::CContent *paContent;

	Translations::CPhrase::CFormat aFormat;

	int iFound {};

	const auto *pLanguage = GetLanguage();

	const char *pszServerContryCode = aServerLanguage.GetCountryCode(), 
	           *pszContryCode = pLanguage ? pLanguage->GetCountryCode() : pszServerContryCode;

	for(const auto &aPhrase : aPhrases)
	{
		const char *pszPhraseName = aPhrase.pszName;

		if(pTranslations->FindPhrase(pszPhraseName, iFound))
		{
			const auto &aTranslationsPhrase = pTranslations->GetPhrase(iFound);

			if(!aTranslationsPhrase.Find(pszContryCode, paContent) && !aTranslationsPhrase.Find(pszServerContryCode, paContent))
			{
				CUtlString sMessage;

				sMessage.Format("Not found \"%s\" country code for \"%s\" phrase\n", pszContryCode, pszPhraseName);
				vecMessages.AddToTail(sMessage);

				continue;
			}

			aPhrase.pTranslated->m_pFormat = &aTranslationsPhrase.GetFormat();
		}
		else
		{
			CUtlString sMessage;

			sMessage.Format("Not found \"%s\" phrase\n", pszPhraseName);
			vecMessages.AddToTail(sMessage);

			continue;
		}

		if(!paContent->IsEmpty())
		{
			aPhrase.pTranslated->m_pContent = paContent;
		}
	}
}

const SamplePlugin::CPlayerData::TranslatedPhrase &SamplePlugin::CPlayerData::GetYourArgumentPhrase() const
{
	return m_aYourArgumentPhrase;
}

const ISample::ILanguage *SamplePlugin::GetServerLanguage() const
{
	return &m_aServerLanguage;
}
