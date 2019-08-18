/*
	Error control
*/
#pragma once

#include <exception>
#include <string>


#if defined(_MSC_VER)
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

namespace Beatmup {

	class Exception : public std::exception {
	private:
		std::string message;
	protected:
		Exception(const char* message, ...);
	public:
		const char* what() const NOEXCEPT{ return message.c_str(); }
	};
  
	class RuntimeError : public Exception {
	public:
		RuntimeError(const char* message): Exception(message) {}
		RuntimeError(const std::string& message): Exception(message.c_str()) {}
		inline static void check(const bool condition, const char* message) {
			if (!condition)
				throw RuntimeError(message);
        }
	};

	class IOError : public Exception {
	private:
		std::string filename;
	public:
		IOError(const std::string& filename, const char * message):
			Exception("Cannot access %s:\n%s", filename.c_str(), message),
			filename(filename) { }
		const std::string& getFilename() const { return filename; }
	};

	class NullTaskInput : public Exception {
	public:
		NullTaskInput(const char* which) :
			Exception("Task input is not set: %s", which) {}

		inline static void check(void* pointer, const char* which) {
			if (!pointer)
				throw NullTaskInput(which);
		}
	};

	/**
		Thrown when an implementation restriction is encountered
	*/
	class ImplementationUnsupported : public Exception {
	public:
		ImplementationUnsupported(const char* description) : Exception(description) {}
	};

	/**
		Thrown when something happens that should never do
	*/
	class Insanity : public Exception {
	private:
		Insanity(const char* message) : Exception(message) {}
	public:
		static void insanity(const char* message);
	};

	class DebugAssertion : public Exception {
	private:
		DebugAssertion(const char* message) : Exception(message) {}
	public:
		static void check(bool condition, const char* message, ...);
	};
}


#ifdef BEATMUP_DEBUG
#define BEATMUP_ASSERT_DEBUG(C) Beatmup::DebugAssertion::check((C), "Debug assertion failed.\n" #C);
#else
#define BEATMUP_ASSERT_DEBUG(C)
#endif
