/*
	Differed destruction of GPU resources
*/
#pragma once
#include <mutex>
#include <vector>
#include "..\environment.h"
#include "..\utils\lockable_object.h"

namespace Beatmup {
	namespace GL {

		class RecycleBin : public LockableObject {
		public:
			/**
				A wrapper for a GPU resource. Destroyed in a GPU-aware thread when emptying the recycle bin.
			*/
			class Item {
			private:
				Item(const Item&) = delete;		//!< disabling copying constructor
			public:
				Item() {}
				virtual ~Item() {}
			};

		private:
			std::vector<Item*> items;
			Environment& env;
			AbstractTask* recycler;

		public:
			RecycleBin(Environment& env);
			~RecycleBin();
		
			/**
				Puts an item into the recycle bin
			*/
			void put(Item* item);

			/**
				Empty the bin destroying all the items in a GPU-aware thread
			*/
			void emptyBin();
		};


		template<class Base> class Object : public Base, public RecycleBin::Item {
		private:
			virtual ~Object() {};
		public:
			Object(GraphicPipeline& gpu) : Base(gpu) {}

			void destroy(RecycleBin& bin) {
				bin.put(this);
			}
		};
	}
}