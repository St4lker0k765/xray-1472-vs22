#include "stdafx.h"
#pragma hdrstop

#include "Blender_fat_flat.h"

CBlender_fat_flat::CBlender_fat_flat	()	{	description.CLS		= B_DEFAULT;	}
CBlender_fat_flat::~CBlender_fat_flat	()	{	}

void	CBlender_fat_flat::Save	(	IWriter& fs )
{
	CBlender::Save	(fs);
}
void	CBlender_fat_flat::Load	(	IReader& fs, WORD version )
{
	CBlender::Load	(fs,version);
}
void	CBlender_fat_flat::Compile(CBlender_Compile& C)
{
	CBlender::Compile		(C);

	C.r2_Pass				("fat_flat_base","fat_flat_base");
	C.r2_Sampler			("s_base",C.L_textures[0]);
	C.r2_End				();
}
