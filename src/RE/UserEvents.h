#pragma once

#include "RE/B/BSFixedString.h"
#include "RE/Offset.Ext.h"

namespace RE
{
	class UserEvents
	{
	public:
		static const RE::BSFixedStringCS& QLook()
		{
			using func_t = decltype(UserEvents::QLook);
			REL::Relocation<func_t> func{ Offset::UserEvents::QLook };
			return func();
		}
	};
}
