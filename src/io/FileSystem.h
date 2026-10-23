#pragma once

#include <filesystem>

class FileSystem
{
public:
	static std::filesystem::path GetExecutableFolder();

	static std::filesystem::path GetShadersFolder();

	static std::filesystem::path GetAssetsFolder();
};