#pragma once

#include "VulkanResourceBase.h"
#include "DeviceContext.h"
#include "Device.h"
#include "FrameBuffer.h"
#include "BaseObject.h"

#include <vulkan/vulkan.h>

namespace suz
{
	class BufferArea;
	class MainWindow;
	class CommandContext;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;				// 基本表面功能
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct SwapChainSpec
	{
		SwapChainSpec(const DeviceContext& Context, const VkSurfaceKHR& Surface)
			: deviceContext(Context), surface(Surface) {
		}

		DeviceContext deviceContext = {};
		VkSurfaceKHR surface = {};

		QueueFamilyCollector familyIndices{};
		Vector<uint32> queueIndices{};						// 队列族索引

		SwapChainSupportDetails swapChainDetails{};			// 交换链支持功能
		VkSurfaceFormatKHR surfaceFormat{};					// 交换链表面格式
		VkPresentModeKHR presentMode{};						// 交换链呈现模式
		VkExtent2D swapChainExtent{};						// 交换链图像宽高

		static VkSwapchainCreateInfoKHR DefaultSwapChainCreateInfoKHR(const SwapChainSpec& Spec);
	};

	class SwapChain : public VulkanResourceBase
	{
	public:
		SwapChain(CommandContext* InCommandContext, const VkRenderPass& RenderPass);
		~SwapChain();

		void InitSwapChain(SwapChainSpec& Spec);

		bool Create(SwapChainSpec& Spec);
		bool CreateSwapChain(SwapChainSpec& Spec);

		void Destroy();

		void ReCreateSwapChain(BufferArea* BufferArea);
		void CleanupSwapChain();

	private:
		/* 创建图像视图 */
		void CreateImageViews();
		/* 创建深度资源 */
		void CreateDepthResources();
		/* 创建帧缓冲 */
		void CreateFrameBuffers();

		DeviceContext m_deviceContext = {};
		CommandContext* m_commandContext = nullptr;
		MainWindow* m_mainWindow = nullptr;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;

		VkSwapchainKHR m_swapChain = {};

		VkExtent2D m_swapChainExtent = {};					// 交换链宽高数值
		VkFormat m_swapChainImageFormat = {};				// 交换链图像格式

		/* 交换链图像及外层包装的视图 */
		Vector<VkImage> m_swapChainImages = {};
		Vector<VkImageView> m_swapChainImageViews = {};

		/* 深度图像资源 */
		VkImage m_depthImage = {};
		VkImageView m_depthImageView = {};
		VkDeviceMemory m_depthImageMemory = {};

		/* 帧缓冲资源 */
		Vector<UniquePtr<FrameBuffer>> m_frameBuffers = {};

	public:
		inline const VkSwapchainKHR& GetSwapChain() const { return m_swapChain; }

		inline const VkExtent2D& GetSwapChainExtent() const { return m_swapChainExtent; }
		inline const VkFormat& GetSwapChainFormat() const { return m_swapChainImageFormat; }
		inline const Vector<VkImageView>& GetSwapChainImageViews() const { return m_swapChainImageViews; }

		inline const FrameBuffer& GetFrameBuffer(usize Index) const { return *m_frameBuffers[Index]; }
	};
}