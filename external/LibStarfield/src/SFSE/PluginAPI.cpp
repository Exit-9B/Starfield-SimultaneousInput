#pragma once

#include "SFSE/PluginAPI.h"
#include "SFSE/Log.h"
#include "stl/stl.h"

#include <functional>
#include <mutex>
#include <vector>

namespace SFSE
{
	namespace detail
	{
		struct APIStorage
		{
		public:
			[[nodiscard]] static APIStorage& get() noexcept
			{
				static APIStorage singleton;
				return singleton;
			}

			PluginHandle pluginHandle{ static_cast<PluginHandle>(kPluginHandle_Invalid) };

			TrampolineInterface* trampolineInterface{ nullptr };
			MessagingInterface*  messagingInterface{ nullptr };

			std::mutex                         apiLock;
			std::vector<std::function<void()>> apiInitRegs;
			bool                               apiInit{ false };

		private:
			APIStorage() noexcept         = default;
			APIStorage(const APIStorage&) = delete;
			APIStorage(APIStorage&&)      = delete;

			~APIStorage() noexcept = default;

			APIStorage& operator=(const APIStorage&) = delete;
			APIStorage& operator=(APIStorage&&)      = delete;
		};

		template <class T>
		T* QueryInterface(const LoadInterface* a_intfc, LoadInterface::ID a_id)
		{
			auto result = static_cast<T*>(a_intfc->QueryInterface(a_id));
			if (result && result->interfaceVersion > T::template Version) {
				log::warn("interface definition is out of date"sv);
			}
			return result;
		}
	}

	void Init(const LoadInterface* a_intfc) noexcept
	{
		if (!a_intfc) {
			stl::report_and_fail("interface is null"sv);
		}

		static_cast<void>(REL::Module::get());

		auto&       storage = detail::APIStorage::get();
		const auto& intfc   = *a_intfc;

		const std::unique_lock l{ storage.apiLock };
		if (!storage.apiInit) {
			storage.pluginHandle = intfc.GetPluginHandle();

			storage.trampolineInterface = detail::QueryInterface<TrampolineInterface>(
				a_intfc,
				LoadInterface::Trampoline);
		}
	}

	PluginHandle GetPluginHandle() noexcept
	{
		return detail::APIStorage::get().pluginHandle;
	}

	const TrampolineInterface* GetTrampolineInterface() noexcept
	{
		return detail::APIStorage::get().trampolineInterface;
	}

	const MessagingInterface* GetMessagingInterface() noexcept
	{
		return detail::APIStorage::get().messagingInterface;
	}

	Trampoline& GetTrampoline()
	{
		static Trampoline trampoline;
		return trampoline;
	}

	void AllocTrampoline(std::size_t a_size, bool a_trySKSEReserve)
	{
		auto& trampoline = GetTrampoline();
		if (auto intfc = GetTrampolineInterface(); intfc && a_trySKSEReserve) {
			auto memory = intfc->AllocateFromBranchPool(GetPluginHandle(), a_size);
			if (memory) {
				trampoline.set_trampoline(memory, a_size);
				return;
			}
		}

		trampoline.create(a_size);
	}
}
