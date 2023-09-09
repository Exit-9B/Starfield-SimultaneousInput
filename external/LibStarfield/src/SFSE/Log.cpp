#include "SFSE/Log.h"

#include <ShlObj.h>

namespace SFSE
{
	std::optional<std::filesystem::path> LogDirectory()
	{
		wchar_t*   buffer{ nullptr };
		const auto result = ::SHGetKnownFolderPath(
			::FOLDERID_Documents,
			::KNOWN_FOLDER_FLAG::KF_FLAG_DEFAULT,
			nullptr,
			std::addressof(buffer));
		std::unique_ptr<wchar_t[], decltype(&::CoTaskMemFree)> knownPath(
			buffer,
			::CoTaskMemFree);
		if (!knownPath || result != S_OK) {
			log::error("failed to get known folder path");
			return std::nullopt;
		}

		std::filesystem::path path = knownPath.get();
		path /= "My Games";
		path /= "Starfield";
		path /= "SFSE";
		path /= "Logs";
		return path;
	}
}
