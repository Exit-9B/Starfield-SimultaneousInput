#pragma once

#include "REL/Relocation.h"

namespace RE
{
	namespace Offset
	{
		namespace BSInputDeviceManager
		{
			constexpr REL::Offset IsUsingGamepad{ 0x2C92620 };
		}

		namespace BSPCGamepadDevice
		{
			constexpr REL::Offset Poll{ 0x2CB151C };
		}

		namespace IMenu
		{
			constexpr REL::Offset ShowCursor{ 0x2E7A220 };
		}

		namespace Main
		{
			constexpr REL::Offset Run_WindowsMessageLoop{ 0x23F79F4 };
		}

		namespace PlayerControls
		{
			namespace LookHandler
			{
				constexpr REL::Offset Vtbl{ 0x4474EC0 };
				constexpr REL::Offset Func10{ 0x1F475D0 };
			}

			namespace Manager
			{
				constexpr REL::Offset ProcessLookInput{ 0x1F4EDA8 };
			}
		}

		namespace ShipHudDataModel
		{
			constexpr REL::Offset PerformInputProcessing{ 0x20FDA98 };
		}

		namespace UI
		{
			constexpr REL::Offset SetCursorStyle{ 0x2E70B64 };
		}
	}
}
