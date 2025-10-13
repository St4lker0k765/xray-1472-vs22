#include "stdafx.h"
#include "uicustomitem.h"
#include "hudmanager.h"
#include <cmath>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUICustomItem::CUICustomItem()
{    
	uAlign			= alNone;
	uFlags			= 0;
	iVisRect.lt.set	(0,0); iVisRect.rb.set(0,0);
}
//--------------------------------------------------------------------

CUICustomItem::~CUICustomItem()
{
}
//--------------------------------------------------------------------
 
void CUICustomItem::Render(FVF::TL*& Pointer, const Ivector2& pos, u32 color, int x1, int y1, int x2, int y2)
{
	CTexture* T		= RCache.get_ActiveTexture(0);
	Ivector2		ts;
	Fvector2		hp;
	ts.set			((int)T->get_Width(),(int)T->get_Height());
	hp.set			(0.5f/float(ts.x),0.5f/float(ts.y));
	if (!(uFlags&flValidRect))	SetRect		(0,0,ts.x,ts.y);

	Fvector2 LTp,RBp;
	Fvector2 LTt,RBt;
	float sc		= HUD().GetScale();
	LTp.set			(pos.x+x1*sc,pos.y+y1*sc);
	RBp.set			(pos.x+x2*sc,pos.y+y2*sc);

	LTt.set			(float(x1)/float(ts.x)+hp.x,float(y1)/float(ts.y)+hp.y);
	RBt.set			(float(x2)/float(ts.x)+hp.x,float(y2)/float(ts.y)+hp.y);

	Pointer->set	(LTp.x,	RBp.y,	color, LTt.x, RBt.y); Pointer++;
	Pointer->set	(LTp.x,	LTp.y,	color, LTt.x, LTt.y); Pointer++;
	Pointer->set	(RBp.x,	RBp.y,	color, RBt.x, RBt.y); Pointer++;
	Pointer->set	(RBp.x,	LTp.y,	color, RBt.x, LTt.y); Pointer++;
}
//--------------------------------------------------------------------

void CUICustomItem::Render(FVF::TL*& Pointer, const Ivector2& pos, u32 color)
{
	Render(Pointer,pos,color,iVisRect.x1,iVisRect.y1,iVisRect.x2,iVisRect.y2);
}
//--------------------------------------------------------------------

void CUICustomItem::Render(FVF::TL*& Pointer, const Ivector2& pos, u32 color, float angle)
{
    CTexture* T = RCache.get_ActiveTexture(0);
    Ivector2 ts;
    Fvector2 hp;
    ts.set((int)T->get_Width(), (int)T->get_Height());
    hp.set(0.5f / float(ts.x), 0.5f / float(ts.y));
    if (!(uFlags & flValidRect)) SetRect(0, 0, ts.x, ts.y);

    angle -= PI_DIV_4;
    const float s1 = std::sinf(angle);
    const float c1 = std::cosf(angle);
    angle += PI_DIV_2;
    const float s2 = std::sinf(angle);
    const float c2 = std::cosf(angle);

    Fvector2 C;
    Ivector2 RS;
    iVisRect.getsize(RS);
    float sc = HUD().GetScale();
    float sz = sc * ((RS.x > RS.y) ? RS.x : RS.y) * 0.7071f;

    Fvector2 LTt, RBt;
    LTt.set(float(iVisRect.x1) / float(ts.x) + hp.x, float(iVisRect.y1) / float(ts.y) + hp.y);
    RBt.set(float(iVisRect.x2) / float(ts.x) + hp.x, float(iVisRect.y2) / float(ts.y) + hp.y);

    // Rotation
    iVisRect.getcenter(RS);
    C.set(RS.x * sc + pos.x, RS.y * sc + pos.y);

    Pointer->set(C.x + s1 * sz, C.y + c1 * sz, color, LTt.x, RBt.y); Pointer++;
    Pointer->set(C.x - s2 * sz, C.y - c2 * sz, color, LTt.x, LTt.y); Pointer++;
    Pointer->set(C.x + s2 * sz, C.y + c2 * sz, color, RBt.x, RBt.y); Pointer++;
    Pointer->set(C.x - s1 * sz, C.y - c1 * sz, color, RBt.x, LTt.y); Pointer++;
}
//--------------------------------------------------------------------
