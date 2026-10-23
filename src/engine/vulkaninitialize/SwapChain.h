#pragma once

#include "BaseObject.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace suz
{
	class BufferArea;
	class MainWindow;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;				// 基本表面功能
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class SwapChain : public BaseObject
	{
	public:
		SwapChain(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const VkSurfaceKHR& Surface);
		~SwapChain();

		virtual void Destroy() override;

		static SwapChainSupportDetails QuerySupportChainSupport(const VkPhysicalDevice& Device, const VkSurfaceKHR& Surface);

		/* 选择最合适的表面格式 (绘制纹理) */
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
		/* 选择最合适的呈现格式 (图像显示到屏幕的条件) */
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes);

		void ReCreateSwapChain(BufferArea* BufferArea);
		void CleanupSwapChain();

	private:
		void CreateSwapChain();
		/* 创建图像视图 */
		void CreateImageViews();

		VkExtent2D GetSwapChainExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

		const VkPhysicalDevice& m_physicalDevice;
		const VkDevice& m_device;
		const VkSurfaceKHR& m_surface;

		MainWindow* m_mainWindow = nullptr;

		VkExtent2D m_swapChainExtent = {};

		VkSwapchainKHR m_swapChain = {};
		VkFormat m_swapChainImageFormat = {};
		std::vector<VkImage> m_swapChainImages = {};
		std::vector<VkImageView> m_swapChainImageViews = {};

	public:
		inline const VkExtent2D& GetSwapChainExtent()const { return m_swapChainExtent; }
		inline const VkSwapchainKHR& GetSwapChain() const { return m_swapChain; }
		inline const VkFormat& GetSwapChainFormat() const { return m_swapChainImageFormat; }
		inline const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_swapChainImageViews; }
	};
}