/*******************************
Copyright (c) 2016-2019 Gr�goire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/

#include "DedicatedDeviceMemoryAllocator.h"
#include "alloc.h"

namespace yave {

DedicatedDeviceMemoryAllocator::DedicatedDeviceMemoryAllocator(DevicePtr dptr, MemoryType type) : DeviceMemoryHeapBase(dptr), _type(type) {
}

DedicatedDeviceMemoryAllocator::~DedicatedDeviceMemoryAllocator() {
}

core::Result<DeviceMemory> DedicatedDeviceMemoryAllocator::alloc(vk::MemoryRequirements reqs) {
	_size += reqs.size;
	return core::Ok(DeviceMemory(this, alloc_memory(device(), reqs, _type), 0, reqs.size));
}

void DedicatedDeviceMemoryAllocator::free(const DeviceMemory& memory) {
	if(memory.vk_offset()) {
		y_fatal("Tried to free memory using non zero offset.");
	}
	_size -= memory.vk_size();
	device()->vk_device().freeMemory(memory.vk_memory());
}

void* DedicatedDeviceMemoryAllocator::map(const DeviceMemoryView& view) {
	return device()->vk_device().mapMemory(view.vk_memory(), view.vk_offset(), VK_WHOLE_SIZE);
}

void DedicatedDeviceMemoryAllocator::unmap(const DeviceMemoryView& view) {
	device()->vk_device().unmapMemory(view.vk_memory());
}

usize DedicatedDeviceMemoryAllocator::allocated_size() const {
	return _size;
}

}
