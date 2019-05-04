#pragma once
#include "../basic_types.h"
#include <mutex>

namespace Beatmup {
	class LockableObject : public Object {
	private:
		std::mutex access;
	public:
		inline void lock() { access.lock(); }

		inline void unlock() { access.unlock(); }

		class LockGuard {
		private:
			LockableObject& obj;
		public:
			LockGuard(LockableObject& obj) : obj(obj)
			{ obj.lock(); }

			LockGuard(LockableObject* obj) : LockGuard(*obj)
			{}

			~LockGuard()
			{ obj.unlock(); }
		};
	};
}