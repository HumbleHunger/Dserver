#include "Exception.h"

#include <execinfo.h>

namespace DJX
{

Exception::Exception(std::string what)
: message_(std::move(what))
{
	fillStackTrace();
}


void Exception::fillStackTrace()
{
	enum
	{
		max_size = 200
	};
	void *buffer[max_size];
	int nptrs = ::backtrace(buffer, max_size);
	char **strings = ::backtrace_symbols(buffer, nptrs);
	if (strings)
	{
		for (int i = 0; i < nptrs; ++i)
		{
			stack_.append(strings[i]);
			stack_.push_back('\n');
		}
	}
	free(strings);
}

} // namespace DJX