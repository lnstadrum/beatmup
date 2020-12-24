/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <exception>
#include <string>
#include <cstdio>


#if defined(_MSC_VER)
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

namespace Beatmup {

    /**
        Base class for all exceptions
    */
    class Exception : public std::exception {
    private:
        std::string message;
    protected:
        inline Exception(const char* message) : message(message) {}

        template<typename ...Args>
        inline Exception(const char* message, const Args&... args) {
            static const int MAX_LENGTH = 4*1024;
            char out[MAX_LENGTH];
#if _MSC_VER
            sprintf_s(out, MAX_LENGTH, message, args...);
#else
            snprintf(out, MAX_LENGTH, message, args...);
#endif
            this->message.assign(out);
        }

    public:
        virtual inline const char* what() const NOEXCEPT override {
            return message.c_str();
        }
    };

    class RuntimeError : public Exception {
    public:
        inline RuntimeError(const std::string& message): Exception(message.c_str()) {}
        inline static void check(const bool condition, const std::string& message) {
            if (!condition)
                throw RuntimeError(message);
        }
    };

    class InvalidArgument : public Exception {
    protected:
        template<typename datatype> InvalidArgument(const char* message, const datatype value): Exception(message, value) {}
    public:
        inline InvalidArgument(const std::string& message): Exception(message.c_str()) {}
        inline static void check(const bool condition, const std::string& message) {
            if (!condition)
                throw InvalidArgument(message);
        }
    };

    class OutOfRange : public InvalidArgument {
    private:
        template<typename datatype> OutOfRange(const char* message, const datatype value): InvalidArgument(message, value) {}
    public:
        template<typename datatype>
        inline static void check(const datatype value, const datatype min, const datatype max, const char* message) {
            if (value < min || max < value)
                throw OutOfRange(message, value);
        }

        template<typename datatype>
        inline static void checkMin(const datatype value, const datatype min, const char* message) {
            if (value < min)
                throw OutOfRange(message, value);
        }
    };

    class IOError : public Exception {
    private:
        std::string filename;
    public:
        inline IOError(const std::string& filename, const char * message):
            Exception("Cannot access %s:\n%s", filename.c_str(), message),
            filename(filename) { }
        const std::string& getFilename() const { return filename; }
    };

    class NullTaskInput : public Exception {
    public:
        inline NullTaskInput(const char* which) : Exception("Task input is not set: %s", which) {}

        inline static void check(const void* pointer, const char* which) {
            if (!pointer)
                throw NullTaskInput(which);
        }
    };

    /**
        Thrown when an implementation restriction is encountered
    */
    class ImplementationUnsupported : public Exception {
    public:
        inline ImplementationUnsupported(const char* description) : Exception(description) {}
    };

    /**
        Thrown when something happens that should never do
    */
    class Insanity : public Exception {
    private:
        inline Insanity(const char* message) : Exception(message) {}
    public:
        static inline void insanity(const char* message)  {
            throw Beatmup::Insanity(message);
        }
    };

#ifdef BEATMUP_DEBUG
    /**
        Thrown when a debugging check does not pass
    */
    class DebugAssertion : public Exception {
    private:
        template<typename ...Args>
        inline DebugAssertion(const std::string& message, const Args&... args) : Exception(message.c_str(), args...) {}
    public:
        template<typename ...Args>
        static inline void check(const bool condition, const std::string& message, const Args&... args) {
            if (!condition)
                throw DebugAssertion(message, args...);
        }
    };
#endif
}


#ifdef BEATMUP_DEBUG
#define BEATMUP_ASSERT_DEBUG(C) Beatmup::DebugAssertion::check((C), "Debug assertion failed.\n" #C);
#else
#define BEATMUP_ASSERT_DEBUG(C)
#endif
