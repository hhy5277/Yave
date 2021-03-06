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

#include "DeviceMemoryHeap.h"
#include "alloc.h"

#include <numeric>
#include <mutex>

namespace yave {

static usize align_size(usize total_byte_size, usize alignent) {
	return (total_byte_size + alignent - 1) & ~(alignent - 1);
}

usize DeviceMemoryHeap::FreeBlock::end_offset() const {
	return offset + size;
}

bool DeviceMemoryHeap::FreeBlock::contiguous(const FreeBlock& blck) const {
	return end_offset() == blck.offset || blck.end_offset() == offset;
}

void DeviceMemoryHeap::FreeBlock::merge(const FreeBlock& block) {
	y_debug_assert(contiguous(block));
	std::tie(offset, size) = std::tuple{std::min(offset, block.offset), size + block.size};
}


DeviceMemoryHeap::DeviceMemoryHeap(DevicePtr dptr, u32 type_bits, MemoryType type) :
		DeviceMemoryHeapBase(dptr),
		_memory(alloc_memory(dptr, heap_size, type_bits, type)),
		_blocks({FreeBlock{0, heap_size}}),
		_mapping(is_cpu_visible(type)
				? static_cast<u8*>(device()->vk_device().mapMemory(_memory, 0, heap_size))
				: nullptr
			) {
}

DeviceMemoryHeap::~DeviceMemoryHeap() {
	if(_blocks.size() != 1) {
		y_fatal("Not all memory has been free: heap fragmented.");
	}
	if(_blocks[0].offset || _blocks[0].size != heap_size) {
		y_fatal("Not all memory has been freed.");
	}
	if(_mapping) {
		device()->vk_device().unmapMemory(_memory);
	}
	device()->vk_device().freeMemory(_memory);
}

DeviceMemory DeviceMemoryHeap::create(usize offset, usize size) {
	return DeviceMemory(this, _memory, offset, size);
}

core::Result<DeviceMemory> DeviceMemoryHeap::alloc(vk::MemoryRequirements reqs) {
	usize size = align_size(reqs.size, alignment);

	for(auto it = _blocks.begin(); it != _blocks.end(); ++it) {
		usize full_size = it->size;

		if(full_size < size) {
			continue;
		}

		usize offset = it->offset;

		usize aligned_offset = align_size(offset, reqs.alignment);
		usize align_correction = aligned_offset - offset;
		usize aligned_size = full_size - align_correction;

		y_debug_assert(aligned_size % alignment == 0);
		y_debug_assert(aligned_offset % alignment == 0);
		y_debug_assert(aligned_offset % reqs.alignment == 0);

		if(aligned_size == size) {
			_blocks.erase_unordered(it);
			if(align_correction) {
				_blocks << FreeBlock{offset, align_correction};
			}
			return core::Ok(create(aligned_offset, size));
		} else if(aligned_size > size) {
			usize end = offset + full_size;
			it->size = end - (it->offset = aligned_offset + size);
			if(align_correction) {
				_blocks << FreeBlock{offset, align_correction};
			}
			return core::Ok(create(aligned_offset, size));
		}
	}

	return core::Err();
}

void DeviceMemoryHeap::free(const DeviceMemory& memory) {
	y_debug_assert(memory.vk_memory() == _memory);
	free(FreeBlock {memory.vk_offset(), memory.vk_size()});
}

void DeviceMemoryHeap::free(const FreeBlock& block) {
	y_debug_assert(block.end_offset() <= heap_size);
	compact_block(block);
}

void DeviceMemoryHeap::compact_block(FreeBlock block) {
	y_profile();
	// sort ?
	bool compacted = false;
	do {
		compacted = false;
		for(auto it = _blocks.begin(); it != _blocks.end(); ++it) {
			FreeBlock b = *it;
			if(b.contiguous(block)) {
				_blocks.erase_unordered(it);
				b.merge(block);
				block = b;
				compacted = true;
				break;
			}
		}
	} while(compacted);
	_blocks << block;
}

void* DeviceMemoryHeap::map(const DeviceMemoryView& view) {
	return static_cast<u8*>(_mapping + view.vk_offset());
}

void DeviceMemoryHeap::unmap(const DeviceMemoryView&) {
}

usize DeviceMemoryHeap::size() const {
	return heap_size;
}

usize DeviceMemoryHeap::available() const {
	usize tot = 0;
	for(const auto& b : _blocks) {
		tot += b.size;
	}
	return tot;
}

usize DeviceMemoryHeap::free_blocks() const {
	return _blocks.size();
}

}
