#pragma once

#include "REL/Version.h"
#include "WinAPI/WinAPI.h"
#include "stl/stl.h"

#include <fmt/core.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

namespace REL
{
	class Segment
	{
	public:
		enum Name : std::size_t
		{
			textx,
			idata,
			rdata,
			data,
			pdata,
			tls,
			textw,
			gfids,
			total
		};

		Segment() noexcept = default;

		Segment(
			std::uintptr_t a_proxyBase,
			std::uintptr_t a_address,
			std::uintptr_t a_size) noexcept
			: _proxyBase(a_proxyBase),
			  _address(a_address),
			  _size(a_size)
		{}

		[[nodiscard]] std::uintptr_t address() const noexcept { return _address; }
		[[nodiscard]] std::size_t    offset() const noexcept
		{
			return address() - _proxyBase;
		}
		[[nodiscard]] std::size_t size() const noexcept { return _size; }

		[[nodiscard]] void* pointer() const noexcept
		{
			return reinterpret_cast<void*>(address());
		}

		template <class T>
		[[nodiscard]] T* pointer() const noexcept
		{
			return static_cast<T*>(pointer());
		}

	private:
		std::uintptr_t _proxyBase{ 0 };
		std::uintptr_t _address{ 0 };
		std::size_t    _size{ 0 };
	};

	class Module
	{
	public:
		[[nodiscard]] static Module& get()
		{
			static Module singleton;
			return singleton;
		}

		[[nodiscard]] std::uintptr_t base() const noexcept { return _base; }
		[[nodiscard]] stl::zwstring  filename() const noexcept { return _filename; }
		[[nodiscard]] Version        version() const noexcept { return _version; }

		[[nodiscard]] Segment segment(Segment::Name a_segment) const noexcept
		{
			return _segments[a_segment];
		}

		[[nodiscard]] void* pointer() const noexcept
		{
			return reinterpret_cast<void*>(base());
		}

		template <class T>
		[[nodiscard]] T* pointer() const noexcept
		{
			return static_cast<T*>(pointer());
		}

	private:
		Module() { load(); }

		Module(const Module&) = delete;
		Module(Module&&)      = delete;

		~Module() noexcept = default;

		Module& operator=(const Module&) = delete;
		Module& operator=(Module&&)      = delete;

		void load()
		{
			auto handle = WinAPI::GetModuleHandle(static_cast<wchar_t*>(nullptr));
			if (handle == nullptr) {
				stl::report_and_fail(
					"Failed to obtain module handle for game executable."sv);
			}
			_base = reinterpret_cast<std::uintptr_t>(handle);

			std::wstring path;
			path.resize(4096);

			path.resize(WinAPI::GetModuleFileName(
				handle,
				path.data(),
				static_cast<std::uint32_t>(path.size())));

			_filename = std::filesystem::path(path).filename().c_str();

			load_version();
			load_segments();
		}

		void load_segments();

		void load_version()
		{
			const auto version = get_file_version(_filename);
			if (version) {
				_version = *version;
			}
			else {
				stl::report_and_fail(fmt::format(
					"Failed to obtain file version info for: {}\n"
					"Please contact the author of this script extender plugin for "
					"further assistance.",
					stl::utf16_to_utf8(_filename).value_or(
						"<unicode conversion error>"s)));
			}
		}

		static constexpr std::array SEGMENTS{
			std::make_pair(".text"sv, WinAPI::IMAGE_SCN_MEM_EXECUTE),
			std::make_pair(".idata"sv, static_cast<std::uint32_t>(0)),
			std::make_pair(".rdata"sv, static_cast<std::uint32_t>(0)),
			std::make_pair(".data"sv, static_cast<std::uint32_t>(0)),
			std::make_pair(".pdata"sv, static_cast<std::uint32_t>(0)),
			std::make_pair(".tls"sv, static_cast<std::uint32_t>(0)),
			std::make_pair(".text"sv, WinAPI::IMAGE_SCN_MEM_WRITE),
			std::make_pair(".gfids"sv, static_cast<std::uint32_t>(0))
		};

		std::wstring                        _filename;
		std::array<Segment, Segment::total> _segments;
		Version                             _version;
		std::uintptr_t                      _base{ 0 };
	};
}
