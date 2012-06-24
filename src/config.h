#pragma once

#define _WIN32_WINNT 0x600
#define WIN32_LEAN_AND_MEAN 1
#define UNICODE  1
#define _UNICODE 1

#ifdef  _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <windows.h>

#ifdef _MSC_VER
#include <WindowsX.h>
#include <ObjBase.h>
#include <StrSafe.h>

#include <ShellApi.h>
#include <ShlObj.h>
#include <GdiPlus.h>
#include <ActivScp.h>

#include <ComDef.h>
#include <ComDefSp.h>
#include <ComUtil.h>

#include <stdio.h>
#include <malloc.h>
#include <exception>
#endif

#define WINCLASS_NAME L"eckApplicationClass"
#define PACKAGE_NAME L"eck"

//EOF
