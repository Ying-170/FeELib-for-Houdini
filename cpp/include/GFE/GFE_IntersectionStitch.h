
#pragma once

#ifndef __GFE_IntersectionStitch_h__
#define __GFE_IntersectionStitch_h__

#include "GFE/GFE_IntersectionStitch.h"

#include "GFE/GFE_GeoFilter.h"




#include "GFE/GFE_Enumerate.h"
#include "GFE/GFE_NodeVerb.h"
#include "SOP/SOP_IntersectionStitch.proto.h"

class GFE_IntersectionStitch : public GFE_AttribFilterWithRef {

public:
    using GFE_AttribFilterWithRef::GFE_AttribFilterWithRef;
    
    void
        setComputeParm(
            const OutType outType      = OutType::Point,
            const bool splitCurve      = false,
            const bool repairResult    = false,
            const bool triangulateMesh = false,
            const bool keepPointAttrib = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        
        this->outType         = outType;
        this->splitCurve      = splitCurve;
        this->repairResult    = repairResult;
        this->triangulateMesh = triangulateMesh;
        this->keepPointAttrib = keepPointAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }
    
    SYS_FORCE_INLINE void setTolerance()
    { useTol = false;  }
    
    SYS_FORCE_INLINE void setTolerance(const bool tolerance)
    { useTol = true; this->tolerance = outSecondInputGeo; }


    
    SYS_FORCE_INLINE void setInsertPoint(const bool outSecondInputGeo)
    { insertPoint = false; this->outSecondInputGeo = outSecondInputGeo; }
    
    SYS_FORCE_INLINE void setInsertPoint()
    { insertPoint = true; }


    
    SYS_FORCE_INLINE void setInputnumAttrib(const GA_Attribute* const inAttrib)
    { inputnumAttrib = inAttrib->getOwner()==GA_ATTRIB_POINT && &inAttrib->getDetail() == geoRef0 ? inAttrib : nullptr; }
    
    SYS_FORCE_INLINE const GA_Attribute* setInputnumAttrib(const UT_StringRef& name)
    { return inputnumAttrib = geoRef0->findPointAttribute(name); }

    
    SYS_FORCE_INLINE void setPrimnumAttrib(const GA_Attribute* const inAttrib)
    { primnumAttrib = inAttrib->getOwner()==GA_ATTRIB_POINT && &inAttrib->getDetail() == geoRef0 ? inAttrib : nullptr; }
    
    SYS_FORCE_INLINE const GA_Attribute* setPrimnumAttrib(const UT_StringRef& name)
    { return primnumAttrib = geoRef0->findPointAttribute(name); }

    
    SYS_FORCE_INLINE void setPrimuvwAttrib(const GA_Attribute* const inAttrib)
    { primuvwAttrib = inAttrib->getOwner()==GA_ATTRIB_POINT && &inAttrib->getDetail() == geoRef0 ? inAttrib : nullptr; }
    
    SYS_FORCE_INLINE const GA_Attribute* setPrimuvwAttrib(const UT_StringRef& name)
    { return primuvwAttrib = geoRef0->findPointAttribute(name); }


    
    
private:


    
#define __TEMP_GFE_IntersectionStitch_InGroupName  "__TEMP_GFE_IntersectionStitch_InGroup"
#define __TEMP_GFE_IntersectionStitch_RefGroupName "__TEMP_GFE_IntersectionStitch_RefGroup"
    
#define __TEMP_GFE_IntersectionStitch_inputnumAttribName  "__TEMP_GFE_IntersectionStitch_inputnumAttrib"
#define __TEMP_GFE_IntersectionStitch_primnumAttribName   "__TEMP_GFE_IntersectionStitch_primnumAttrib"
#define __TEMP_GFE_IntersectionStitch_primuvwAttribName   "__TEMP_GFE_IntersectionStitch_primuvwAttrib"
    
    virtual bool
        computeCore() override
    {
        UT_ASSERT_MSG_P(interStitchVerb, "Does not have interStitch Verb");
        
        UT_ASSERT_MSG_P(cookparms, "Does not have cookparms");
        if (!cookparms)
            return false;
        
        if (insertPoint && !(inputnumAttrib && primnumAttrib && primuvwAttrib))
            return false;
            
        if (groupParser.isEmpty())
            return true;
        
        const bool repairResultFinal = repairResult && (insertPoint || !outSecondInputGeo);
        if (repairResultFinal)
        {
            ptnumNewLast = geo->getNumPoints();
            numprimInput = geo->getNumPrimitives();
        }
        
        GU_DetailHandle geoTmp_h;
        geoTmp = new GU_Detail();
        geoTmp_h.allocateAndSet(geoTmp);
        geoTmp->replaceWith(*geo);
        
        GU_DetailHandle geoRefTmp_h;
        geoRefTmp = new GU_Detail();
        geoRefTmp_h.allocateAndSet(geoRefTmp);
        geoRefTmp->replaceWith(*geoRef0);

        GFE_Enumerate enumerate0(geoTmp, cookparms);
        GFE_Enumerate enumerate1(geoRefTmp, cookparms);
        if (triangulateMesh)
        {
            enumAttrib0 = enumerate0.findOrCreateTuple(true, GA_ATTRIB_PRIMITIVE);
            enumAttrib1 = enumerate1.findOrCreateTuple(true, GA_ATTRIB_PRIMITIVE);
            enumerate0.compute();
            enumerate1.compute();
            
            geoTmp->convex();
            geoRefTmp->convex();
        }
        else
        {
            enumAttrib0 = nullptr;
            enumAttrib1 = nullptr;
        }
        // GU_DetailHandle geoEmpty_h;
        // GU_Detail* geoEmpty = new GU_Detail();
        // //GU_Detail* const geoEmpty = nullptr;
        // geoEmpty_h.allocateAndSet(geoEmpty);
        
        inputgdh.clear();
        inputgdh.emplace_back(geoTmp_h);
        if (insertPoint)
        {
            //inputgdh.emplace_back(geoEmpty_h);
            inputgdh.emplace_back(GU_DetailHandle());
            inputgdh.emplace_back(geoRefTmp_h);
        }
        else
        {
            inputgdh.emplace_back(geoRefTmp_h);
            inputgdh.emplace_back(GU_DetailHandle());
            //inputgdh.emplace_back(geoEmpty_h);
        }
        
        intersectionStitch();

        if (repairResultFinal)
            repair();
        
        //geo->getGroupTable(GA_GROUP_PRIMITIVE)->destroy(__TEMP_GFE_IntersectionStitch_InGroupName);
        //geo->getGroupTable(insertPoint ? GA_GROUP_POINT : GA_GROUP_PRIMITIVE)->destroy(__TEMP_GFE_IntersectionStitch_RefGroupName);
        
        return true;
    }


    void intersectionAnalysis()
    {
        interAnalysisParms.setUseproxtol(useTol);
        interAnalysisParms.setProxtol(tolerance);
        interAnalysisParms.setSplitcurves(splitCurve);
        interAnalysisParms.setKeeppointattribs(keepPointAttrib);

        
        if (groupParser.getHasGroup())
        {
            if (groupParser.isDetached() || !groupParser.isPrimitiveGroup())
            {
                GA_PrimitiveGroup* const inGroup = geoTmp->newPrimitiveGroup(__TEMP_GFE_IntersectionStitch_InGroupName);
                GFE_GroupUnion::groupUnion(inGroup, groupParser.getPrimitiveGroup());
                //inGroup->combine(groupParser.getPrimitiveGroup());
                interAnalysisParms.setAgroup(__TEMP_GFE_IntersectionStitch_InGroupName);
            }
            else
            {
                interAnalysisParms.setAgroup(groupParser.getName());
            }
        }
        else
        {
            interAnalysisParms.setAgroup("");
        }
        
        if (groupParserRef0.getHasGroup())
        {
            if (groupParserRef0.isDetached() || !(insertPoint ? groupParserRef0.isPointGroup() : groupParserRef0.isPrimitiveGroup()))
            {
                GA_Group* const refGroup = geoRefTmp->getGroupTable(insertPoint ? GA_GROUP_POINT : GA_GROUP_PRIMITIVE)->newGroup(__TEMP_GFE_IntersectionStitch_RefGroupName);
                
                if (insertPoint)
                    GFE_GroupUnion::groupUnion(refGroup, groupParserRef0.getPointGroup());
                    //refGroup->combine(groupParserRef0.getPointGroup());
                else
                    GFE_GroupUnion::groupUnion(refGroup, groupParserRef0.getPrimitiveGroup());
                    //refGroup->combine(groupParserRef0.getPrimitiveGroup());
                // if (insertPoint)
                //     static_cast<GA_PointGroup&>(*refGroup) = *groupParser.getPointGroup();
                // else
                //     static_cast<GA_PrimitiveGroup&>(*refGroup) = *groupParser.getPrimitiveGroup();
                
                interAnalysisParms.setBgroup(__TEMP_GFE_IntersectionStitch_RefGroupName);
            }
            else
            {
                interAnalysisParms.setBgroup(groupParserRef0.getName());
            }
        }
        else
        {
            interAnalysisParms.setAgroup("");
        }
        
        
        
        if (insertPoint)
        {
            UT_ASSERT_P(inputnumAttrib);
            UT_ASSERT_P(primnumAttrib);
            UT_ASSERT_P(primuvwAttrib);
            
            const bool detachedInputnum = inputnumAttrib->isDetached();
            const bool detachedPrimnum  = primnumAttrib->isDetached();
            const bool detachedPrimuvw  = primuvwAttrib->isDetached();

            if (detachedInputnum)
                GFE_Attribute::clone(*geoRefTmp, *inputnumAttrib, __TEMP_GFE_IntersectionStitch_inputnumAttribName);
            if (detachedPrimnum)
                GFE_Attribute::clone(*geoRefTmp, *primnumAttrib,  __TEMP_GFE_IntersectionStitch_primnumAttribName);
            if (detachedPrimuvw)
                GFE_Attribute::clone(*geoRefTmp, *primuvwAttrib,  __TEMP_GFE_IntersectionStitch_primuvwAttribName);
            
            interAnalysisParms.setInputnumattrib(detachedInputnum ? UT_StringHolder(__TEMP_GFE_IntersectionStitch_inputnumAttribName) : inputnumAttrib->getName());
            interAnalysisParms.setPrimnumattrib (detachedPrimnum  ? UT_StringHolder(__TEMP_GFE_IntersectionStitch_primnumAttribName)  : primnumAttrib->getName());
            interAnalysisParms.setPrimuvwattrib (detachedPrimuvw  ? UT_StringHolder(__TEMP_GFE_IntersectionStitch_primuvwAttribName)  : primuvwAttrib->getName());
        }
        
        geo->clear();
        GU_DetailHandle destgdh;
        destgdh.allocateAndSet(geo->asGU_Detail(), false);
        SOP_NodeCache* const nodeCache = interAnalysisVerb->allocCache();
        const auto interCookparms = GFE_NodeVerb::newCookParms(cookparms, interAnalysisParms, nodeCache, &destgdh, &inputgdh);
        interAnalysisVerb->cook(interCookparms);
        
    }



    
    void intersectionStitch()
    {
        interStitchParms.setUseproxtol(useTol);
        interStitchParms.setProxtol(tolerance);
        interStitchParms.setSplitcurves(splitCurve);
        interStitchParms.setKeeppointattribs(keepPointAttrib);

        
        if (groupParser.getHasGroup())
        {
            if (groupParser.isDetached() || !groupParser.isPrimitiveGroup())
            {
                GA_PrimitiveGroup* const inGroup = geoTmp->newPrimitiveGroup(__TEMP_GFE_IntersectionStitch_InGroupName);
                GFE_GroupUnion::groupUnion(inGroup, groupParser.getPrimitiveGroup());
                //inGroup->combine(groupParser.getPrimitiveGroup());
                interStitchParms.setAgroup(__TEMP_GFE_IntersectionStitch_InGroupName);
            }
            else
            {
                interStitchParms.setAgroup(groupParser.getName());
            }
        }
        else
        {
            interStitchParms.setAgroup("");
        }
        
        if (groupParserRef0.getHasGroup())
        {
            if (groupParserRef0.isDetached() || !(insertPoint ? groupParserRef0.isPointGroup() : groupParserRef0.isPrimitiveGroup()))
            {
                GA_Group* const refGroup = geoRefTmp->getGroupTable(insertPoint ? GA_GROUP_POINT : GA_GROUP_PRIMITIVE)->newGroup(__TEMP_GFE_IntersectionStitch_RefGroupName);
                
                if (insertPoint)
                    GFE_GroupUnion::groupUnion(refGroup, groupParserRef0.getPointGroup());
                    //refGroup->combine(groupParserRef0.getPointGroup());
                else
                    GFE_GroupUnion::groupUnion(refGroup, groupParserRef0.getPrimitiveGroup());
                    //refGroup->combine(groupParserRef0.getPrimitiveGroup());
                // if (insertPoint)
                //     static_cast<GA_PointGroup&>(*refGroup) = *groupParser.getPointGroup();
                // else
                //     static_cast<GA_PrimitiveGroup&>(*refGroup) = *groupParser.getPrimitiveGroup();
                
                interStitchParms.setBgroup(__TEMP_GFE_IntersectionStitch_RefGroupName);
            }
            else
            {
                interStitchParms.setBgroup(groupParserRef0.getName());
            }
        }
        else
        {
            interStitchParms.setAgroup("");
        }
        
        
        
        if (insertPoint)
        {
            UT_ASSERT_P(inputnumAttrib);
            UT_ASSERT_P(primnumAttrib);
            UT_ASSERT_P(primuvwAttrib);
            
            const bool detachedInputnum = inputnumAttrib->isDetached();
            const bool detachedPrimnum  = primnumAttrib->isDetached();
            const bool detachedPrimuvw  = primuvwAttrib->isDetached();

            if (detachedInputnum)
                GFE_Attribute::clone(*geoRefTmp, *inputnumAttrib, __TEMP_GFE_IntersectionStitch_inputnumAttribName);
            if (detachedPrimnum)
                GFE_Attribute::clone(*geoRefTmp, *primnumAttrib,  __TEMP_GFE_IntersectionStitch_primnumAttribName);
            if (detachedPrimuvw)
                GFE_Attribute::clone(*geoRefTmp, *primuvwAttrib,  __TEMP_GFE_IntersectionStitch_primuvwAttribName);
            
            interStitchParms.setInputnumattrib(detachedInputnum ? UT_StringHolder(__TEMP_GFE_IntersectionStitch_inputnumAttribName) : inputnumAttrib->getName());
            interStitchParms.setPrimnumattrib (detachedPrimnum  ? UT_StringHolder(__TEMP_GFE_IntersectionStitch_primnumAttribName)  : primnumAttrib->getName());
            interStitchParms.setPrimuvwattrib (detachedPrimuvw  ? UT_StringHolder(__TEMP_GFE_IntersectionStitch_primuvwAttribName)  : primuvwAttrib->getName());
        }
        
        geo->clear();
        GU_DetailHandle destgdh;
        destgdh.allocateAndSet(geo->asGU_Detail(), false);
        SOP_NodeCache* const nodeCache = interStitchVerb->allocCache();
        const auto interStitchCookparms = GFE_NodeVerb::newCookParms(cookparms, interStitchParms, nodeCache, &destgdh, &inputgdh);
        interStitchVerb->cook(interStitchCookparms);
    }




    void repair()
    {
        ptnumNewLast = geo->getNumPoints() - ptnumNewLast;

        const GA_PrimitiveGroupUPtr delGroupUPtr = geo->createDetachedPrimitiveGroup();
        GA_PrimitiveGroup* const delGroup = delGroupUPtr.get();
        
        UTparallelFor(geo->getPrimitiveSplittableRange(), [this, delGroup](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset primoff = start; primoff < end; ++primoff)
                {
                    const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                    if (numvtx <= 0)
                        continue;
                    
                    const GA_Offset primPtoff_last = geo->primPoint(primoff, numvtx-1);
                    const GA_Index  primPtnum_last = geo->pointIndex(primPtoff_last);
                    
                    if (primPtnum_last >= ptnumNewLast)
                        continue;
                    delGroup->setElement(primoff, true);
                }
            }
        }, subscribeRatio, minGrainSize);

        
        GA_Offset start, end;
        for (GA_Iterator it(geo->getPrimitiveRange(delGroup)); it.blockAdvance(start, end); )
        {
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                
                const GA_Offset primvtxoff0 = geo->getPrimitiveVertexOffset(primoff, 0);
                const GA_Offset primptoff0  = geo->vertexPoint(primvtxoff0);
                
                GA_Offset nextPrimoff;
                for (GA_Offset pointVtxoff = geo->pointVertex(primptoff0); GFE_Type::isValidOffset(pointVtxoff); pointVtxoff = geo->vertexToNextVertex(pointVtxoff))
                {
                    nextPrimoff = geo->vertexPrimitive(pointVtxoff);
                    if (geo->primitiveIndex(nextPrimoff) >= numprimInput)
                        break;
                }
                
                // const GA_Size numvtx_nextPrim = geo->getPrimitiveVertexCount(nextPrimoff);
                for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                {
                    const GA_Offset nextPrimptoff = geo->primPoint(primoff, vtxpnum);
                    //const GA_Offset newvtx = geo->appendVertex();
                    //geo->getTopology().wireVertexPrimitive(newvtx, nextPrimoff);
                    //geo->getTopology().wireVertexPoint(newvtx, nextPrimptoff);
                    //geo->getTopology().addVertex(newvtx);
                    //const GEO_PrimPoly* const gPrimPoly = geo->getGEOPrimPoly(nextPrimoff);
                    geo->stealVertex(nextPrimoff, geo->primVertex(primoff, vtxpnum));
                    //geo->getGEOPrimPoly(nextPrimoff)->appendVertex(nextPrimptoff);
                    //static_cast<GEO_PrimPoly*>(geo->asGEO_Detail()->getGEOPrimitive(nextPrimoff))->appendVertex(nextPrimptoff);
                }
                geo->destroyPrimitiveOffset(primoff);
            }
        }
    }
    
    
public:
    
    enum OutType
    {
        Point,
        FirstGeo,
        DualGeo,
    };
    
    bool splitCurve = false;
    bool repairResult = false;
    bool triangulateMesh = false;
    bool keepPointAttrib = true;
    
private:
    bool insertPoint = true;
    bool outSecondInputGeo = false;
    bool useTol = true;
    fpreal64 tolerance = 1e-05;
    
private:
    GU_Detail* geoTmp = nullptr;
    GU_Detail* geoRefTmp = nullptr;

private:
    GA_Index ptnumNewLast;
    GA_Size numprimInput;
    
    const GA_Attribute* enumAttrib0;
    const GA_Attribute* enumAttrib1;
    
    const GA_Attribute* inputnumAttrib = nullptr;
    const GA_Attribute* primnumAttrib  = nullptr;
    const GA_Attribute* primuvwAttrib  = nullptr;
    
    UT_Array<GU_ConstDetailHandle> inputgdh;
    SOP_IntersectionStitchParms interStitchParms;
    const SOP_NodeVerb* const interStitchVerb   = SOP_NodeVerb::lookupVerb("intersectionstitch");

    
    SOP_IntersectionAnalysisParms interAnalysisParms;
    const SOP_NodeVerb* const interAnalysisVerb = SOP_NodeVerb::lookupVerb("intersectionanalysis");
        
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;

#undef __TEMP_GFE_IntersectionStitch_InGroupName
#undef __TEMP_GFE_IntersectionStitch_RefGroupName
    
#undef __TEMP_GFE_IntersectionStitch_inputnumAttribName
#undef __TEMP_GFE_IntersectionStitch_primnumAttribName
#undef __TEMP_GFE_IntersectionStitch_primuvwAttribName
    
}; // End of class GFE_IntersectionStitch





#endif
