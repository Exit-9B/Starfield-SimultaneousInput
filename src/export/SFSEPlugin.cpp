#include "Plugin.h"
#include "RE/Offset.Ext.h"

#include "REL/Module.h"
#include "REL/Pattern.h"
#include "REL/Relocation.h"
#include "SFSE/API.h"
#include "SFSE/Interfaces.h"
#include "SFSE/Logger.h"
#include "SFSE/Trampoline.h"

#include "RE/B/BSFixedString.h"
#include "RE/MouseMoveEvent.h"
#include "RE/UserEvents.h"

#include <spdlog/spdlog.h>
#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

#include <array>
#include <format>
#include <memory>
#include <tuple>
#include <utility>

using namespace std::string_view_literals;

#define DLLEXPORT __declspec(dllexport)
#define SFSEAPI __cdecl

namespace
{
	void InitializeLog()
	{
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = SFSE::log::log_directory();
		if (!path) {
			SFSE::stl::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= std::format("{}.log"sv, Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
			path->string(),
			true);
#endif

#ifndef NDEBUG
		const auto level = spdlog::level::trace;
#else
		const auto level = spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("%s(%#): [%^%l%$] %v");
	}
}

extern "C" DLLEXPORT constexpr auto SFSEPlugin_Version = []()
{
	SFSE::PluginVersionData v{};

	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);
	v.AuthorName("Parapets");

	v.UsesAddressLibrary(true);
	v.IsLayoutDependent(true);

	return v;
}();

namespace RE
{
	class BSInputDeviceManager;

	namespace PlayerControls
	{
		class LookHandler;
	}
}

bool IsUsingGamepad(RE::BSInputDeviceManager* a_inputDeviceManager)
{
	using func_t = decltype(IsUsingGamepad);
	REL::Relocation<func_t> func{ RE::Offset::BSInputDeviceManager::IsUsingGamepad };
	return func(a_inputDeviceManager);
}

static bool UsingThumbstickLook = false;

bool IsUsingThumbstickLook(RE::BSInputDeviceManager*)
{
	return UsingThumbstickLook;
}

bool IsGamepadCursor(RE::BSInputDeviceManager* a_inputDeviceManager)
{
	return UsingThumbstickLook && IsUsingGamepad(a_inputDeviceManager);
}

extern "C" DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
	InitializeLog();
	SFSE::log::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string("."sv));

	SFSE::Init(a_sfse);
	SFSE::AllocTrampoline(28);

	// Allow any look input
	{
		auto vtbl = REL::Relocation<std::uintptr_t>(
			RE::Offset::PlayerControls::LookHandler::Vtbl);
		vtbl.write_vfunc(
			1,
			+[](RE::PlayerControls::LookHandler*, RE::InputEvent* event) -> bool
			{
				if (RE::UserEvents::QLook() != event->QUserEvent()) {
					return false;
				}

				if (event->eventType == RE::INPUT_EVENT_TYPE::MouseMove) {
					UsingThumbstickLook = false;
				}
				else if (event->eventType == RE::INPUT_EVENT_TYPE::Thumbstick) {
					UsingThumbstickLook = true;
				}

				return true;
			});
	}

	// Prevent left thumbstick from changing device
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::BSPCGamepadDevice::Poll,
			0x2A0);

		REL::Pattern<"C6 43 08 01">().match_or_fail(hook.address());
		REL::safe_fill(hook.address(), REL::NOP, 0x4);
	}

	{
		auto hookLocs = {
			// Fix slow movement on 2 quadrants?
			std::make_pair(RE::Offset::PlayerControls::LookHandler::Func10, 0xE),
			// Fix look sensitivity
			std::make_pair(RE::Offset::PlayerControls::Manager::ProcessLookInput, 0x68),
			// Prevent cursor from escaping window
			std::make_pair(RE::Offset::Main::Run_WindowsMessageLoop, 0x39),
			// Fix mouse movement for ship reticle
			std::make_pair(RE::Offset::ShipHudDataModel::PerformInputProcessing, 0x7AF),
			std::make_pair(RE::Offset::ShipHudDataModel::PerformInputProcessing, 0x82A),
		};

		auto& trampoline = SFSE::GetTrampoline();
		for (auto [id, offset] : hookLocs) {
			auto hook = REL::Relocation<std::uintptr_t>(id, offset);

			REL::Pattern<"E8">().match_or_fail(hook.address());

			trampoline.write_call<5>(hook.address(), IsUsingThumbstickLook);
		}
	}

	{
		auto hookLocs = {
			// Show cursor for menus
			std::make_pair(RE::Offset::IMenu::ShowCursor, 0x14),
			// Use pointer style cursor
			std::make_pair(RE::Offset::UI::SetCursorStyle, 0x98),
		};

		auto& trampoline = SFSE::GetTrampoline();
		for (auto [id, offset] : hookLocs) {
			auto hook = REL::Relocation<std::uintptr_t>(id, offset);

			REL::Pattern<"E8">().match_or_fail(hook.address());
			trampoline.write_call<5>(hook.address(), IsGamepadCursor);
		}
	}

	return true;
}
