#pragma once

#include "REL/Version.h"
#include "SFSE/Trampoline.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <string_view>

namespace SFSE
{
	typedef std::uint32_t PluginHandle;

	struct PluginInfo
	{
		enum
		{
			kInfoVersion = 1
		};

		std::uint32_t infoVersion;
		const char*   name;
		std::uint32_t version;
	};

	enum
	{
		kPluginHandle_Invalid = 0xFFFFFFFF
	};

	struct LoadInterface
	{
		static constexpr std::uint32_t Version = 1;
		enum ID : std::uint32_t
		{
			Invalid = 0,
			Messaging,
			Trampoline,
			Max,
		};

		std::uint32_t sfseVersion;
		std::uint32_t runtimeVersion;
		std::uint32_t interfaceVersion;
		void* (*QueryInterface)(ID id);

		PluginHandle (*GetPluginHandle)(void);

		const PluginInfo* (*GetPluginInfo)(const char* name);
	};

	struct MessagingInterface
	{
		static constexpr std::uint32_t     Version = 1;
		static constexpr LoadInterface::ID ID      = LoadInterface::Messaging;

		struct Message
		{
			const char*   sender;
			std::uint32_t type;
			std::uint32_t dataLen;
			void*         data;
		};

		using EventCallback = void (*)(Message* msg);

		// SFSE messages
		enum
		{
			kMessage_PostLoad, // sent to registered plugins once all plugins have been
			                   // loaded (no data)
			kMessage_PostPostLoad, // sent right after kMessage_PostPostLoad to facilitate
			                       // the correct dispatching/registering of
			                       // messages/listeners
		};

		std::uint32_t interfaceVersion;

		bool (*RegisterListener)(
			PluginHandle  listener,
			const char*   sender,
			EventCallback handler);

		bool (*Dispatch)(
			PluginHandle  sender,
			std::uint32_t messageType,
			void*         data,
			std::uint32_t dataLen,
			const char*   receiver);
	};

	struct TrampolineInterface
	{
		static constexpr std::uint32_t     Version = 1;
		static constexpr LoadInterface::ID ID      = LoadInterface::Trampoline;

		std::uint32_t interfaceVersion;

		void* (*AllocateFromBranchPool)(PluginHandle plugin, std::size_t size);
		void* (*AllocateFromLocalPool)(PluginHandle plugin, std::size_t size);
	};

	struct PluginVersionData
	{
	public:
		enum
		{
			kVersion = 1,
		};

		constexpr void AuthorName(std::string_view a_name) noexcept
		{
			SetCharBuffer(a_name, std::span{ author });
		}

		constexpr void CompatibleVersions(
			std::initializer_list<REL::Version> a_versions) noexcept
		{
			assert(a_versions.size() < std::size(compatibleVersions) - 1);
			std::transform(
				a_versions.begin(),
				a_versions.end(),
				std::begin(compatibleVersions),
				[](const REL::Version& a_version) noexcept { return a_version.pack(); });
		}

		constexpr void MinimumRequiredXSEVersion(REL::Version a_version) noexcept
		{
			xseMinimum = a_version.pack();
		}

		constexpr void PluginName(std::string_view a_plugin) noexcept
		{
			SetCharBuffer(a_plugin, std::span{ pluginName });
		}

		constexpr void PluginVersion(REL::Version a_version) noexcept
		{
			pluginVersion = a_version.pack();
		}

		constexpr void HasNoStructUse(bool a_value) noexcept { noStructUse = a_value; }

		constexpr void UsesAddressLibrary(bool a_value) noexcept
		{
			addressLibrary = a_value;
		}

		constexpr void UsesSigScanning(bool a_value) noexcept { sigScanning = a_value; }

		constexpr void UsesInitialLayout(bool a_value) noexcept
		{
			initialLayout = a_value;
		}

		const std::uint32_t dataVersion{ kVersion };
		std::uint32_t       pluginVersion          = 0;
		char                pluginName[256]        = {};
		char                author[256]            = {};
		bool                sigScanning    : 1     = false;
		bool                addressLibrary : 1     = false;
		std::uint8_t        padding1       : 6     = 0;
		std::uint8_t        padding2               = 0;
		std::uint16_t       padding3               = 0;
		bool                noStructUse   : 1      = false;
		bool                initialLayout : 1      = true;
		std::uint8_t        padding4      : 6      = 0;
		std::uint8_t        padding5               = 0;
		std::uint16_t       padding6               = 0;
		std::uint32_t       compatibleVersions[16] = {};
		std::uint32_t       xseMinimum             = 0;
		std::uint32_t       reservedNonBreaking    = 0;
		std::uint32_t       reservedBreaking       = 0;

	private:
		static constexpr void SetCharBuffer(
			std::string_view a_src,
			std::span<char>  a_dst) noexcept
		{
			assert(a_src.size() < a_dst.size());
			std::fill(a_dst.begin(), a_dst.end(), '\0');
			std::copy(a_src.begin(), a_src.end(), a_dst.begin());
		}
	};
	static_assert(offsetof(PluginVersionData, dataVersion) == 0x000);
	static_assert(offsetof(PluginVersionData, pluginVersion) == 0x004);
	static_assert(offsetof(PluginVersionData, pluginName) == 0x008);
	static_assert(offsetof(PluginVersionData, author) == 0x108);
	static_assert(offsetof(PluginVersionData, padding2) == 0x209);
	static_assert(offsetof(PluginVersionData, padding3) == 0x20A);
	static_assert(offsetof(PluginVersionData, padding5) == 0x20D);
	static_assert(offsetof(PluginVersionData, padding6) == 0x20E);
	static_assert(offsetof(PluginVersionData, compatibleVersions) == 0x210);
	static_assert(offsetof(PluginVersionData, xseMinimum) == 0x250);
	static_assert(offsetof(PluginVersionData, reservedNonBreaking) == 0x254);
	static_assert(offsetof(PluginVersionData, reservedBreaking) == 0x258);
	static_assert(sizeof(PluginVersionData) == 0x25C);

	void Init(const LoadInterface* a_intfc) noexcept;
	void RegisterForAPIInitEvent(std::function<void()> a_fn);

	PluginHandle  GetPluginHandle() noexcept;
	std::uint32_t GetReleaseIndex() noexcept;

	const TrampolineInterface* GetTrampolineInterface() noexcept;
	const MessagingInterface*  GetMessagingInterface() noexcept;

	Trampoline& GetTrampoline();
	void        AllocTrampoline(std::size_t a_size, bool a_trySFSEReserve = true);
}
