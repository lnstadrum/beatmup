/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#include "listing.h"
#include "input_stream.h"
#include "../exception.h"
#include <cstring>


using namespace Beatmup;


class Listing::Parser {
public:
    typedef std::map<std::string, std::vector<Listing::Block>> Content;
private:
    Content& chapters;
    std::vector<Listing::Block>* currentChapter;
    std::string chapterIndent;
    int lineCounter;

    void extractKeyValuePair(std::string line) {
        const auto delim = line.find(":");
        if (delim != std::string::npos) {
            size_t i = delim + 1;
            while (i < line.size() && line[i] == ' ')
                ++i;
            currentChapter->back().mapping.emplace(line.substr(0, delim), line.substr(i));
            return;
        }
        throw RuntimeError("Line " + std::to_string(lineCounter) + ": cannot extract key-value pair from " + line);
    }

    void processLine(std::string line) {
        const std::string originalLine = line;
        lineCounter++;

        if (line.empty())
            return;

        // CR+LF
        if (line.back() == '\r')
            line.pop_back();

        // process comments
        auto numberSign = line.find('#');
        if (numberSign != std::string::npos) {
            bool quoted = false;
            for (size_t i = 0; i < numberSign - 1; ++i)
                if (line[i] == '"')
                    quoted = !quoted;
            // the number sign '#' is not quoted
            if (!quoted)
                line = line.substr(0, numberSign);
        }

        // right trim
        auto l = line.find_last_not_of(' ');
        if (l == std::string::npos)
            return;
        if (l != line.length() - 1)
            line = line.substr(0, l + 1);

        // empty
        if (line.empty())
            return;

        // new chapter
        if (line.front() != ' ' && line.front() != '-' && line.back() == ':') {
            line.pop_back();
            auto it = chapters.emplace(std::piecewise_construct, std::forward_as_tuple(line), std::forward_as_tuple());
            currentChapter = &it.first->second;
            return;
        }

        // new entry
        if (currentChapter) {
            // no chapter indent yet defined, expecting the one
            if (chapterIndent.empty()) {
                auto i = line.find_first_not_of(' ');
                if (i != std::string::npos && i + 2 < line.length() && line[i] == '-' && line[i + 1] == ' ') {
                    chapterIndent = line.substr(0, i + 2);
                    currentChapter->push_back(lineCounter);
                    extractKeyValuePair(line.substr(i + 2));
                    return;
                }
            }

            // if the line starts with the chapter indent, new entry
            else if (line.substr(0, chapterIndent.length()) == chapterIndent) {
                currentChapter->push_back(lineCounter);
                extractKeyValuePair(line.substr(chapterIndent.length()));
                return;
            }

            // if the line starts with a whitespace ident as long as the chapter indent, new key-value pair
            else if (line.find_first_not_of(' ') == chapterIndent.length()) {
                extractKeyValuePair(line.substr(chapterIndent.length()));
                return;
            }
        }

        throw RuntimeError("Line " + std::to_string(lineCounter) + ": unexpected indent\n" + originalLine);
    }


public:
    Parser(InputStream& stream, Content& chapters) : chapters(chapters), currentChapter(nullptr), lineCounter(0) {
        static const int SIZE = 1024;
        char buffer[SIZE];
        std::string line;
        while (!stream.eof()) {
            std::memset(buffer, 0, SIZE);
            stream(buffer, SIZE);
            int i = 0;
            while (i < SIZE && buffer[i] != 0) {
                if (buffer[i] == '\n') {
                    processLine(line);
                    line = "";
                }
                else
                    line += buffer[i];
                ++i;
            }
        }
        processLine(line);
    }


    Parser(std::istream& stream, Content& chapters) : chapters(chapters), currentChapter(nullptr), lineCounter(0) {
        for (std::string line; std::getline(stream, line); )
            processLine(line);
    }
};


void Listing::Block::printOut(std::ostream& str) {
    bool first = true;
    for (auto& it : mapping) {
        str << (first ? " - " : "   ") << it.first << ": " << it.second << std::endl;
        first = false;
    }
}


std::string Listing::Block::operator[](const std::string& key) const {
    auto it = mapping.find(key);
    if (it == mapping.cend())
        if (lineNumber > 0) {
            throw InvalidArgument("Key not found in a block at line " + std::to_string(lineNumber) + ": " + key);
        }
        else {
            throw InvalidArgument("Key not found: " + key);
        }
    return it->second;
}


Listing::Listing(InputStream& stream) {
    Parser parser(stream, chapters);
}


Listing::Listing(std::istream& stream) {
    Parser parser(stream, chapters);
}


void Listing::printOut(std::ostream& str) {
    for (auto& it : chapters) {
        str << it.first << ":" << std::endl;
        for (auto& i : it.second)
            i.printOut(str);
    }
}

const std::vector<Listing::Block>& Listing::operator[](const std::string& key) const {
    auto it = chapters.find(key);
    if (it == chapters.cend())
        throw new InvalidArgument("Key not found: " + key);
    return it->second;
}


void Listing::emplace(const std::string& key, Block&& block) {
    if (!has(key))
        chapters.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple());
    chapters[key].emplace_back(block);
}