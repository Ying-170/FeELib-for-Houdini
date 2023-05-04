
#pragma once

#ifndef __GFE_SplitPoint_h__
#define __GFE_SplitPoint_h__



//#include "GFE/GFE_SplitPoint.h"

#include "GFE/GFE_GEOFilter.h"
//#include "GA/GA_Types.h"
#include "GA/GA_ElementWrangler.h"

#include "GFE/GFE_Type.h"



class GFE_SplitPoint : public GFE_AttribFilter {

private:
    bool splitByAttrib = false;


public:
    using GFE_AttribFilter::GFE_AttribFilter;

    ~GFE_SplitPoint()
    {
    }


    void
        setGroup(
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr
        )
    {
        groupParser.setGroup(geoPrimGroup);
    }

    void
        setGroup(
            const UT_StringHolder& primGroupName = ""
        )
    {
        groupParser.setPrimitiveGroup(primGroupName);
    }

    
    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }


    SYS_FORCE_INLINE void setSplitByAttrib(const GA_Attribute* const attribPtr = nullptr)
    {
        splitAttribPtr = attribPtr;
    }

    SYS_FORCE_INLINE void setSplitByAttrib(const GA_AttributeOwner owner, const UT_StringRef& name)
    {
        splitAttribPtr = geo->findAttribute(owner, name);
    }



private:

    // can not use in parallel unless for each GA_Detail
    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;


        return true;
    }


    GA_Size GEOsplitPoints(
        GEO_Detail* geo,
        const GA_ElementGroup* group
    )
    {
        if (group && group->isEmpty())
            return 0;

        UT_AutoInterrupt boss("Making Unique Points");
        if (boss.wasInterrupted())
            return 0;

        // Behaviour is as if it's a point group if no group is present.
        GA_GroupType grouptype = group ? group->classType() : GA_GROUP_POINT;
        const GA_PointGroup* point_group = nullptr;
        GA_PointGroupUPtr point_group_deleter;
        if (group)
        {
            if (grouptype == GA_GROUP_POINT)
                point_group = static_cast<const GA_PointGroup*>(group);
            else if (grouptype == GA_GROUP_VERTEX || grouptype == GA_GROUP_PRIMITIVE)
            {
                GA_PointGroup* ptgroup = new GA_PointGroup(*geo);
                point_group_deleter.reset(ptgroup);
                ptgroup->combine(group);
                point_group = ptgroup;
            }
        }

        GA_Size numpointsadded = 0;

        char bcnt = 0;

        UT_UniquePtr<GA_PointWrangler> ptwrangler(nullptr);

        UT_SmallArray<GA_Offset> vtxoffs;
        GA_Offset points_end = geo->getPointMap().lastOffset() + 1;
        GA_Offset start, end;
        for (GA_Iterator it(geo->getPointRange(point_group)); it.blockAdvance(start, end); )
        {
            // Prevent going beyond the original points; new points don't need to be split.
            if (start >= points_end)
                break;
            if (end > points_end)
                end = points_end;

            for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
            {
                vtxoffs.setSize(0);

                // Make vertex order consistent, regardless of linked list order,
                // by sorting in vertex offset order
                bool has_other = false;
                if (grouptype == GA_GROUP_POINT)
                {
                    for (GA_Offset vtxoff = geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = geo->vertexToNextVertex(vtxoff))
                        vtxoffs.append(vtxoff);
                }
                else
                {
                    for (GA_Offset vtxoff = geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = geo->vertexToNextVertex(vtxoff))
                    {
                        GA_Offset group_offset = vtxoff;
                        if (grouptype == GA_GROUP_PRIMITIVE)
                            group_offset = geo->vertexPrimitive(vtxoff);
                        if (group->contains(group_offset))
                            vtxoffs.append(vtxoff);
                        else
                            has_other = true;
                    }
                }

                // If there's only one vertex in the group and none outside, nothing to split.
                // If there are no vertices in the group, also nothing to split.
                // Unique all but first vertex if all vertices of the point are in vtxoffs.
                GA_Size nvtx = vtxoffs.size();
                GA_Size points_to_add = nvtx - GA_Size(!has_other);
                if (points_to_add <= 0)
                    continue;

                numpointsadded += points_to_add;

                if (!ptwrangler)
                    ptwrangler.reset(new GA_PointWrangler(*geo, GA_AttributeFilter::selectPublic()));

                // TODO: This is not perfect; we should be sorting by primitive order
                //       and then vertex order within a primitive, so that results
                //       are the same whether the input was loaded from a file or
                //       computed.
                vtxoffs.stdsort(std::less<GA_Offset>());

                bool skip_first = !has_other;
                GA_Offset newptoff = geo->appendPointBlock(points_to_add);
                for (exint j = exint(skip_first); j < nvtx; ++j, ++newptoff)
                {
                    if (!bcnt++ && boss.wasInterrupted())
                        return numpointsadded;

                    ptwrangler->copyAttributeValues(newptoff, ptoff);
                    geo->setVertexPoint(vtxoffs(j), newptoff);
                }
            }
        }
        if (numpointsadded > 0)
        {
            // If we'd only added/removed points, we could use
            // geo->bumpDataIdsForAddOrRemove(true, false, false),
            // but we also rewired vertices, so we need to bump the
            // linked-list topology attributes.
            geo->getAttributes().bumpAllDataIds(GA_ATTRIB_POINT);
            GA_ATITopology* topo = geo->getTopology().getPointRef();
            if (topo)
                topo->bumpDataId();
            topo = geo->getTopology().getVertexNextRef();
            if (topo)
                topo->bumpDataId();
            topo = geo->getTopology().getVertexPrevRef();
            if (topo)
                topo->bumpDataId();
            // Edge groups might also be affected, if any edges
            // were on points that were split, so we bump their
            // data IDs, just in case.
            geo->edgeGroups().bumpAllDataIds();
        }
        return numpointsadded;
    }

    GA_Size GEOsplitPointsByAttrib(
            GEO_Detail* geo,
            const GA_ElementGroup* group,
            const GA_Attribute* attrib,
            fpreal tolerance)
    {
        bool isptgroup = (group == nullptr || group->getOwner() == GA_ATTRIB_POINT);
        return geoSplitPointsByAttrib(
            geo,
            geo->getPointRange(isptgroup ? UTverify_cast<const GA_PointGroup*>(group) : nullptr),
            isptgroup ? nullptr : group,
            attrib,
            tolerance);
    }

    GA_Size GEOsplitPointsByAttrib(
            GEO_Detail* geo,
            const GA_Range& points,
            const GA_Attribute* attrib,
            fpreal tolerance)
    {
        return geoSplitPointsByAttrib(
            geo,
            points,
            nullptr,
            attrib,
            tolerance);
    }

private:

    template<typename T>
    bool compareVertices(
            GA_Offset v1,
            GA_Offset v2,
            const GA_ROHandleT<T>& attrib,
            int tuplesize,
            fpreal tol)
    {
        for (int i = 0; i < tuplesize; i++)
        {
            if (SYSabs(attrib.get(v1, i) - attrib.get(v2, i)) > tol)
                return false;
        }
        return true;
    }

    GA_Size geoSplitPointsByAttrib(
            GEO_Detail* geo,
            const GA_Range& points,
            const GA_ElementGroup* group,
            const GA_Attribute* attrib,
            fpreal tolerance
        )
    {
        // Point groups should be weeded out by the wrappers below.
        UT_ASSERT(!group || group->classType() == GA_GROUP_PRIMITIVE || group->classType() == GA_GROUP_VERTEX);
        bool is_primgroup = group && group->classType() == GA_GROUP_PRIMITIVE;

        GA_Size numpointsadded = 0;

        UT_ASSERT(attrib);
        if (!attrib)
            return numpointsadded;

        GA_AttributeOwner owner = attrib->getOwner();
        if (owner == GA_ATTRIB_POINT || owner == GA_ATTRIB_DETAIL)
            return numpointsadded;

        const GA_AIFCompare* compare = NULL;
        GA_ROHandleT<int64> attribi;
        GA_ROHandleR attribf;
        int tuplesize;
        const GA_ATINumeric* numeric = dynamic_cast<const GA_ATINumeric*>(attrib);
        if (numeric)
        {
            GA_StorageClass storeclass = attrib->getStorageClass();
            if (storeclass == GA_STORECLASS_INT)
            {
                attribi = attrib;
                UT_ASSERT(attribi.isValid());
                tuplesize = attribi.getTupleSize();
            }
            else if (storeclass == GA_STORECLASS_FLOAT)
            {
                attribf = attrib;
                UT_ASSERT(attribf.isValid());
                tuplesize = attribf.getTupleSize();
            }
            else
            {
                UT_ASSERT_MSG(0, "Why does a GA_ATINumeric have a storage class other than int or float?");
                compare = attrib->getAIFCompare();
            }
        }
        else
        {
            compare = attrib->getAIFCompare();
            if (compare == NULL)
            {
                UT_ASSERT_MSG(0, "Missing an implementation of GA_AIFCompare!");
                return numpointsadded;
            }
        }

        UT_UniquePtr<GA_PointWrangler> ptwrangler(nullptr);

        UT_SmallArray<GA_Offset> vtxlist;
        GA_Size initnumpts = geo->getNumPoints();
        for (GA_Iterator it(points); !it.atEnd(); ++it)
        {
            GA_Offset ptoff = *it;
            GA_Index ptidx = geo->pointIndex(ptoff);
            if (ptidx >= initnumpts)
                break; // New points are already split

            bool splitfound;
            do
            {
                GA_Offset vtxoff = geo->pointVertex(ptoff);
                if (!GAisValid(vtxoff))
                    break; // Point doesn't belong to anyone

                // To avoid the results being dependent on the order of the
                // linked-list topology attribute, find the highest vertex offset
                // in the primitive of highest offset to be the base vertex.
                // Lowest would make more sense, but highest is more compatible with
                // the previous code.
                GA_Offset primoff = geo->vertexPrimitive(vtxoff);
                GA_Offset basevtxoff = GA_INVALID_OFFSET;
                GA_Offset baseprimoff = GA_INVALID_OFFSET;
                if (!group || group->contains(is_primgroup ? primoff : vtxoff))
                {
                    basevtxoff = vtxoff;
                    baseprimoff = primoff;
                }
                vtxoff = geo->vertexToNextVertex(vtxoff);
                while (GAisValid(vtxoff))
                {
                    primoff = geo->vertexPrimitive(vtxoff);

                    if (!group || group->contains(is_primgroup ? primoff : vtxoff))
                    {
                        if (primoff > baseprimoff || (primoff == baseprimoff && vtxoff > basevtxoff))
                        {
                            basevtxoff = vtxoff;
                            baseprimoff = primoff;
                        }
                    }
                    vtxoff = geo->vertexToNextVertex(vtxoff);
                }

                if (!GAisValid(basevtxoff))
                    break; // No vertices in group on current point

                // We have a vertex on the point that's in the group.
                // If any vertices mismatch, we make a new point.

                GA_Offset baseoffset = (owner == GA_ATTRIB_VERTEX) ? basevtxoff : baseprimoff;
                vtxlist.setSize(1);
                vtxlist(0) = basevtxoff;
                splitfound = false;
                for (vtxoff = geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = geo->vertexToNextVertex(vtxoff))
                {
                    if (vtxoff == basevtxoff)
                        continue;
                    GA_Offset offset = (owner == GA_ATTRIB_VERTEX) ? vtxoff : geo->vertexPrimitive(vtxoff);
                    bool match;
                    if (attribi.isValid())
                        match = compareVertices<int64>(baseoffset, offset, attribi, tuplesize, tolerance);
                    else if (attribf.isValid())
                        match = compareVertices<fpreal>(baseoffset, offset, attribf, tuplesize, tolerance);
                    else
                    {
                        bool success = compare->isEqual(match, *attrib, baseoffset, *attrib, offset);
                        if (!success)
                            match = true;
                    }
                    if (match)
                    {
                        // Only append vertices in the group
                        if (!group || group->contains(is_primgroup ? geo->vertexPrimitive(vtxoff) : vtxoff))
                        {
                            vtxlist.append(vtxoff);
                        }
                    }
                    else
                    {
                        // Split regardless of whether the unmatched vertex is in the group.
                        // This may or may not be the behaviour people expect,
                        // but it's ambiguous as to what people would expect.
                        // At least, if all vertices in the group have one value
                        // and all vertices out of the group have a different value,
                        // they probably want a new point, and that's accomplished
                        // with this condition.
                        splitfound = true;
                    }
                }

                if (splitfound)
                {
                    // A split has been found! Create a new point
                    // using all the entries of vtxlist.
                    GA_Offset newptoff = geo->appendPointOffset();
                    ++numpointsadded;
                    if (!ptwrangler)
                        ptwrangler.reset(new GA_PointWrangler(*geo, GA_AttributeFilter::selectPublic()));
                    ptwrangler->copyAttributeValues(newptoff, ptoff);
                    for (exint i = 0; i < vtxlist.size(); i++)
                        geo->setVertexPoint(vtxlist(i), newptoff);
                }
            } while (splitfound);
        }

        if (numpointsadded > 0)
        {
            // If we'd only added/removed points, we could use
            // geo->bumpDataIdsForAddOrRemove(true, false, false),
            // but we also rewired vertices, so we need to bump the
            // linked-list topology attributes.
            geo->getAttributes().bumpAllDataIds(GA_ATTRIB_POINT);
            GA_ATITopology* topo = geo->getTopology().getPointRef();
            if (topo)
                topo->bumpDataId();
            topo = geo->getTopology().getVertexNextRef();
            if (topo)
                topo->bumpDataId();
            topo = geo->getTopology().getVertexPrevRef();
            if (topo)
                topo->bumpDataId();
            // Edge groups might also be affected, if any edges
            // were on points that were split, so we bump their
            // data IDs, just in case.
            geo->edgeGroups().bumpAllDataIds();
        }
        return numpointsadded;
    }

public:
    const GA_Attribute* splitAttribPtr = nullptr;
    fpreal splitAttribTol = 1e-05;

private:

    exint subscribeRatio = 64;
    exint minGrainSize = 64;

}; // End of class GFE_SplitPoint

#endif
