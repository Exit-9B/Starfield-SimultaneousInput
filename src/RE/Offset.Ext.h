#pragma once

#include "REL/Relocation.h"

namespace RE
{
	namespace Offset
	{
		namespace BSInputDeviceManager
		{
			constexpr REL::ID IsUsingGamepad{ 178879 };
		}

		namespace BSPCGamepadDevice
		{
			constexpr REL::ID Poll{ 179249 };
		}

		namespace IMenu
		{
			constexpr REL::ID ShowCursor{ 187256 };
		}

		namespace Main
		{
			constexpr REL::ID Run_WindowsMessageLoop{ 149028 };
		}

		namespace PlayerControls
		{
			namespace LookHandler
			{
				constexpr REL::ID Vtbl{ 407288 };
				constexpr REL::ID Func10{ 129152 };
			}

			namespace Manager
			{
				constexpr REL::ID ProcessLookInput{ 129407 };
			}
		}

		namespace ShipHudDataModel
		{
			constexpr REL::ID PerformInputProcessing{ 137087 };
		}

		namespace UI
		{
			constexpr REL::ID SetCursorStyle{ 187051 };
		}

		namespace UserEvents
		{
			constexpr REL::ID QLook{ 120233 };
		}
	}
}
