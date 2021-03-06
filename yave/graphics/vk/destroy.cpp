/*******************************
Copyright (C) 2013-2016 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it widptr be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#include "destroy.h"
#include <yave/device/Device.h>

namespace yave {
namespace detail {

void destroy(DevicePtr dptr, vk::Buffer buffer) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyBuffer(buffer);
	}
}

void destroy(DevicePtr dptr, vk::Image image) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyImage(image);
	}
}

void destroy(DevicePtr dptr, vk::ImageView image_view) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyImageView(image_view);
	}
}

void destroy(DevicePtr dptr, vk::RenderPass render_pass) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyRenderPass(render_pass);
	}
}

void destroy(DevicePtr dptr, vk::Framebuffer framebuffer) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyFramebuffer(framebuffer);
	}
}

void destroy(DevicePtr dptr, vk::Pipeline pipeline) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyPipeline(pipeline);
	}
}

void destroy(DevicePtr dptr, vk::PipelineLayout pipeline_layout) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyPipelineLayout(pipeline_layout);
	}
}

void destroy(DevicePtr dptr, vk::ShaderModule module) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyShaderModule(module);
	}
}


void destroy(DevicePtr dptr, vk::Sampler sampler) {
	/*if(dptr)*/ {
		dptr->vk_device().destroySampler(sampler);
	}
}

void destroy(DevicePtr dptr, vk::SwapchainKHR swapchain) {
	/*if(dptr)*/ {
		dptr->vk_device().destroySwapchainKHR(swapchain);
	}
}

void destroy(DevicePtr dptr, vk::CommandPool pool) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyCommandPool(pool);
	}
}

void destroy(DevicePtr dptr, vk::Fence fence) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyFence(fence);
	}
}

void destroy(DevicePtr dptr, vk::DescriptorPool pool) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyDescriptorPool(pool);
	}
}

void destroy(DevicePtr dptr, vk::DescriptorSetLayout layout) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyDescriptorSetLayout(layout);
	}
}

void destroy(DevicePtr dptr, vk::Semaphore semaphore) {
	/*if(dptr)*/ {
		dptr->vk_device().destroySemaphore(semaphore);
	}
}

void destroy(DevicePtr dptr, vk::QueryPool pool) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyQueryPool(pool);
	}
}

void destroy(DevicePtr dptr, vk::Event event) {
	/*if(dptr)*/ {
		dptr->vk_device().destroyEvent(event);
	}
}




void destroy(DevicePtr dptr, vk::SurfaceKHR surface) {
	/*if(dptr)*/ {
		dptr->instance().vk_instance().destroySurfaceKHR(surface);
	}
}

}
}
