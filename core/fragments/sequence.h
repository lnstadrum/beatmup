/*
	Sequence: a set of fragments allowing for basic editing in a transparent way
*/

#pragma once
#include "../basic_types.h"
#include "fragment.h"
#include "../exception.h"
#include <vector>

namespace Beatmup {
	namespace Fragments {

		/**
			A set of signal fragments allowing for basic editing operations (copy, paste, cut in two, shrink, etc.).
			Operations are performed on fragment set, not on the data directly.
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
				void moveTo(dtime time);

				/**
					Moves the pointer forward an arbitrary number of samples
				*/
				void step();

				/**
					Enables "watching mode", i.e., if the sequence is modified, the pointer will follow the modifications and remain valid
				*/
				void watch();

				inline void jump(dtime time)				{ moveTo(currentTime + time); }
				inline dtime getTime() const				{ return currentTime; }
				inline bool hasData() const				{ return !pointer.isNull(); }
				inline int samplesAvailable() const		{ return pointer.length; }
			};

		private:
			std::vector<FragmentPtr> fragments;		//!< the content
			std::vector<dtime> cumTimes;				//!< cumulative sum of fragment lengths, starts from 0, of N+1 entries (N = num. of fragments)
			std::vector<Pointer*> pointers;			//!< pointers currently accessing the sequence

			/**
				Log search for a fragment containing given time moment
				\return a valid fragment index if available, VOID_LEFT or VOID_RIGHT otherwise
			*/
			int findFragment(dtime time) const;

			/**
				\internal
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
				\internal
				Initializes an empty sequence, used to bootstrap copying operations.
			*/
			virtual Sequence* createEmpty() const = 0;

			/**
				\internal
				Adds a new fragment at the end of the sequence
			*/
			void concatenate(Fragment& fragment, dtime offset, dtime length);

			/**
				\internal
				Resets pointers once the sequence changes to keep them consistent
			*/
			void syncPointers();

		public:
			inline const dtime getLength() const { return cumTimes.back(); }

			inline bool isEmpty() const { return getLength() == 0; }

			void clear();

			/**
				Shrinks the sequence to specified time borders.
				\param timeLeft		left limit to shrink to (included), i.e., the earliest time moment from which the new (shrinked) sequence will begin
				\param timeRight	right limit to shrink to (excluded)
			*/
			void shrink(dtime timeLeft, dtime timeRight);

			/**
				Copies a specified piece of the sequence to another sequence
				\param fromTime		the beginning of the piece to copy out (inclued in the resulting sequence)
				\param toTime		the ending of the piece (excluded of the resulting sequence)
			*/
			Sequence* copy(dtime fromTime, dtime toTime) const;

			/**
				Inserts a sequence in a specified position in time
				\param sequence		the thing to insert
				\param time			the position
			*/
			void insert(const Sequence& sequence, dtime time);

			/**
				Erases a part of a sequence between two given time moments
			*/
			void remove(dtime fromTime, dtime toTime);

			void split(dtime time, Sequence* left, Sequence* right);

			class AccessException : public Exception {
			public:
				AccessException(const char* message, const Sequence& sequence) : Exception(message) {}
			};
		};


		template<class T> class SequenceToolkit : public Sequence {
		public:
			T* copy(dtime fromTime, dtime toTime) const { return (T*)Sequence::copy(fromTime, toTime); }
		};
	}
}
