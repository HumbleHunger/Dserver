#ifndef DJX_EXCEPTION_H
#define DJX_EXCEPTION_H

#include <exception>
#include <string>

namespace DJX
{

class Exception : public std::exception
{
public:
	Exception(std::string what);
	~Exception() = default;

	const char* what() const noexcept override
	{
		return message_.c_str();
	}

	const char* stackTrace() const noexcept
	{
		return stack_.c_str();
	}

private:
	void fillStackTrace();
	std::string message_;
	std::string stack_;
};

} // namespace DJX

#endif