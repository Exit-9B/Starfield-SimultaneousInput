#pragma once

#include <cstddef>

namespace stl
{
	namespace nttp
	{
		template <class CharT, std::size_t N>
		struct string
		{
			using char_type       = CharT;
			using pointer         = char_type*;
			using const_pointer   = const char_type*;
			using reference       = char_type&;
			using const_reference = const char_type&;
			using size_type       = std::size_t;

			static constexpr auto npos = static_cast<std::size_t>(-1);

			consteval string(const_pointer a_string) noexcept
			{
				for (size_type i = 0; i < N; ++i) {
					c[i] = a_string[i];
				}
			}

			[[nodiscard]] consteval const_reference operator[](
				size_type a_pos) const noexcept
			{
				assert(a_pos < N);
				return c[a_pos];
			}

			[[nodiscard]] consteval const_reference back() const noexcept
			{
				return (*this)[size() - 1];
			}
			[[nodiscard]] consteval const_pointer data() const noexcept { return c; }
			[[nodiscard]] consteval bool          empty() const noexcept
			{
				return this->size() == 0;
			}
			[[nodiscard]] consteval const_reference front() const noexcept
			{
				return (*this)[0];
			}
			[[nodiscard]] consteval size_type length() const noexcept { return N; }
			[[nodiscard]] consteval size_type size() const noexcept { return length(); }

			template <std::size_t POS = 0, std::size_t COUNT = npos>
			[[nodiscard]] consteval auto substr() const noexcept
			{
				return string < CharT,
					   COUNT != npos ? COUNT : N - POS > (this->data() + POS);
			}

			char_type c[N] = {};
		};

		template <class CharT, std::size_t N>
		string(const CharT (&)[N]) -> string<CharT, N - 1>;
	}
}
