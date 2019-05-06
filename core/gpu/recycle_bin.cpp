#include "recycle_bin.h"
#include "../parallelism.h"
#include "../debug.h"

using namespace Beatmup;
using namespace GL;

/**
	A task to empty the recycle bin
*/
class Recycler : public AbstractTask {
private:
	std::vector<RecycleBin::Item*>& items;

public:
	Recycler(std::vector<RecycleBin::Item*>& items) : items(items) {}


	ExecutionTarget getExecutionTarget() const {
		return ExecutionTarget::useGPUIfAvailable;
	}


	bool process(TaskThread& thread) {
		return true;
	}


	bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
		for (auto& item : items) {
			RecycleBin::Item* deleting = item;
			item = NULL;
			if (deleting)
				delete deleting;
		}
		items.clear();
		return true;
	}
};


RecycleBin::RecycleBin(Environment& env) : env(env)
{
	recycler = new Recycler(items);
}


RecycleBin::~RecycleBin() {
	delete recycler;
}


void RecycleBin::put(RecycleBin::Item* item) {
	lock();
	items.push_back(item);
	unlock();
}


void RecycleBin::emptyBin() {
	lock();
	if (items.size() > 0)
		env.performTask(*recycler);
	unlock();
}