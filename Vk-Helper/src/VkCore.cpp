#include <VkCore.hpp>
#include <VkInitializers.hpp>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory>

// A basic debug callback. A more advanced one could be created, but this will do for basic debugging
VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
	VkDebugReportFlagsEXT       flags,
	VkDebugReportObjectTypeEXT  objectType,
	uint64_t                    object,
	size_t                      location,
	int32_t                     messageCode,
	const char*                 pLayerPrefix,
	const char*                 pMessage,
	void*                       pUserData)
{
	printf(pMessage);
	return VK_FALSE;
}


// Compare the required layers to the avaliable layers on the system
bool VkHelper::CheckLayersSupport(const char** layers, int count)
{
	// Find out how many layers are avaliable on the system
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Using the count, tell the system how many layer definitions we want to read
	// These layer properties are the layers that are avaliable on the system
	std::unique_ptr<VkLayerProperties[]> layerProperties(new VkLayerProperties[layerCount]());
	vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.get());

	// Loop through for each layer we want to check
	for (int i = 0; i < count; ++i)
	{
		bool layerFound = false;
		// Loop through for each avaliable system layer and atempt to find our required layer
		for (int j = 0; j < layerCount; ++j)
		{
			// Check to see if the layer matches
			if (strcmp(layers[i], layerProperties[j].layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		// If we are missing the required layer, report back
		if (!layerFound)
		{
			return false;
		}
	}
	// Found all the layers
	return true;
}

VkInstance VkHelper::CreateInstance(const char ** extensions, unsigned int extensions_count, const char ** layers, unsigned int layerCount, const char * app_name, uint32_t app_ver, 
	const char * engine_name, uint32_t engine_ver, uint32_t api_version)
{
	VkInstance instance;

	VkApplicationInfo app_info = VkHelper::ApplicationInfo(
		app_name,                                                // Application name
		app_ver,                                                 // Application version
		engine_name,                                             // Engine name
		engine_ver,                                              // Engine version
		api_version                                              // Required API version
	);

	VkInstanceCreateInfo create_info = VkHelper::InstanceCreateInfo(
		app_info,                                                // Pointer to the application information created
		extensions,                                              // The raw data of the extensions to enable
		extensions_count,                                        // The amount of extensions we wish to enable
		layers,                                                  // The raw data of the layers to enable
		layerCount                                               // The amount of layers we wish to enable
	);

	VkResult result = vkCreateInstance(
		&create_info,                                            // Information to pass to the function
		NULL,                                                    // Memory allocation callback
		&instance                                                // The Vulkan instance to be initialized
	);

	// Was the vulkan instance created sucsessfully
	assert(result == VK_SUCCESS);

	return instance;
}

// Attach a debugger to the application to give us validation feedback.
// This is usefull as it tells us about any issues without application
VkDebugReportCallbackEXT VkHelper::CreateDebugger(const VkInstance& instance)
{
	// Get the function pointer for the debug callback function within VK
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));


	/*PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT =
		reinterpret_cast<PFN_vkDebugReportMessageEXT>
		(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));*/


	// Define a CreateInfo for our new callback
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;            // Callback Type
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |                            // What we wukk be notified about
		VK_DEBUG_REPORT_WARNING_BIT_EXT |                                                 //...
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;                                      //...
	callbackCreateInfo.pfnCallback = &MyDebugReportCallback;                              // Our function that will be called on a callback
	callbackCreateInfo.pUserData = nullptr;                                               // A custom data pointer that the user can define. Since we are calling a non member function
	                                                                                      // it may be usefull to pass a pointer instance of the engine or rendering libary, in this case
	                                                                                      // we dont need anything

	VkDebugReportCallbackEXT callback;

	// Create the new callback
	VkResult result = vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);

	// Was the vulkan callback created sucsessfully
	assert(result == VK_SUCCESS);

	return callback;
}

void VkHelper::DestroyDebugger(const VkInstance & instance, const VkDebugReportCallbackEXT & debugger)
{
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

	// Destroy the debug callback
	vkDestroyDebugReportCallbackEXT(
		instance,
		debugger,
		nullptr
	);
}

bool VkHelper::HasRequiredExtentions(const VkPhysicalDevice & physical_device, const char ** required_extentions, const uint32_t & required_extention_count)
{
	// Get all the extentions on device
	// First we need to get the extention count
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(
		physical_device,                                       // Physical Device we are checking
		nullptr,                                               // 
		&extension_count,                                      // A return parameter to allow us to get the amount of extentions on the gpu
		nullptr                                                // A return pointer that sets a pointer array full of the extention names
															   // When set to nullptr, we are telling the system we want to get the device count
	);

	// Now get all the avaliable extentions on the device
	std::unique_ptr<VkExtensionProperties[]> available_extensions(new VkExtensionProperties[extension_count]());
	vkEnumerateDeviceExtensionProperties(
		physical_device,                                       // Physical Device we are checking
		nullptr,                                               // 
		&extension_count,                                      // A return parameter to allow us to get the amount of extentions on the gpu
		available_extensions.get()                             // A return pointer that sets a pointer array full of the extention names
	);

	// Loop through all wanted extentions to make sure they all exist
	for (int i = 0; i < required_extention_count; ++i)
	{
		bool extention_found = false;
		// Loop through for each avaliable device extention and atempt to find our required extention
		for (int j = 0; j < extension_count; ++j)
		{
			// Check to see if the layer matches
			if (strcmp(required_extentions[i], available_extensions[j].extensionName) == 0)
			{
				extention_found = true;
				break;
			}
		}
		// If we are missing the extention layer, report back
		if (!extention_found)
		{
			return false;
		}
	}
	return true;
}

bool VkHelper::GetQueueFamily(const VkPhysicalDevice & physical_device, VkQueueFlags required_queue_flags, uint32_t & queue_family_index)
{
	// Fist we need to get the amount of queue families on the system
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		physical_device,                                       // Physical Device we are checking
		&queue_family_count,                                   // A return parameter to allow us to get the amount of queue families on the gpu
		nullptr                                                // A return pointer that sets a pointer array full of the extention names
	);

	// Next get the queue families
	std::unique_ptr<VkQueueFamilyProperties[]> queue_families(new VkQueueFamilyProperties[queue_family_count]());
	vkGetPhysicalDeviceQueueFamilyProperties(
		physical_device,                                       // Physical Device we are checking
		&queue_family_count,                                   // A return parameter to allow us to get the amount of queue families on the gpu
		queue_families.get()                                   // A return pointer that sets a pointer array full of the extention names
															   // When set to nullptr, we are telling the system we want to get the queue family count
	);

	// Loop through all the queue families to find one that matches our criteria
	for (int i = 0; i < queue_family_count; ++i)
	{
		// Make sure there are queues in the queue family
		if (queue_families[i].queueCount > 0)
		{
			// Check if the queue family has the correct queues
			// Using bitwise AND check that the queue flags are set
			if ((queue_families[i].queueFlags & required_queue_flags) == required_queue_flags)
			{
				queue_family_index = i;
				return true;
			}
		}
	}
	// Return a invalid state
	return false;
}

bool VkHelper::GetPhysicalDevice(const VkInstance& instance, VkPhysicalDevice & physical_device, VkPhysicalDeviceProperties & device_properties, uint32_t & queue_family_index,
	VkPhysicalDeviceFeatures& device_features, VkPhysicalDeviceMemoryProperties& device_mem_properties, const char ** physical_device_extentions, const unsigned int extention_count,
	VkQueueFlags required_queue_flags)
{
	// Find all avaliable devices within the system
	// Get the device count
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(
		instance,                                           // Refrence to our Vulkan Instance
		&device_count,                                      // A return parameter for the amount of system devices
		nullptr                                             // A pointer return parameter for all devices, if left nullptr, then we are saying, we just want the device count
	);

	// Get the Device Instances
	std::unique_ptr<VkPhysicalDevice[]> devices(new VkPhysicalDevice[device_count]());
	// Get the physical devices
	vkEnumeratePhysicalDevices(
		instance,                                           // Refrence to our Vulkan Instance
		&device_count,                                      // A return parameter for the amount of system devices
		devices.get()                                       // A pointer return parameter to get all devices
	);

	// Now we have all the physical devices that are inside the device, we need to find a suitable one for our needs
	for (int i = 0; i < device_count; i++)
	{
		// First we need to check that the device has all the required extentions
		if (HasRequiredExtentions(devices[i], physical_device_extentions, extention_count))
		{
			// Next each physical device has a list of queue families that controll the flow of commands to and from the CPU side of the system
			// to the GPU side. Each queue family contains a diffrent amount of Queues, some contain all of them, some may contain a mixed bag or
			// some may only contain 1.

			// The common queues we consern ourself with is the GraphicsQueue, but there is also compute (for general compute tasks), 
			// transfer (moving data between the devices) and sparse (for device memory managment).

			uint32_t queue_family = 0;
			if (GetQueueFamily(devices[i], required_queue_flags, queue_family))
			{
				// Now we know the device is valid, there may be several valid devices within the system and we need a way of choosing between them
				// So we can get the properties of the device and find a way of prioritising them
				VkPhysicalDeviceProperties physical_device_properties;
				vkGetPhysicalDeviceProperties(
					devices[i],
					&physical_device_properties
				);
				// Find the features that are avaliable on the device
				VkPhysicalDeviceFeatures physical_device_features;
				vkGetPhysicalDeviceFeatures(
					devices[i],
					&physical_device_features
				);

				// Get all information about the devices memory
				VkPhysicalDeviceMemoryProperties physical_device_mem_properties;
				vkGetPhysicalDeviceMemoryProperties(
					devices[i],
					&physical_device_mem_properties
				);

				// First if the chosen device is null and the only GPU that reports back is a intergrated gpu then use it untill we find a deticated one
				if (physical_device == VK_NULL_HANDLE || physical_device != VK_NULL_HANDLE && physical_device_properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					physical_device = devices[i];
					queue_family_index = queue_family;
					device_properties = physical_device_properties;
					device_features = physical_device_features;
					device_mem_properties = physical_device_mem_properties;
				}
			}
		}
	}

	// Check to see if the device is valid
	return physical_device != VK_NULL_HANDLE;
}

VkDevice VkHelper::CreateDevice(const VkPhysicalDevice & physical_device, VkDeviceQueueCreateInfo * queue_infos, uint32_t queue_info_count, VkPhysicalDeviceFeatures & physical_device_features,
	const char** extensions, unsigned int extensions_count)
{
	// Create a device create info for the device
	VkDeviceCreateInfo device_create_info = VkHelper::DeviceCreateInfo(
		queue_infos,                                            // Queues we want to enable
		queue_info_count,                                       // How many queue instances
		physical_device_features,                               // The physical devices instances
		extensions,                                             // What device extentions do we want
		extensions_count                                        // How many extentions
	);
	VkDevice device = VK_NULL_HANDLE;
	// Finish off by creating the device itself
	VkResult result = vkCreateDevice(
		physical_device,
		&device_create_info,
		nullptr,
		&device
	);
	// Was the vulkan device created sucsessfully
	assert(result == VK_SUCCESS);
	return device;
}

VkCommandPool VkHelper::CreateCommandPool(const VkDevice & device, const uint32_t & queue_family, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo pool_info = VkHelper::CommandPoolCreateInfo(
		queue_family,                                                    // What queue family we are wanting to use to send commands to the GPU
		flags
	);

	VkCommandPool command_pool = VK_NULL_HANDLE;
	// Create the command pool
	VkResult result = vkCreateCommandPool(
		device,
		&pool_info,
		nullptr,
		&command_pool
	);
	// Was the vulkan command pool created sucsessfully
	assert(result == VK_SUCCESS);
	return command_pool;
}
