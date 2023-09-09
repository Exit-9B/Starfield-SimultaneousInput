#pragma once

#include <cassert>
#include <cstdint>

namespace RE
{
	class BSStringPool
	{
	public:
		class Entry
		{
		public:
			enum : std::uint16_t
			{
				kShallow      = 1 << 14,
				kWide         = 1 << 15,
				kRefCountMask = 0x3FFF
			};

			template <class T>
			[[nodiscard]] const T* data() const noexcept;

			template <>
			[[nodiscard]] const char* data<char>() const noexcept
			{
				return u8();
			}

			template <>
			[[nodiscard]] const wchar_t* data<wchar_t>() const noexcept
			{
				return u16();
			}

			[[nodiscard]] const Entry* leaf() const noexcept
			{
				auto iter = this;
				while (iter && iter->shallow()) {
					iter = iter->_right;
				}
				return iter;
			}

			[[nodiscard]] std::uint32_t length() const noexcept
			{
				const auto entry = leaf();
				return entry ? entry->_length : 0;
			}

			[[nodiscard]] bool shallow() const noexcept { return _flags & kShallow; }
			[[nodiscard]] std::uint32_t size() const noexcept { return length(); }

			[[nodiscard]] const char* u8() const noexcept
			{
				const auto entry = leaf();
				if (entry) {
					assert(!entry->wide());
					return reinterpret_cast<const char*>(entry + 1);
				}
				else {
					return nullptr;
				}
			}

			[[nodiscard]] const wchar_t* u16() const noexcept
			{
				const auto entry = leaf();
				if (entry) {
					assert(entry->wide());
					return reinterpret_cast<const wchar_t*>(entry + 1);
				}
				else {
					return nullptr;
				}
			}

			[[nodiscard]] bool wide() const noexcept { return _flags & kWide; }

			// members
			Entry*                 _left;  // 00
			std::uint16_t          _flags; // 08
			volatile std::uint16_t _crc;   // 08
			union
			{
				std::uint32_t _length;
				Entry*        _right;
			}; // 10
		};
		static_assert(sizeof(Entry) == 0x18);
	};
}
