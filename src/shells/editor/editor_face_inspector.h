// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "editor.h"

class EditorMainWindow;
class EditorFaceInspector : public FXDialogBox
{
	FXDECLARE( EditorFaceInspector )

	inline EditorFaceInspector() {};

public:
	EditorFaceInspector( EditorMainWindow *parent );
};
