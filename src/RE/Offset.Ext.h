#pragma once

#include "REL/Relocation.h"

namespace RE
{
	namespace Offset
	{
		namespace BSInputDeviceManager
		{
			constexpr REL::Offset IsUsingGamepad{ 0x2C92960 };
		}

		namespace BSPCGamepadDevice
		{
			constexpr REL::Offset Poll{ 0x2CB185C };
		}

		namespace IMenu
		{
			constexpr REL::Offset ShowCursor{ 0x2E7A570 };
		}

		namespace Main
		{
			constexpr REL::Offset Run_WindowsMessageLoop{ 0x23F7984 };
		}

		namespace PlayerControls
		{
			namespace LookHandler
			{
				constexpr REL::Offset Vtbl{ 0x4473F08 };
				constexpr REL::Offset Func10{ 0x1F47600 };
			}

			namespace Manager
			{
				constexpr REL::Offset ProcessLookInput{ 0x1F4EDD8 };
			}
		}

		namespace ShipHudDataModel
		{
			constexpr REL::Offset PerformInputProcessing{ 0x20FDAC8 };
		}

		namespace UI
		{
			constexpr REL::Offset SetCursorStyle{ 0x2E70EB4 };
		}
	}
}
