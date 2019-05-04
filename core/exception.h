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
	public:
		Exception(const char * message, ...);
		const char* what() const NOEXCEPT{ return message.c_str(); }
		static void check(bool condition, const char* message);
	};

	class NullTaskInput : public Exception {
	public:
		NullTaskInput(const char* which) : Exception("Task input is not set: %s", which) {}
		static void check(void* pointer, const char* which);
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

}

#define BEATMUP_ERROR(...) throw Beatmup::Exception(__VA_ARGS__)

#ifdef BEATMUP_DEBUG
#define BEATMUP_ASSERT_DEBUG(C) { if (!(C)) throw Beatmup::Exception("Debug assertion failed: " #C); }
#else
#define BEATMUP_ASSERT_DEBUG(C)
#endif