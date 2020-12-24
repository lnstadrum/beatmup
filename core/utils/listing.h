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

#pragma once
#include <string>
#include <map>
#include <vector>
#include <ostream>

#include "input_stream.h"

namespace Beatmup {

    /**
        Parser of simple YAML-like listings.
        Listing consists of chapters. Chapters are ordered sequences of blocks of key-value pairs:
            chapter1:
             - key: value
               arg: another value
             - key: stuff  # comments are like this
               param: number
            chapter2:
             - stuff: stuff
    */
    class Listing {
        class Parser;
    public:
        /**
            Set of key-value pairs
        */
        class Block {
            friend class Parser;

        private:
            std::map<std::string, std::string> mapping;
            int lineNumber;

        protected:
            inline Block(int lineNumber): lineNumber(lineNumber) {}
        public:
            inline Block(): lineNumber(-1) {}
            inline Block(const std::map<std::string, std::string>& mapping): mapping(mapping), lineNumber(-1) {}

            /**
                Prints out the block to an output stream.
                \param[in,out] stream       The output stream
            */
            void printOut(std::ostream& stream);

            /**
                Retrieves a value by key.
                If not found, an exception is thrown.
                \param[in] key      The key
            */
            std::string operator[](const std::string& key) const;

            /**
                Returns `true` if a value is defined for a specific key in the block.
                \param[in] key      The key
            */
            inline bool has(const std::string& key) const { return mapping.find(key) != mapping.cend(); }

            /**
                Returns a value by key casted to a given type.
                An exception is thrown if no value defined for the given key.
                \tparam T           The value type
                \param[in] key      The key
            */
            template<typename T>
            inline T get(const std::string& key) const;

            /**
                Returns a value by key casted to a given type.
                \tparam T                   The value type
                \param[in] key              The key
                \param[in] defaultValue     Default value returned if no value is defined for the key.
            */
            template<typename T>
            inline T get(const std::string& key, const T defaultValue) const {
                return has(key) ? get<T>(key) : defaultValue;
            }

            /**
                Sets a value for a specific key.
                \tparam T           The value type
                \param[in] key      The key
                \param[in] value    The new value
            */
            template<typename T>
            void set(const std::string& key, T value);

            /**
                Returns line number the block starts at.
            */
            inline int getLineNumber() const { return lineNumber; }
        };

    private:
        std::map<std::string, std::vector<Block>> chapters;

    public:
        Listing() {}
        Listing(InputStream& stream);
        Listing(std::istream& stream);

        /**
            Prints out the listing to an output stream.
            Respects its own format when printing, i.e., the printed output may be reparsed into a listing.
            \param[in,out] stream       The output stream
        */
        void printOut(std::ostream& stream);

        /**
            Returns `true` if a specific chapter is present in the listing.
            \param[in] key      The chapter name
        */
        inline bool has(const std::string& key) const { return chapters.find(key) != chapters.cend(); }

        /**
            Retrieves a chapter in the listing by its name.
            If not found, an exception is thrown.
            \param[in] key      The chapter name
        */
        const std::vector<Block>& operator[](const std::string& key) const;

        /**
            Adds a block to a chapter.
            \param[in] key      The chapter name
            \param[in] block    The block
        */
        void emplace(const std::string& key, Block&& block);
    };

    template<>
    inline std::string Listing::Block::get(const std::string& key) const {
        return (*this)[key];
    }

    template<>
    inline int Listing::Block::get(const std::string& key) const {
        return std::atoi((*this)[key].c_str());
    }

    template<>
    inline float Listing::Block::get(const std::string& key) const {
        return std::atof((*this)[key].c_str());
    }

    template<>
    inline bool Listing::Block::get(const std::string& key) const {
        return (*this)[key] == "true";
    }

    template<>
    inline void Listing::Block::set(const std::string& key, std::string value) {
        mapping[key] = value;
    }

    template<>
    inline void Listing::Block::set(const std::string& key, float value) {
        mapping[key] = std::to_string(value);
    }

    template<>
    inline void Listing::Block::set(const std::string& key, int value) {
        mapping[key] = std::to_string(value);
    }
}