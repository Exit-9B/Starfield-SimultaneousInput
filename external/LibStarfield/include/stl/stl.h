#pragma once

#include "WinAPI/WinAPI.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <regex>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>

using namespace std::literals;

namespace stl
{
	template <class CharT>
	using basic_zstring = std::basic_string_view<CharT>;

	using zstring  = basic_zstring<char>;
	using zwstring = basic_zstring<wchar_t>;

	template <class T, class U>
	[[nodiscard]] inline auto adjust_pointer(U* a_ptr, std::ptrdiff_t a_adjust) noexcept
	{
		auto addr = a_ptr ? reinterpret_cast<std::uintptr_t>(a_ptr) + a_adjust : 0;
		if constexpr (std::is_const_v<U> && std::is_volatile_v<U>) {
			return reinterpret_cast<std::add_cv_t<T>*>(addr);
		}
		else if constexpr (std::is_const_v<U>) {
			return reinterpret_cast<std::add_const_t<T>*>(addr);
		}
		else if constexpr (std::is_volatile_v<U>) {
			return reinterpret_cast<std::add_volatile_t<T>*>(addr);
		}
		else {
			return reinterpret_cast<T*>(addr);
		}
	}

	[[nodiscard]] inline auto utf8_to_utf16(std::string_view a_in) noexcept
		-> std::optional<std::wstring>
	{
		const auto cvt = [&](wchar_t* a_dst, std::size_t a_length)
		{
			return WinAPI::MultiByteToWideChar(
				WinAPI::CP_UTF8,
				0,
				a_in.data(),
				static_cast<int>(a_in.length()),
				a_dst,
				static_cast<int>(a_length));
		};

		const auto len = cvt(nullptr, 0);
		if (len == 0) {
			return std::nullopt;
		}

		std::wstring out(len, '\0');
		if (cvt(out.data(), out.length()) == 0) {
			return std::nullopt;
		}

		return out;
	}

	[[nodiscard]] inline auto utf16_to_utf8(std::wstring_view a_in) noexcept
		-> std::optional<std::string>
	{
		const auto cvt = [&](char* a_dst, std::size_t a_length)
		{
			return WinAPI::WideCharToMultiByte(
				WinAPI::CP_UTF8,
				0,
				a_in.data(),
				static_cast<int>(a_in.length()),
				a_dst,
				static_cast<int>(a_length),
				nullptr,
				nullptr);
		};

		const auto len = cvt(nullptr, 0);
		if (len == 0) {
			return std::nullopt;
		}

		std::string out(len, '\0');
		if (cvt(out.data(), out.length()) == 0) {
			return std::nullopt;
		}

		return out;
	}

	[[noreturn]] inline void report_and_fail(
		std::string_view     a_msg,
		std::source_location a_loc = std::source_location::current())
	{
		const auto body = [&]()
		{
			const std::filesystem::path p        = a_loc.file_name();
			auto                        filename = p.lexically_normal().generic_string();

			const std::regex r{ R"((?:^|[\\\/])(?:include|src)[\\\/](.*)$)" };
			std::smatch      matches;
			if (std::regex_search(filename, matches, r)) {
				filename = matches[1].str();
			}

			return utf8_to_utf16(fmt::format("{}({}): {}", filename, a_loc.line(), a_msg))
				.value_or(L"<character encoding error>"s);
		}();

		const auto caption = []()
		{
			const auto           maxPath = WinAPI::GetMaxPath();
			std::vector<wchar_t> buf;
			buf.reserve(maxPath);
			buf.resize(maxPath / 2);
			std::uint32_t result = 0;
			do {
				buf.resize(buf.size() * 2);
				result = WinAPI::GetModuleFileName(
					WinAPI::GetCurrentModule(),
					buf.data(),
					static_cast<std::uint32_t>(buf.size()));
			} while (result && result == buf.size() &&
			         buf.size() <= std::numeric_limits<std::uint32_t>::max());

			if (result && result != buf.size()) {
				std::filesystem::path p(buf.begin(), buf.begin() + result);
				return p.filename().native();
			}
			else {
				return L""s;
			}
		}();

		spdlog::log(
			spdlog::source_loc{
				a_loc.file_name(),
				static_cast<int>(a_loc.line()),
				a_loc.function_name() },
			spdlog::level::critical,
			a_msg);
		WinAPI::MessageBox(
			nullptr,
			body.c_str(),
			(caption.empty() ? nullptr : caption.c_str()),
			0);
		WinAPI::TerminateProcess(WinAPI::GetCurrentProcess(), EXIT_FAILURE);
	}

	template <class To, class From>
	[[nodiscard]] To unrestricted_cast(From a_from) noexcept
	{
		if constexpr (std::is_same_v<std::remove_cv_t<From>, std::remove_cv_t<To>>) {
			return To{ a_from };

			// From != To
		}
		else if constexpr (std::is_reference_v<From>) {
			return stl::unrestricted_cast<To>(std::addressof(a_from));

			// From: NOT reference
		}
		else if constexpr (std::is_reference_v<To>) {
			return *stl::unrestricted_cast<
				std::add_pointer_t<std::remove_reference_t<To>>>(a_from);

			// To: NOT reference
		}
		else if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
			return static_cast<To>(
				const_cast<void*>(static_cast<const volatile void*>(a_from)));
		}
		else if constexpr (
			(std::is_pointer_v<From> && std::is_integral_v<To>) ||
			(std::is_integral_v<From> && std::is_pointer_v<To>)) {
			return reinterpret_cast<To>(a_from);
		}
		else {
			union
			{
				std::remove_cv_t<std::remove_reference_t<From>> from;
				std::remove_cv_t<std::remove_reference_t<To>>   to;
			};

			from = std::forward<From>(a_from);
			return to;
		}
	}
}
