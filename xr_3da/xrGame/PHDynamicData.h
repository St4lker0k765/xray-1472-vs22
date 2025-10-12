// PHDynamicData.h: interface for the PHDynamicData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#pragma warning(push)
#pragma warning(disable:4995)
#include <ode/ode.h>
#pragma warning(pop)

#include <vector>
#include <cfloat>

#include "PHInterpolation.h"

#if !defined(AFX_PHDynamicData_H__ACC01646_B581_4639_B78C_30311432021B__INCLUDED_)
#define AFX_PHDynamicData_H__ACC01646_B581_4639_B78C_30311432021B__INCLUDED_

class PHDynamicData
{
public:
    dVector3  pos;
    dMatrix3  R;
    Fmatrix   BoneTransform;
private:
    dBodyID            body = nullptr;
    CPHInterpolation*  p_parent_body_interpolation = nullptr;
    CPHInterpolation   body_interpolation;
    dGeomID            geom = nullptr;
    dGeomID            transform = nullptr;
    // PHDynamicData*   Childs;
    std::vector<PHDynamicData> Childs;
    unsigned int       numOfChilds = 0;
    Fmatrix            ZeroTransform;

public:
    inline void UpdateInterpolation() {
        body_interpolation.UpdatePositions();
        body_interpolation.UpdateRotations();
    }
    void UpdateInterpolationRecursive();
    void InterpolateTransform(Fmatrix& transform);
    void InterpolateTransformVsParent(Fmatrix& transform);
    PHDynamicData& operator [] (unsigned int i) { return Childs[i]; }
    void Destroy();
    void Create(unsigned int numOfchilds, dBodyID Body);
    void CalculateData(void);
    PHDynamicData* GetChild(unsigned int ChildNum);
    bool SetChild(unsigned int ChildNum, unsigned int numOfchilds, dBodyID body);
    void SetAsZero();
    void SetAsZeroRecursive();
    void SetZeroTransform(Fmatrix& aTransform);
    PHDynamicData(unsigned int numOfchilds, dBodyID body);
    PHDynamicData();
    virtual ~PHDynamicData();

    void GetWorldMX(Fmatrix& aTransform) {
        dMatrix3 RR;
        dQtoR(dBodyGetQuaternion(body), RR);
        DMXPStoFMX(RR, dBodyGetPosition(body), aTransform);
    }

    void GetTGeomWorldMX(Fmatrix& aTransform) {
        if (!transform) return;

        Fmatrix NormTransform, TransformMx;
        dVector3 P0 = { 0, 0, 0, -1 };
        Fvector Translate, Translate1;

        DMXPStoFMX(dBodyGetRotation(body), P0, NormTransform);
        DMXPStoFMX(dGeomGetRotation(dGeomTransformGetGeom(transform)), P0, TransformMx);

        Memory.mem_copy(&Translate,  dGeomGetPosition(dGeomTransformGetGeom(transform)), sizeof(Fvector));
        Memory.mem_copy(&Translate1, dBodyGetPosition(body),                             sizeof(Fvector));

        aTransform.identity();
        aTransform.translate_over(Translate);
        aTransform.mulA(NormTransform);
        aTransform.translate_over(Translate1);
        aTransform.mulA(TransformMx);
    }

    static inline void DMXPStoFMX(const dReal* R, const dReal* pos, Fmatrix& aTransform) {
        Memory.mem_copy(&aTransform, R,   sizeof(Fmatrix));
        aTransform.transpose();
        Memory.mem_copy(&aTransform.c, pos, sizeof(Fvector));
        aTransform._14 = 0.f;
        aTransform._24 = 0.f;
        aTransform._34 = 0.f;
        aTransform._44 = 1.f;
    }

    static inline void FMX33toDMX(const Fmatrix33& aTransform, dReal* R) {
        R[0]  = aTransform._11;
        R[4]  = aTransform._12;
        R[8]  = aTransform._13;

        R[1]  = aTransform._21;
        R[5]  = aTransform._22;
        R[9]  = aTransform._23;

        R[2]  = aTransform._31;
        R[6]  = aTransform._32;
        R[10] = aTransform._33;
    }

private:
    void CalculateR_N_PosOfChilds(dBodyID parent);

public:
    bool SetGeom(dGeomID ageom);
    bool SetTransform(dGeomID ageom);
};

#endif // !defined(AFX_PHDynamicData_H__ACC01646_B581_4639_B78C_30311432021B__INCLUDED_)
