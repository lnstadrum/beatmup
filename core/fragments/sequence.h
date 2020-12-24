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
#include "../basic_types.h"
#include "fragment.h"
#include "../exception.h"
#include <vector>

namespace Beatmup {
    namespace Fragments {

        /**
            Fragmented signal in time domain.
            Implements basic editing operations (copy, paste, cut in two, shrink, etc.).
            Operations are performed on the set of fragments, but not directly to the signal content in memory.
        */
        class Sequence : public Object {
        protected:
            /**
                Points to a position within a sequence
            */
            class Pointer {
            private:
                bool writing;				//!< if `true`, the pointer is used to modify the data
                bool watching;				//!< if `true`, the pointer will remain valid even if the sequence is being modified
                dtime currentTime;
                int fragmentIdx;

            protected:
                Sequence& sequence;
                FragmentPtr pointer;		//!< pointed fragment
                Pointer(Sequence& sequence, dtime time, bool writing);
                ~Pointer();

            public:
                /**
                    Sets pointer to a specific time
                    \param time     The time (in samples) to set the pointer to
                */
                void moveTo(dtime time);

                /**
                    Moves the pointer forward an arbitrary number of samples
                */
                void step();

                /**
                    Enables "watching mode", i.e., if the sequence is modified, the pointer will follow the modifications and remain valid
                */
                void watch();

                /**
                    Moves the pointer by a given number of samples relatively to the current position
                */
                inline void jump(dtime by)				{ moveTo(currentTime + by); }

                inline dtime getTime() const				{ return currentTime; }
                inline bool hasData() const				{ return !pointer.isNull(); }
                inline int samplesAvailable() const		{ return pointer.length; }
            };

        private:
            std::vector<FragmentPtr> fragments;     //!< the content
            std::vector<dtime> cumTimes;            //!< cumulative sum of fragment lengths, starts from 0, of N+1 entries (N = num. of fragments)
            std::vector<Pointer*> pointers;         //!< pointers currently accessing the sequence

            /**
                Log search for a fragment containing given time moment
                \return a valid fragment index if available, VOID_LEFT or VOID_RIGHT otherwise
            */
            int findFragment(dtime time) const;

            /**
                Splits a given fragment in two. Does not recompute the cumulative time index.
                \param index	the fragment index
                \param delta	additional offset w.r.t. fragment internal offset specifying where to split
            */
            void splitFragment(int index, dtime delta);

            static const int
                VOID_LEFT = -123,		//!< returned by getFragment() when the entire sequence is on the right of the given time moment
                VOID_RIGHT = -456;		//!< returned by getFragment() when the entire sequence is on the left of the given time moment

        protected:
            Sequence();
            virtual ~Sequence();

            /**
                Initializes an empty sequence, used to bootstrap copying operations.
            */
            virtual Sequence* createEmpty() const = 0;

            /**
                Adds a new fragment at the end of the sequence
            */
            void concatenate(Fragment& fragment, dtime offset, dtime length);

            /**
                Resets pointers once the sequence changes to keep them consistent
            */
            void syncPointers();

        public:
            /**
                Returns sequence duration in number of samples
             */
            inline const dtime getDuration() const { return cumTimes.back(); }

            /**
                Returns `true` if sequence contains no samples
             */
            inline bool isEmpty() const { return getDuration() == 0; }

            /**
                Removes the content of the sequence making it empty (of zero duration).
            */
            void clear();

            /**
                Shrinks the sequence to given time bounds
                \param timeLeft     Left limit to shrink to (included), i.e., the earliest time moment from which the new (shrinked) sequence will begin
                \param timeRight    Right limit to shrink to (excluded)
            */
            void shrink(dtime timeLeft, dtime timeRight);

            /**
                Copies a given piece of the current sequence into new Sequence
                \param fromTime     The beginning of the piece to copy (included in the resulting sequence)
                \param toTime       The end of the piece (excluded from the resulting sequence)
            */
            Sequence* copy(dtime fromTime, dtime toTime) const;

            /**
                Inserts a Sequence at a given position in time
                \param sequence     The sequence to insert
                \param time         The time to insert from
            */
            void insert(const Sequence& sequence, dtime time);

            /**
                Erases a part of the sequence between two given time moments
                \param fromTime     The beginning of the part to remove
                \param toTime       The end of the part to remove
            */
            void remove(dtime fromTime, dtime toTime);

            /**
                Splits the sequence in two at a specific time.
                Creates two new Sequence instances. The content of the current sequence remains unchanged.
                \param time         The time moment to cut at
                \param left         The left part of the split
                \param right        The right part of the split
            */
            void split(dtime time, Sequence* left, Sequence* right) const;

            class AccessException : public Exception {
            public:
                AccessException(const char* message, const Sequence& sequence) : Exception(message) {}
            };
        };


        template<class T> class SequenceToolkit : public Sequence {
        public:
            T* copy(dtime fromTime, dtime toTime) const { return static_cast<T*>(Sequence::copy(fromTime, toTime)); }
        };
    }
}
