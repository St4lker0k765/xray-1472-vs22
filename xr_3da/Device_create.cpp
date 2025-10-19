#include "stdafx.h"

void CRenderDevice::_Create	(LPCSTR shName)
{
	Memory.mem_compact		();

	// after creation
	bReady					= TRUE;

	// General Render States
	mView.identity			();
	mProject.identity		();
	mFullTransform.identity	();
	vCameraPosition.set		(0,0,0);
	vCameraDirection.set	(0,0,1);
	vCameraTop.set			(0,1,0);
	vCameraRight.set		(1,0,0);

	HW.Caps.Update			();
	for (u32 i=0; i<HW.Caps.pixel.dwStages; i++) 
	{
		if (psDeviceFlags.test(rsAnisotropic))	{
			CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MINFILTER,	D3DTEXF_ANISOTROPIC ));
			CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MAGFILTER,	D3DTEXF_ANISOTROPIC ));
			CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MIPFILTER,	D3DTEXF_LINEAR		));
			CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MAXANISOTROPY, 4				));
		} else {
			CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MINFILTER,	D3DTEXF_LINEAR 		));
			CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MAGFILTER,	D3DTEXF_LINEAR 		));
			CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MIPFILTER,	D3DTEXF_LINEAR		));
			CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MAXANISOTROPY, 4				));
		}
		float fBias = -1.f;
		CHK_DX(HW.pDevice->SetSamplerState		( i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD) (&fBias))));
	}
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_DITHERENABLE,		TRUE				));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_COLORVERTEX,		TRUE				));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_ZENABLE,			TRUE				));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_SHADEMODE,			D3DSHADE_GOURAUD	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW			));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_ALPHAFUNC,			D3DCMP_GREATER		));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_LOCALVIEWER,		FALSE				));

	/*
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_POSITIONORDER,		D3DORDER_CUBIC));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_NORMALORDER,		D3DORDER_CUBIC));
	float tess = 3.f;
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_PATCHSEGMENTS,		*((u32*)&tess)));
	*/

	CHK_DX(HW.pDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_COLOR1	));

	if (psDeviceFlags.test(rsWireframe))	{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_FILLMODE,			D3DFILL_WIREFRAME	)); }
	else									{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_FILLMODE,			D3DFILL_SOLID		)); }
	if (psDeviceFlags.test(rsAntialias))	{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS,TRUE				));	}
	else									{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS,FALSE				)); }
	if (psDeviceFlags.test(rsNormalize))	{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_NORMALIZENORMALS,	TRUE				)); }
	else									{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_NORMALIZENORMALS,	FALSE				)); }

	// ******************** Fog parameters
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGCOLOR,			0					));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_RANGEFOGENABLE,	FALSE				));
	if (HW.Caps.bTableFog)	{
		CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGTABLEMODE,	D3DFOG_LINEAR		));
		CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGVERTEXMODE,	D3DFOG_NONE			));
	} else {
		CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGTABLEMODE,	D3DFOG_NONE			));
		CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGVERTEXMODE,	D3DFOG_LINEAR		));
	}

	// Signal everyone - device created
	RCache.OnDeviceCreate		();
	Gamma.Update				();
	Shader.OnDeviceCreate		(shName);
	seqDevCreate.Process		(rp_DeviceCreate);
	Statistic.OnDeviceCreate	();
	dwFrame						= 0;
}

void CRenderDevice::Create	() 
{
	if (bReady)	return;		// prevent double call
	Log("Starting RENDER device...");

	HW.CreateDevice(m_hWnd);
	dwWidth		= HW.DevPP.BackBufferWidth;
	dwHeight	= HW.DevPP.BackBufferHeight;
	fWidth_2	= float(dwWidth/2);
	fHeight_2	= float(dwHeight/2);
	fFOV		= 90.f;
	fASPECT		= 1.f;

	string256		fname; 
	FS.update_path	(fname,"$game_data$","shaders.xr");

	_Create			(fname);

	PreCache		(0);
}
