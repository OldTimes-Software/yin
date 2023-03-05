/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl_console.h>

#include "postprocessing.h"

/****************************************
 * PRIVATE
 ****************************************/

static Material *fxaaMaterial = NULL;

static bool fxaaEnabled = false;

static void RegisterFXAAConsoleVariables( void )
{
	PlRegisterConsoleVariable( "r.fxaa", "Enable FXAA anti-aliasing.", "1", PL_VAR_BOOL, &fxaaEnabled, NULL, true );
}

static bool SetupFXAAEffect( void )
{
	fxaaMaterial = YinCore_Material_Cache( "materials/post/fxaa.mat.n", CACHE_GROUP_WORLD, false, false );
	if ( fxaaMaterial == NULL )
		return false;

	return true;
}

static void CleanupFXAAEffect( void )
{
	YinCore_Material_Release( fxaaMaterial );
}

static void DrawFXAAEffect( const YRViewport *viewport )
{
	if ( fxaaEnabled )
		return;

	YR_Draw2DQuad( fxaaMaterial, viewport->x, viewport->y, viewport->width, viewport->height );
}

/****************************************
 * PUBLIC
 ****************************************/

const PostProcessEffect *PP_FXAA_GetEffect( void )
{
	static PostProcessEffect renderFXAAPostProcess;
	PL_ZERO_( renderFXAAPostProcess );

	renderFXAAPostProcess.RegisterConsoleVariables = RegisterFXAAConsoleVariables;
	renderFXAAPostProcess.Setup                    = SetupFXAAEffect;
	renderFXAAPostProcess.Cleanup                  = CleanupFXAAEffect;
	renderFXAAPostProcess.Draw                     = DrawFXAAEffect;

	return &renderFXAAPostProcess;
}
