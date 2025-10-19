#include "stdafx.h"

void CRenderDevice::_Destroy	(BOOL bKeepTextures)
{
	// before destroy
	bReady						= FALSE;
	Statistic.OnDeviceDestroy	();
	seqDevDestroy.Process		(rp_DeviceDestroy);
	Shader.OnDeviceDestroy		(bKeepTextures);
	RCache.OnDeviceDestroy		();

	Memory.mem_compact			();
}

void CRenderDevice::Destroy	(void) {
	if (!bReady) return;

	Log("Destroying Direct3D...");

	ShowCursor					(TRUE);
	HW.Validate					();

	_Destroy					(FALSE);

	// real destroy
	HW.DestroyDevice			();
}

void CRenderDevice::Reset		(LPCSTR shName, BOOL bKeepTextures)
{
	ShowCursor				(TRUE);
	u32 tm_start			= TimerAsync();
	Memory.mem_compact		();
	HW.Reset				(m_hWnd);
	dwWidth					= HW.DevPP.BackBufferWidth;
	dwHeight				= HW.DevPP.BackBufferHeight;
	fWidth_2				= float(dwWidth/2);
	fHeight_2				= float(dwHeight/2);
	PreCache				(10);
	u32 tm_end				= TimerAsync();
	Msg						("*** RESET [%d ms]",tm_end-tm_start);
	ShowCursor				(FALSE);
}
