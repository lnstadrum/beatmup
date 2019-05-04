#include "fragment.h"

using namespace Beatmup::Fragments;


Fragment::Fragment() : referenceCount(0), sampleCount(0) {};


Fragment* Fragment::edit() {
	std::lock_guard<std::mutex> lock(access);
	if (referenceCount > 1) {
		--referenceCount;
		Fragment* copy = clone();
		copy->referenceCount = 1;
		return copy;
	}
	return this;
}


Fragment* Fragment::use() {
	std::lock_guard<std::mutex> lock(access);
	++referenceCount;
	return this;
}


void Fragment::drop() {
	std::unique_lock<std::mutex> lock(access);
	if (referenceCount <= 1) {
		lock.unlock();
		delete this;
		return;
	}
	--referenceCount;
	lock.unlock();
}


void FragmentPtr::operator =(const FragmentPtr& src) {
	if (!isNull())
		fragment->drop();
	if (src.isNull())
		nullify();
	else {
		fragment = src.fragment->use();
		offset = src.offset;
		length = src.length;
	}
}


FragmentPtr::FragmentPtr() {
	fragment = NULL;
	offset = length = 0;
}



FragmentPtr::FragmentPtr(Fragment& fragment, int offset, int length):
	fragment(&fragment), offset(offset), length(length)
{
	fragment.use();
}


FragmentPtr::FragmentPtr(const FragmentPtr& src) {
	if (src.isNull()) {
		fragment = NULL;
		offset = length = 0;
	}
	else {
		fragment = src.fragment->use();
		offset = src.offset;
		length = src.length;
	}
}


FragmentPtr::FragmentPtr(FragmentPtr&& src) {
	fragment = src.fragment;
	offset = src.offset;
	length = src.length;
	// nullifying source
	src.fragment = NULL;
	src.offset = src.length = 0;
}


FragmentPtr::~FragmentPtr() {
	if (!isNull())
		fragment->drop();
}


void FragmentPtr::editData() {
	fragment = fragment->edit();
		// No need to check if the fragment is cloned or not and drop the original if it is.
		// If it is cloned, there is at least one reference to the original fragment.
		// So no dropping, just decrease ref counter in fragment->edit().
}


void FragmentPtr::nullify() {
	if (!isNull())
		fragment->drop();
	fragment = NULL;
	offset = length = 0;
}