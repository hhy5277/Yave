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

#include "EntityWorld.h"

namespace yave {
namespace ecs {

EntityWorld::EntityWorld() {
}


EntityId EntityWorld::create_entity() {
	return _entities.create();
}

void EntityWorld::remove_entity(EntityId id) {
	if(id.is_valid()) {
		_deletions << id;
	}
}

bool EntityWorld::exists(EntityId id) const {
	return _entities.contains(id);
}

const EntityIdPool& EntityWorld::entities() const {
	return _entities;
}

void EntityWorld::flush() {
	y_profile();
	for(const auto& c : _component_containers) {
		c.second->remove(_deletions);
	}
	for(EntityId id : _deletions) {
		_entities.recycle(id);
	}
	_deletions.clear();
}

core::String EntityWorld::type_name(ComponentTypeIndex index) const {
	return y::detail::demangle_type_name(index.name());
}


const ComponentContainerBase* EntityWorld::container(ComponentTypeIndex type) const {
	if(auto it = _component_containers.find(type); it != _component_containers.end()) {
		return it->second.get();
	}
	return nullptr;
}

ComponentContainerBase* EntityWorld::container(ComponentTypeIndex type) {
	auto& container = _component_containers[type];
	if(!container) {
		container = detail::create_container(type);
	}
	return container.get();
}

serde2::Result EntityWorld::serialize(WritableAssetArchive& writer) const {
	if(!writer(u64(_entities.size()))) {
		return core::Err();
	}

	for(EntityId id : _entities) {
		if(!writer(u64(id.index()))) {
			return core::Err();
		}
	}

	if(!writer(u32(_component_containers.size()))) {
		return core::Err();
	}
	for(const auto& container : _component_containers) {
		if(!detail::serialize_container(writer, container.second.get())) {
			return core::Err();
		}
	}
	return core::Ok();
}

serde2::Result EntityWorld::deserialize(ReadableAssetArchive& reader) {
	*this = EntityWorld();

	u64 entity_count = 0;
	if(!reader(entity_count)) {
		return core::Err();
	}

	for(u64 i = 0; i != entity_count; ++i) {
		u64 index = 0;
		if(!reader(index)) {
			return core::Err();
		}
		_entities.create_with_index(EntityIndex(index)).unwrap();
	}
	y_debug_assert(_entities.size() == entity_count);

	u32 container_count = 0;
	if(!reader(container_count)) {
		return core::Err();
	}
	for(u32 i = 0; i != container_count; ++i) {
		if(auto container = detail::deserialize_container(reader)) {
			_component_containers[container->type()] = std::move(container);
		} else {
			log_msg("Component type can not be deserialized.", Log::Warning);
		}
	}

	for(const auto& p : _component_containers) {
		log_msg(type_name(p.first));
		for(EntityIndex i : p.second->indexes()) {
			if(!_entities.contains(EntityId::from_unversioned_index(i))) {
				return core::Err();
			}
		}
	}

	return core::Ok();
}

}
}
