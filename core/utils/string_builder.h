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
#include <cstdarg>
#include <string>

namespace Beatmup {
    /**
        Toolset to build a string content.
    */
    class StringBuilder {
        std::string& str;
    public:
        StringBuilder(std::string& workspace) :
            str(workspace)
        {}

        StringBuilder& replace(const std::string& search, const std::string& replacement) {
            size_t pos = 0;
            while ((pos = str.find(search, pos)) != std::string::npos) {
                str.replace(pos, search.length(), replacement);
                pos += replacement.length();
            }
            return *this;
        }

        StringBuilder& operator()(const std::string& append) {
            str.append(append);
            return *this;
        }

        StringBuilder& line(const std::string& append) {
            str.append(append + "\n");
            return *this;
        }

        template<const int BUF_SIZE = 256> StringBuilder& printf(const char* format, ...) {
            va_list args;
            va_start(args, format);
            char buffer[BUF_SIZE];
#ifdef _MSC_VER
            vsnprintf_s
#else
            vsnprintf
#endif
            (buffer, BUF_SIZE, format, args);
            str.append(buffer);
            va_end(args);
            return *this;
        }

        StringBuilder& nl() {
            str.append("\n");
            return *this;
        }

        operator std::string& () {
            return str;
        }

        void dump(std::string filename);
    };

    /**
        StringBuilder including a string container.
    */
    class String : public StringBuilder {
    private:
        std::string str;
    public:
        String(): StringBuilder(str) {}

        String(const String& content): StringBuilder(str) {
            str = content.str;
        }

        String(const std::string& content): StringBuilder(str) {
            str = content;
        }

        String(std::initializer_list<const char*> lines) : StringBuilder(str) {
            static const std::string NL("\n");
            for (auto& line : lines)
                str += line + NL;
        }

        StringBuilder& operator =(const std::string& content) {
            str = content;
            return *this;
        }
    };
}
