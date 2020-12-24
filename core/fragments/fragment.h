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
#include <mutex>

namespace Beatmup {
    /**
        Abstract fragmented signals representation.
        Used internally for audio signals.
    */
    namespace Fragments {
        /**
            Represents a continuous set of data samples
        */
        class Fragment {
            Fragment(const Fragment&) = delete; //!< disabling copying constructor

        private:
            std::mutex access;                  //!< exclusive access
            unsigned int referenceCount;        //!< number of occurrences of this frame in sequences

        protected:
            int sampleCount;                    //!< number of samples within this frame
            Fragment();
            virtual ~Fragment() {};

        public:
            inline const int getSampleCount() const { return sampleCount; }
            virtual Fragment* clone() const = 0;

            /**
                Enables editing of the current frame
            */
            Fragment* edit();

            /**
                References the frame when it is used one more time
            */
            Fragment* use();

            /**
                Dereferences the frame when it is not used any more
            */
            void drop();
        };

        /**
            Pointer to a Fragment.
            Handles reference counting of the pointer Fragment.
        */
        struct FragmentPtr {
        public:
            Fragment* fragment;
            int offset;			//!< offset in samples inside the fragment since its beginning
            int length;			//!< number of samples to use from the fragment
            void operator =(const FragmentPtr&);

            /**
                Initialize to null
            */
            FragmentPtr();
            FragmentPtr(Fragment& fragment, int offset, int length);
            FragmentPtr(const FragmentPtr&);
            FragmentPtr(FragmentPtr&&);

            ~FragmentPtr();

            void editData();
            inline bool isNull() const { return fragment == NULL; }
            void nullify();
        };
    }}