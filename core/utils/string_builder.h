/**
    String builder
*/

#pragma once
#include <stdarg.h>
#include <string>

namespace Beatmup {
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
      
		StringBuilder& operator()(const char* append) {
			str.append(append);
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

		void dump(std::string filename);
	};

    /**
        StringBuilder with an encorporated container
    */
    class String : public StringBuilder {
    private:
        std::string str;
    public:
        String(): StringBuilder(str) {}
        String(const std::string& content): StringBuilder(str) {
            str = content;
        }
        
        StringBuilder& operator =(const std::string& content) {
            str = content;
			return *this;
        }
      
        operator std::string&() {
            return str;
        }
    };
}