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

#include "memory.h"
#include "exception.h"
#include "debug.h"
#include <cstdlib>
#include <cstdint>
#include <cstring>


#if BEATMUP_PLATFORM_WINDOWS
    #include <windows.h>
    #undef min
    #undef max
#else
    #include <unistd.h>
    #include <sys/statfs.h>
    #include <sys/sysinfo.h>
#endif


using namespace Beatmup;

const size_t AlignedMemory::DEFAULT_ALIGNMENT = 8;


uint64_t AlignedMemory::available() {
#if BEATMUP_PLATFORM_WINDOWS
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullAvailPhys;
#else
    struct sysinfo info;
    sysinfo(&info);
    return info.freeram * info.mem_unit;
#endif
}

uint64_t AlignedMemory::total() {
#if BEATMUP_PLATFORM_WINDOWS
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#else
    struct sysinfo info;
    sysinfo(&info);
    return info.totalram * info.mem_unit;
#endif
}


AlignedMemory::AlignedMemory(size_t size, size_t align) {
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(size > 0, "Trying to allocate zero memory");
    BEATMUP_DEBUG_I("Allocating %lu Kbytes (%lu MB free)...", (unsigned long)(size / 1024), (unsigned long)(available() / 1048576));
#endif
    rawAddr = malloc(size + align);
    alignAddr = reinterpret_cast<void*>(
            reinterpret_cast<std::uintptr_t>(rawAddr) + (align - reinterpret_cast<std::uintptr_t>(rawAddr) % align)
    );
}


AlignedMemory::AlignedMemory(size_t size, int value, size_t align): AlignedMemory(size, align) {
    memset(alignAddr, value, size);
}


AlignedMemory::~AlignedMemory() {
    if (rawAddr)
        std::free(rawAddr);
}


AlignedMemory& AlignedMemory::operator=(AlignedMemory&& other) {
    if (rawAddr)
        std::free(rawAddr);
    rawAddr = other.rawAddr;
    alignAddr = other.alignAddr;
    other.rawAddr = nullptr;
    other.alignAddr = nullptr;
    return *this;
}


void AlignedMemory::free() {
    if (rawAddr) {
        std::free(rawAddr);
        rawAddr = nullptr;
        alignAddr = nullptr;
    }
}
