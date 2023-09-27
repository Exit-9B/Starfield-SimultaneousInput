#pragma once

#include "RE/InputEvent.h"

#include <cstdint>

namespace RE
{
	class IDEvent : public InputEvent
	{
	public:
		~IDEvent() override; // 00

		// override
		bool				   HasIDCode() const override;	// 01
		const BSFixedStringCS& QUserEvent() const override; // 02

		// members
		BSFixedStringCS userEvent; // 28
		std::uint64_t	unk30;
	};
	static_assert(sizeof(IDEvent) == 0x38);
}
