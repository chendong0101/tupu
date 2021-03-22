#include "properties.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>
#include <ace/OS_NS_string.h>
#include <ace/OS_NS_strings.h>

using namespace std;

namespace platform {

tstring Properties::getProperty(const tstring& key) const
{
	map<tstring, tstring>::const_iterator p = _properties.find(key);
	if(p != _properties.end())
	{
		return p->second;
	}
	else
	{
		return tstring();
	}
}

tstring Properties::getPropertyWithDefault(const tstring& key, const tstring& value) const
{
	//IceUtil::Mutex::Lock sync(*this);
	_mutex.acquire_read();

	map<tstring, tstring>::const_iterator p = _properties.find(key);
	if(p != _properties.end())
	{
		_mutex.release();
		return p->second;
	}
	else
	{
		_mutex.release();
		return value;
	}
}

int Properties::getPropertyAsInt(const tstring& key) const
{
	return getPropertyAsIntWithDefault(key, 0);
}

int Properties::getPropertyAsIntWithDefault(const tstring& key, int value) const
{
	//IceUtil::Mutex::Lock sync(*this);
	_mutex.acquire_read();
	value = getPropertyAsIntWithDefault_i(key, value);
	_mutex.release();

	return value;
}

int Properties::getPropertyAsIntWithDefault_i(const tstring& key, int value) const
{
	map<tstring, tstring>::const_iterator p = _properties.find(key);
	if(p != _properties.end())
	{
		return ACE_OS::atoi(p->second.c_str());
	}
	return value;
}

StringDict Properties::getPropertiesForPrefix(const tstring& prefix) const
{
	//IceUtil::Mutex::Lock sync(*this);

	_mutex.acquire_read();

	StringDict result;
	map<tstring, tstring>::const_iterator p;
	for(p = _properties.begin(); p != _properties.end(); ++p)
	{
		if(prefix.empty() || p->first.compare(0, prefix.size(), prefix) == 0)
		{
			result.insert(*p);
		}
	}

	_mutex.release();
	return result;
}

void Properties::setProperty(const tstring& key, const tstring& value)
{
	if(key.empty())
	{
		return; //不允许空的key
	}

	//
	// Check if the property is legal. (We write to cerr instead of
	// using a logger because no logger may be established at the time
	// the property is parsed.)
	//

	//IceUtil::Mutex::Lock sync(*this);
	_mutex.acquire_write();

	//与ice不一样，当没有定义ice时，无需检查key是否满足ice的标准，直接set到_properties中即可
	_properties[key] = value;

	_mutex.release();
}

void Properties::setPropertyCase(const tstring& key, const tstring& value)
{
    if(key.empty())
    {
        return; //不允许空的key
    }

    //
    // Check if the property is legal. (We write to cerr instead of
    // using a logger because no logger may be established at the time
    // the property is parsed.)
    //

    //IceUtil::Mutex::Lock sync(*this);
    _mutex.acquire_write();

    std::map<std::tstring, std::tstring>::iterator it = _properties.begin();
    while (it != _properties.end()){
        if (ACE_OS::strcasecmp(it->first.c_str(), key.c_str()) == 0){
            it->second = value;
            break;
        }
        ++it;
    }

    if (it == _properties.end()){
        _properties[key] = value;
    }

    _mutex.release();
}

StringSeq Properties::getCommandLineOptions() const
{
	//IceUtil::Mutex::Lock sync(*this);
	_mutex.acquire_read();

	StringSeq result;
	result.reserve(_properties.size());
	map<tstring, tstring>::const_iterator p;
	for(p = _properties.begin(); p != _properties.end(); ++p)
	{
		result.push_back(ACE_TEXT("--") + p->first + ACE_TEXT("=") + p->second);
	}

	_mutex.release();
	return result;
}

StringSeq Properties::parseCommandLineOptions(const tstring& prefix, const StringSeq& options)
{
	tstring pfx = prefix;
	if(!pfx.empty() && pfx[pfx.size() - 1] != '.')
	{
		pfx += '.';
	}
	pfx = ACE_TEXT("--") + pfx;

	StringSeq result;
	StringSeq::size_type i;
	for(i = 0; i < options.size(); i++)
	{
		tstring opt = options[i];
		if(opt.find(pfx) == 0)
		{
			if(opt.find('=') == tstring::npos)
			{
				opt += ACE_TEXT("=1");
			}

			parsePropLine(opt.substr(2));
		}
		else
		{
			result.push_back(opt);
		}
	}
	return result;
}

StringSeq Properties::parseCommandLineOptions(const StringSeq& options)
{
	return parseCommandLineOptions(ACE_TEXT(""), options);
}

int Properties::loadText(const ACE_TCHAR* text)
{
	std::basic_istringstream<ACE_TCHAR> s(text);
	return this->loadStream(s);
}

int Properties::loadDict(const StringDict& dict)
{
	for(StringDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
	{
		this->setProperty(p->first, p->second);
	}
	return 0;
}

int Properties::loadFile(const std::tstring& file)
{
	basic_ifstream<ACE_TCHAR> in(ACE_TEXT_ALWAYS_CHAR(file.c_str()));

	int r = this->loadStream(in);

	if (r != 0)
		return -1;

	config_filename_ = file;
	return 0;
}

int Properties::loadStream(std::basic_istream<ACE_TCHAR>& in)
{
	if(!in)
		return -1;

	std::tstring line;
	while(getline(in, line))
	{
		parsePropLine(line);
	}

	return 0;
}

Properties::Properties()
{
}

Properties::Properties(const Properties& rhs) 
	: _properties(rhs._properties)
	, config_filename_(rhs.config_filename_)
{
}

Properties::Properties(const StringDict& dict)
	: _properties(dict)
{
}

void Properties::parsePropLine(const tstring& line)
{
	const ACE_TCHAR delim[] = ACE_TEXT(" \t\r\n");
	const ACE_TCHAR delim2[] = ACE_TEXT(" \t\r\n=");
	tstring s = line;

	tstring::size_type idx = s.find('#');
	if(idx != tstring::npos)
	{
		s.erase(idx);
	}

	idx = s.find_last_not_of(delim);
	if(idx != tstring::npos && idx + 1 < s.length())
	{
		s.erase(idx + 1);
	}

	tstring::size_type beg = s.find_first_not_of(delim);
	if(beg == tstring::npos)
	{
		return;
	}

	tstring::size_type end = s.find_first_of(delim2, beg);
	if(end == tstring::npos)
	{
		return;
	}

	tstring key = s.substr(beg, end - beg);

	end = s.find('=', end);
	if(end == tstring::npos)
	{
		return;
	}
	++end;

	tstring value;
	beg = s.find_first_not_of(delim, end);
	if(beg != tstring::npos)
	{
		end = s.length();
		value = s.substr(beg, end - beg);
	}

	setProperty(key, value);
}

bool Properties::match(const tstring& s, const tstring& pat, bool matchPeriod)
{
	assert(!s.empty());
	assert(!pat.empty());

	if(pat.find('*') == tstring::npos)
	{
		return s == pat;
	}

	tstring::size_type sIndex = 0;
	tstring::size_type patIndex = 0;
	do
	{
		if(pat[patIndex] == '*')
		{
			//
			// Don't allow matching x..y against x.*.y if requested -- star matches non-empty sequence only.
			//
			if(!matchPeriod && s[sIndex] == '.')
			{
				return false;
			}
			while(sIndex < s.size() && (matchPeriod || s[sIndex] != '.'))
			{
				++sIndex;
			}
			patIndex++;
		}
		else
		{
			if(pat[patIndex] != s[sIndex])
			{
				return false;
			}
			++sIndex;
			++patIndex;
		}
	}
	while(sIndex < s.size() && patIndex < pat.size());

	return sIndex == s.size() && patIndex == pat.size();
}

Properties& Properties::operator =(const Properties& aceproperty)
{
	this->_properties = aceproperty._properties;
	this->config_filename_ = aceproperty.config_filename_;
	return *this;
}

void Properties::clear()
{
    _properties.clear();
}

size_t Properties::size() const
{
    return _properties.size();
}

}//namespace util

