#pragma once

#include <ace/Thread_Mutex.h>
#include <vector>
#include <map>
#include <string>

namespace std {

typedef std::basic_string<ACE_TCHAR> tstring;

}

namespace platform
{

typedef std::vector<std::tstring> StringSeq;
typedef std::map<std::tstring,std::tstring> StringDict;

/// 模仿Java的Properties
class Properties
{
public:
	Properties();
	Properties(const StringDict&);
	Properties(const Properties&);
	Properties& operator = (const Properties& properties);

	///@(加载properties
	int loadText(const ACE_TCHAR* text);
	int loadDict(const StringDict&);
	int loadFile(const std::tstring& filename);
	int loadStream(std::basic_istream<ACE_TCHAR>& stream);
	///@}

	///@{ 取property
	std::tstring getProperty(const std::tstring&) const;
	std::tstring getPropertyWithDefault(const std::tstring&, const std::tstring&) const;
	int getPropertyAsInt(const std::tstring&) const;
	int getPropertyAsIntWithDefault(const std::tstring&, int) const;
	int getPropertyAsIntWithDefault_i(const std::tstring& key, int value) const;
	///@}

	///取以prefix开头的properties
	StringDict getPropertiesForPrefix(const std::tstring& prefix) const;

	///@{ 设置property
	void setProperty(const std::tstring&, const std::tstring&);
	/// case insensitive
	void setPropertyCase(const std::tstring&, const std::tstring&);
	///@}

	///将properties转换成命令行参数的形式
	StringSeq getCommandLineOptions() const;

	///@{将命令行参数的形式转换成properties
	StringSeq parseCommandLineOptions(const std::tstring& prefix, const StringSeq&);
	StringSeq parseCommandLineOptions(const StringSeq&);
	///@}

	//Properties* clone() const;

	///取properties的个数
	size_t size() const;
	///清空
	void clear();

	//int save(std::tstring prefix);

private:

	void parsePropLine(const std::tstring&);
	bool match(const std::tstring& s, const std::tstring& pat, bool matchPeriod = false);

	StringDict _properties;

	std::tstring config_filename_;

	mutable ACE_Thread_Mutex _mutex;
};


}//end namespace util

