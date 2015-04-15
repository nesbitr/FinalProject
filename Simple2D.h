/* Simple2D Graphics Library for C++
   (c) Katy Coe (sole proprietor) 2012-2013

   No unauthorized copying or distribution. You may re-use Simple2D in your own non-commercial projects.
   For commercial applications, you must obtain a commercial license.

   Support and documentation: www.djkaty.com/simple2d
*/
#pragma once

#ifndef __SIMPLE2D_INCLUDED_
#define __SIMPLE2D_INCLUDED_
#include "Simple2DLib.h"

// The entry point for your application
void Simple2DStart();

// ==========================================================================
// Windows entry point
// ==========================================================================

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Simple2DStart();
}
#endif