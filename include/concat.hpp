#ifndef _INCLUDE_METAMOD_SOURCE_CONCAT_HPP_
#	define _INCLUDE_METAMOD_SOURCE_CONCAT_HPP_

#	include <vector>

#	include <tier0/bufferstring.h>
#	include <tier1/utlvector.h>

template<class T>
struct ConcatLine
{
	T m_aStartWith;
	T m_aPadding;
	T m_aEnd;
	T m_aEndAndNextLine;

	std::vector<T> GetKeyValueConcat(const T &aKey, const T &aValue) const
	{
		return {m_aStartWith, aKey, m_aPadding, aValue, m_aEnd};
	}

	std::vector<T> GetKeyValueConcatString(const T &aKey, const T &aValue) const
	{
		return {m_aStartWith, aKey, m_aPadding, "\"", aValue, "\"", m_aEnd};
	}
}; // ConcatLine

using ConcatLineStringBase = ConcatLine<const char *>;

struct ConcatLineString : ConcatLineStringBase
{
	using Base = ConcatLineStringBase;

	const char *AppendToBuffer(CBufferString &sMessage, const char *aKey, const char *aValue) const
	{
		auto vecConcat = Base::GetKeyValueConcat(aKey, aValue);

		return sMessage.AppendConcat(vecConcat.size(), vecConcat.data(), NULL);
	}

	const char *AppendStringToBuffer(CBufferString &sMessage, const char *aKey, const char *aValue) const
	{
		auto vecConcat = Base::GetKeyValueConcatString(aKey, aValue);

		return sMessage.AppendConcat(vecConcat.size(), vecConcat.data(), NULL);
	}

	int AppendToVector(CUtlVector<const char *> vecMessage, const char *aKey, const char *aValue) const
	{
		auto vecConcat = Base::GetKeyValueConcat(aKey, aValue);

		return vecMessage.AddMultipleToTail(vecConcat.size(), vecConcat.data());
	}

	int AppendStringToVector(CUtlVector<const char *> vecMessage, const char *aKey, const char *aValue) const
	{
		auto vecConcat = Base::GetKeyValueConcatString(aKey, aValue);

		return vecMessage.AddMultipleToTail(vecConcat.size(), vecConcat.data());
	}
}; // ConcatLineString

#endif // _INCLUDE_METAMOD_SOURCE_CONCAT_HPP_
