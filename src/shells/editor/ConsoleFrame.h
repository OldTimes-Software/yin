// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "editor.h"

namespace os::editor
{
	class ConsoleFrame : public FXVerticalFrame
	{
		FXDECLARE( ConsoleFrame )

	public:
		ConsoleFrame( FXComposite *composite );
		virtual ~ConsoleFrame();

		void PushMessage( int level, const char *msg, const PLColour &colour );

	protected:
	private:
		inline ConsoleFrame() = default;

		FXText *logField;
		FXButton *submitButton;
		FXTextField *submitField;
	};
}// namespace os::editor
