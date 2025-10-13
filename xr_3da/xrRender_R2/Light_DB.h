#pragma once

#include "..\light.h"
#include "light.h"

class CLight_DB
{
public:
	Fvector3				sun_dir;
	Fvector4				sun_color;

private:
	xr_vector<xrLIGHT_control>	v_static_controls;
	xr_vector<light*>			v_static;
	std::set<light*>				v_dynamic_active;
	std::set<light*>				v_dynamic_inactive;

	xr_vector<light*>			v_selected_shadowed;
	xr_vector<light*>			v_selected_unshadowed;
public:
	void					add_sector_lights	(xr_vector<WORD> &L);

	void					Load				(IReader* fs);
	void					Unload				();

	void					Activate			(light* L);
	void					Deactivate			(light* L);

	light*					Create				();
	void					Destroy				(light*);

	CLight_DB				();
	~CLight_DB				();
};
