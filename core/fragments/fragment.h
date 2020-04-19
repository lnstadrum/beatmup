/*
    Frame: a piece of data samples (video frames or audio samples)
*/
#pragma once
#include <mutex>
namespace Beatmup {
    namespace Fragments {
        /**
            Represents a piece of data samples
        */
        class Fragment {
            Fragment(const Fragment&) = delete;		//!< disabling copying constructor
        private:
            std::mutex access;				//!< exclusive access
            unsigned int referenceCount;	//!< number of occurrences of this frame in sequences
        protected:
            int sampleCount;		//!< number of samples within this frame
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
            Wraps fragment pointer
        */
        struct FragmentPtr {
        public:
            Fragment* fragment;
            int offset;			//!< offset in samples inside the framgent since its beginning
            int length;			//!< number of samples to use from the fragment
            void operator =(const FragmentPtr&);
            /**
                Initialize to NULL
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