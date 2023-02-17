
#pragma once

#ifndef __GA_FeE_PointInMesh_h__
#define __GA_FeE_PointInMesh_h__

//#include "GA_FeE/GA_FeE_PointInMesh.h"

#include "GA/GA_Detail.h"


//#include "GA_FeE/GA_FeE_Type.h


namespace GA_FeE_PointInMesh {





    template<typename T>
    static UT_Matrix4T<T>
        pointInMesh_wn(
            GA_Detail* const geo,
            const GA_Detail* const refGeo,
            const GA_Detail* restGeo,
            const UT_StringHolder& attribToXformPattern,
            const GA_ROHandleT<UT_Vector3T<T>>& posRest_h,
            const GA_ROHandleT<UT_Vector3T<T>>& posRef_h
        ) {
        if (!attribToXformPattern.isstring() || attribToXformPattern.isEmpty())
            return nullptr;

        UT_ASSERT_P(geo);
        UT_ASSERT_P(refGeo);

        if (!restGeo)
            restGeo = geo;

        UT_Matrix4T<T> mtx;
    }



    static void
        pointInMesh_wn(
            const SOP_NodeVerb::CookParms& cookparms,
            GA_Detail* const geo,
            const GA_Detail* const refGeo,
            const GA_Detail* restGeo,
            const UT_StringHolder& attribToXformPattern,
            const UT_StringHolder& restPosAttribName = "",
            const UT_StringHolder& refPosAttribName = ""
        ) {
        if (!attribToXformPattern.isstring() || attribToXformPattern.isEmpty())
            return;

        UT_ASSERT_P(geo);
        UT_ASSERT_P(refGeo);

        if (!restGeo)
            restGeo = geo;

        const GA_Attribute* posRestAttribPtr = restGeo->findPointAttribute(restPosAttribName);
        if(!posRestAttribPtr)
            posRestAttribPtr = restGeo->findPointAttribute("P");

        const GA_Attribute* posRefAttribPtr = refGeo->findPointAttribute(refPosAttribName);
        if (!posRefAttribPtr)
            posRefAttribPtr = refGeo->findPointAttribute("P");

        pointInMesh_wn<fpreal>(geo, refGeo, restGeo, attribToXformPattern, posRestAttribPtr, posRefAttribPtr);

    }

    static GA_PointGroup*
    addGroupPointInMesh_wn
        const SOP_NodeVerb::CookParms& cookparms,
        GA_Detail* const geo,
        const GA_Detail* const refGeo,
        const GA_Detail* restGeo,
        const UT_StringHolder& attribToXformPattern,
        const UT_StringHolder& restPosAttribName = "",
        const UT_StringHolder& refPosAttribName = ""
        ) {
        if (!attribToXformPattern.isstring() || attribToXformPattern.isEmpty())
            return;

        UT_ASSERT_P(geo);
        UT_ASSERT_P(refGeo);

        if (!restGeo)
            restGeo = geo;

        const GA_Attribute* posRestAttribPtr = restGeo->findPointAttribute(restPosAttribName);
        if (!posRestAttribPtr)
            posRestAttribPtr = restGeo->findPointAttribute("P");

        const GA_Attribute* posRefAttribPtr = refGeo->findPointAttribute(refPosAttribName);
        if (!posRefAttribPtr)
            posRefAttribPtr = refGeo->findPointAttribute("P");

        pointInMesh_wn<fpreal>(geo, refGeo, restGeo, attribToXformPattern, posRestAttribPtr, posRefAttribPtr);
    }








} // End of namespace GA_FeE_PointInMesh

#endif