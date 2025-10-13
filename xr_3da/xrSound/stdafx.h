// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef stdafxH
#define stdafxH
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <xrCore.h>
#include <windows.h>    // базовые типы, FOURCC макросы тянут через mmsystem.h
#include <mmsystem.h>   // mmioFOURCC, MMTIME, multimedia
#include <mmreg.h>      // WAVEFORMATEX, (LP/LPC)WAVEFORMATEX
#include <msacm.h>      // ACM: fccType/fccComp use FOURCC
#include <dsound.h>     // DirectSound




#define _RELEASE(x)			{ if(x) { (x)->Release();       (x)=NULL; } }

#include "xrCDB.h"
#include "sound.h"


// TODO: reference additional headers your program requires here
#endif

