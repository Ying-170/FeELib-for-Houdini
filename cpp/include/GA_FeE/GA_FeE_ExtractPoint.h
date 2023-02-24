
#pragma once

#ifndef __GA_FeE_ExtractPoint_h__
#define __GA_FeE_ExtractPoint_h__


//#include "GA_FeE/GA_FeE_ExtractPoint.h"

#include "GA/GA_Detail.h"

#include "GA_FeE/GA_FeE_Type.h"
#include "GA_FeE/GA_FeE_Detail.h"
#include "GA_FeE/GA_FeE_Attribute.h"
#include "GA_FeE/GA_FeE_Group.h"




namespace GA_FeE_ExtractPoint {






//extractPoint(geo, groupName, reverseGroup, delGroup);
static void
extractPoint(
    GA_Detail* const geo,
    GA_PointGroup*& group,
    const bool reverseGroup = false,
    const bool delInputGroup = true
)
{
    UT_VERIFY_P(geo);
    if (group)
    {
        if (group->isEmpty())
        {
            if (reverseGroup)
            {
                return;
            }
            else
            {
                GA_FeE_Detail::clearElement(geo);
                return;
            }
        }
        else if (group->entries() == geo->getNumPoints())
        {
            if (reverseGroup)
            {
                GA_FeE_Detail::clearElement(geo);
                return;
            }
            else
            {
                return;
            }
        }
    }
    else
    {
        if (reverseGroup)
        {
            return;
        }
        else
        {
            GA_FeE_Detail::clearElement(geo);
            return;
        }
    }

    constexpr int kernel = 2;
    switch (kernel)
    {
    case 0:
    {
        if (reverseGroup)
        {
            if (group->entries() <= 4096)
            {
                geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
            }
            else
            {
            }
        }
        else
        {
            if (group->entries() >= geo->getNumPoints() - 4096)
            {
                geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
            }
            else
            {
                //geo->merge();
            }
        }
    }
    break;
    case 1:
    {
        GA_OffsetList offList = GA_FeE_Detail::getOffsetList(geo, group, !reverseGroup);
        GA_IndexMap& ptmap = geo->getIndexMap(GA_ATTRIB_POINT);

        for (GA_Size arrayi = 0; arrayi < offList.size(); ++arrayi)
        {
            ptmap.destroyOffset(offList[arrayi]);
        }
    }
    break;
    case 2:
        geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
        break;
    default:
        break;
    }
    //geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
    //geo->destroyUnusedPoints(group);
    if (delInputGroup && group && !group->isDetached())
    {
        geo->destroyGroup(group);
        group = nullptr;
    }
}

//extractPoint(geo, groupName, reverseGroup, delGroup);
static void
extractPoint(
    GA_Detail* const geo,
    const GA_Detail* const srcGeo,
    const GA_PointGroup* const group,
    const bool reverseGroup = false,
    const bool delInputGroup = true
)
{
    UT_VERIFY_P(geo);
    UT_VERIFY_P(srcGeo);
    if (group)
    {
        if (group->isEmpty())
        {
            if (reverseGroup)
            {
                return;
            }
            else
            {
                GA_FeE_Detail::clearElement(geo);
                return;
            }
        }
        else if (group->entries() == geo->getNumPoints())
        {
            if (reverseGroup)
            {
                GA_FeE_Detail::clearElement(geo);
                return;
            }
            else
            {
                return;
            }
        }
    }
    else
    {
        if (reverseGroup)
        {
            return;
        }
        else
        {
            GA_FeE_Detail::clearElement(geo);
            return;
        }
    }

    constexpr int kernel = 3;
    switch (kernel)
    {
    case 0:
    {
        if (reverseGroup)
        {
            if (group->entries() <= 4096)
            {
                geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
            }
            else
            {
                GA_FeE_Detail::clearElement(geo);
                static_cast<GEO_Detail*>(geo)->mergePoints(*static_cast<const GEO_Detail*>(srcGeo), group, false, false);
            }
        }
        else
        {
            if (group->entries() >= geo->getNumPoints() - 4096)
            {
                geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
            }
            else
            {
                GA_FeE_Detail::clearElement(geo);
                static_cast<GEO_Detail*>(geo)->mergePoints(*static_cast<const GEO_Detail*>(srcGeo), group, false, false);
            }
        }
    }
    break;
    case 1:
    {
        GA_OffsetList offList = GA_FeE_Detail::getOffsetList(geo, group, !reverseGroup);
        GA_IndexMap& ptmap = geo->getIndexMap(GA_ATTRIB_POINT);

        for (GA_Size arrayi = 0; arrayi < offList.size(); ++arrayi)
        {
            ptmap.destroyOffset(offList[arrayi]);
        }
    }
    break;
    case 2:
        geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
        break;
    case 3:
        //GA_FeE_Detail::clearElement(geo);
        geo->clear();
        //geo->mergePoints(*srcGeo, group, false, false);
        static_cast<GEO_Detail*>(geo)->mergePoints(*static_cast<const GEO_Detail*>(srcGeo), GA_Range(srcGeo->getPointMap(), group, reverseGroup));
        break;
    default:
        break;
    }
    //geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
    //geo->destroyUnusedPoints(group);
    if (delInputGroup)
    {
        GA_PointGroup* const inputGroup = geo->findPointGroup(group->getName());
        if (inputGroup)
            geo->destroyGroup(inputGroup);
    }
}



//extractPoint(geo, groupName, reverseGroup, delGroup);
static void
extractPoint(
    GA_Detail* const geo,
    GA_PointGroup*& group,
    const UT_StringHolder& delPrimGroup,
    const UT_StringHolder& delPointGroup,
    const UT_StringHolder& delVertexGroup,
    const UT_StringHolder& delEdgeGroup,
    const bool reverseGroup = false,
    const bool delInputGroup = true
)
{
    UT_VERIFY_P(geo);
    if (group)
    {
        GA_FeE_Group::delStdGroup(geo, delPrimGroup, "", delVertexGroup, delEdgeGroup);
        GA_GroupTable* groupTable = geo->getGroupTable(GA_GROUP_POINT);
        for (GA_GroupTable::iterator<GA_Group> it = groupTable->beginTraverse(); !it.atEnd(); ++it)
        {
            GA_PointGroup* const geoGroup = static_cast<GA_PointGroup*>(it.group());
            if (geoGroup->isDetached())
                continue;
            //if (geoGroup->getName() == group->getName())
            if (geoGroup == group)
                continue;
            if (geoGroup->getName().match(delPointGroup))
                continue;
            groupTable->destroy(geoGroup);
        }
    }
    else
    {
        GA_FeE_Group::delStdGroup(geo, delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup);
    }

    extractPoint(geo, group,
        reverseGroup, delInputGroup);
}


//GA_FeE_Detail::extractPoint(cookparms, outGeo0, inGeo0, groupName,
//    delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup,
//    sopparms.getReverseGroup(), sopparms.getDelInputGroup());
static void
extractPoint(
    const SOP_NodeVerb::CookParms& cookparms,
    GA_Detail* const geo,
    const UT_StringHolder& groupName,
    const UT_StringHolder& delPrimGroup,
    const UT_StringHolder& delPointGroup,
    const UT_StringHolder& delVertexGroup,
    const UT_StringHolder& delEdgeGroup,
    const bool reverseGroup = false,
    const bool delInputGroup = true
)
{
    UT_VERIFY_P(geo);

    GOP_Manager gop;
    GA_PointGroup* group = GA_FeE_Group::findOrParsePointGroupDetached(cookparms, geo, groupName, gop);
    extractPoint(geo, group,
        delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup,
        reverseGroup, delInputGroup);
}




//extractPoint(geo, groupName, reverseGroup, delGroup);
SYS_FORCE_INLINE
static void
extractPoint(
    GA_Detail* const geo,
    const GA_Detail* const srcGeo,
    const GA_PointGroup* const group,
    const UT_StringHolder& delPrimAttrib,
    const UT_StringHolder& delPointAttrib,
    const UT_StringHolder& delVertexAttrib,
    const UT_StringHolder& delDetailAttrib,
    const UT_StringHolder& delPrimGroup,
    const UT_StringHolder& delPointGroup,
    const UT_StringHolder& delVertexGroup,
    const UT_StringHolder& delEdgeGroup,
    const bool reverseGroup = false,
    const bool delInputGroup = true
)
{
    UT_VERIFY_P(geo);
    UT_VERIFY_P(srcGeo);
    geo->replaceWithPoints(*srcGeo);

    GA_FeE_Attribute::delStdAttribute(geo, delPrimAttrib, delPointAttrib, delVertexAttrib, delDetailAttrib);
    GA_FeE_Group::delStdGroup(geo, delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup);

    extractPoint(geo, srcGeo, group,
        reverseGroup, delInputGroup);

    GA_FeE_Group::delStdGroup(geo, GA_GROUP_POINT, delPointGroup);
}

//extractPoint(geo, groupName, reverseGroup, delGroup);
SYS_FORCE_INLINE
static void
extractPoint(
    GA_Detail* const geo,
    const GA_PointGroup* const group,
    const UT_StringHolder& delPrimAttrib,
    const UT_StringHolder& delPointAttrib,
    const UT_StringHolder& delVertexAttrib,
    const UT_StringHolder& delDetailAttrib,
    const UT_StringHolder& delPrimGroup,
    const UT_StringHolder& delPointGroup,
    const UT_StringHolder& delVertexGroup,
    const UT_StringHolder& delEdgeGroup,
    const bool reverseGroup = false,
    const bool delInputGroup = true
)
{
    UT_VERIFY_P(geo);
    geo->replaceWithPoints(*geo);

    GA_FeE_Attribute::delStdAttribute(geo, delPrimAttrib, delPointAttrib, delVertexAttrib, delDetailAttrib);
    GA_FeE_Group::delStdGroup(geo, delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup);

    extractPoint(geo, geo, group,
        reverseGroup, delInputGroup);

    GA_FeE_Group::delStdGroup(geo, GA_GROUP_POINT, delPointGroup);
}


//GA_FeE_Detail::extractPoint(cookparms, outGeo0, inGeo0, groupName,
//    delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup,
//    sopparms.getReverseGroup(), sopparms.getDelInputGroup());
SYS_FORCE_INLINE
static void
extractPoint(
    const SOP_NodeVerb::CookParms& cookparms,
    GA_Detail* const geo,
    const GA_Detail* const srcGeo,
    const UT_StringHolder& groupName,
    const UT_StringHolder& delPrimAttrib,
    const UT_StringHolder& delPointAttrib,
    const UT_StringHolder& delVertexAttrib,
    const UT_StringHolder& delDetailAttrib,
    const UT_StringHolder& delPrimGroup,
    const UT_StringHolder& delPointGroup,
    const UT_StringHolder& delVertexGroup,
    const UT_StringHolder& delEdgeGroup,
    const bool reverseGroup = false,
    const bool delInputGroup = true
)
{
    UT_VERIFY_P(geo);

    GOP_Manager gop;
    const GA_PointGroup* const group = GA_FeE_Group::findOrParsePointGroupDetached(cookparms, srcGeo, groupName, gop);
    extractPoint(geo, srcGeo, group,
        delPrimAttrib, delPointAttrib, delVertexAttrib, delDetailAttrib,
        delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup,
        reverseGroup, delInputGroup);

    geo->bumpDataIdsForAddOrRemove(1, 0, 0);
}


//GA_FeE_ExtractPoint::extractPoint(cookparms, outGeo0, groupName,
//    delPrimAttrib, delPointAttrib, delVertexAttrib, delDetailAttrib,
//    delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup,
//    sopparms.getReverseGroup(), sopparms.getDelInputGroup());
SYS_FORCE_INLINE
static void
extractPoint(
    const SOP_NodeVerb::CookParms& cookparms,
    GA_Detail* const geo,
    const UT_StringHolder& groupName,
    const UT_StringHolder& delPrimAttrib,
    const UT_StringHolder& delPointAttrib,
    const UT_StringHolder& delVertexAttrib,
    const UT_StringHolder& delDetailAttrib,
    const UT_StringHolder& delPrimGroup,
    const UT_StringHolder& delPointGroup,
    const UT_StringHolder& delVertexGroup,
    const UT_StringHolder& delEdgeGroup,
    const bool reverseGroup = false,
    const bool delInputGroup = true
)
{
    UT_VERIFY_P(geo);

    GOP_Manager gop;
    const GA_PointGroup* const group = GA_FeE_Group::findOrParsePointGroupDetached(cookparms, geo, groupName, gop);
    extractPoint(geo, group,
        delPrimAttrib, delPointAttrib, delVertexAttrib, delDetailAttrib,
        delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup,
        reverseGroup, delInputGroup);

    geo->bumpDataIdsForAddOrRemove(1, 0, 0);
}





} // End of namespace GA_FeE_ExtractPoint

#endif
