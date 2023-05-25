
#pragma once

#ifndef __GFE_DeleteVertex_h__
#define __GFE_DeleteVertex_h__

//#include "GFE/GFE_DelVertex.h"

#include "GFE/GFE_GeoFilter.h"



class GFE_DelVertex : public GFE_GeoFilter {


public:
	using GFE_GeoFilter::GFE_GeoFilter;


    void
        setComputeParm(
            const bool delDegeneratePrims = false,
            const bool delUnusedPoints = true,
            const bool reverseGroup = false,
			const bool delGroup = false
        )
    {
        setHasComputed();
        this->delDegeneratePrims = delDegeneratePrims;
        this->delUnusedPoints = delUnusedPoints;
        this->reverseGroup = reverseGroup;
        this->delGroup = delGroup;
    }






private:

    virtual bool
        computeCore() override
    {
        if (!reverseGroup && groupParser.isEmpty())
            return true;

		switch (groupParser.classType())
		{
		case GA_GROUP_PRIMITIVE: delVertex(groupParser.getPrimitiveGroup()); break;
		case GA_GROUP_POINT:     delVertex(groupParser.getPointGroup());     break;
		case GA_GROUP_VERTEX:    delVertex(groupParser.getVertexGroup());    break;
		case GA_GROUP_EDGE:      delVertex(groupParser.getEdgeGroup());      break;
		default:                 delVertex();                                break;
		}

    	if (delGroup)
    		groupParser.delGroup();
    	
        return true;
    }


	void delVertex()
	{
		if (reverseGroup)
			return;
    	
		if (delDegeneratePrims)
			geo->destroyPrimitives(geo->getPrimitiveRange(), delUnusedPoints);
		else
		{
			GA_Topology& topo = geo->getTopology();
			GA_Offset start, end;
			for (GA_Iterator it(geo->getPrimitiveRange()); it.blockAdvance(start, end); )
			{
				for (GA_Offset primoff = start; primoff < end; ++primoff)
				{
					const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
					for (GA_Size i = 0; i < vertices.size(); i++)
					{
						geo->getPrimitive(primoff)->releaseVertex(vertices[i]);
						topo.delVertex(vertices[i]);
                        //geo->destroyVertexOffset(vtxoff);
					}
				}
			}
			if (delUnusedPoints)
				geo->destroyUnusedPoints();
		}
	}


	void delVertex(const GA_PrimitiveGroup* const group)
	{
    	if (!group)
			return delVertex();

		if (delDegeneratePrims)
			geo->destroyPrimitives(geo->getPrimitiveRange(group, reverseGroup), delUnusedPoints);
		else
		{
			GA_PointGroupUPtr pointGroupUPtr;
			GA_PointGroup* pointGroup;
			if (delUnusedPoints)
			{
				pointGroupUPtr = geo->createDetachedPointGroup();
				pointGroup = pointGroupUPtr.get();
			}

			GA_Topology& topo = geo->getTopology();
			const GA_ATITopology* const vtxPointRef = topo.getPointRef();

			GA_Offset start, end;
			for (GA_Iterator it(geo->getPrimitiveRange(group, reverseGroup)); it.blockAdvance(start, end); )
			{
				for (GA_Offset primoff = start; primoff < end; ++primoff)
				{
					const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
					for (GA_Size i = 0; i < vertices.size(); i++)
					{
						if (delUnusedPoints)
							pointGroup->setElement(vtxPointRef->getLink(vertices[i]), true);
						geo->getPrimitive(primoff)->releaseVertex(vertices[i]);
						topo.delVertex(vertices[i]);
					}
				}
			}
			if (delUnusedPoints)
				geo->destroyUnusedPoints(pointGroup);
		}
	}


	void delVertex(const GA_PointGroup* const group)
	{
		if (!group)
			return delVertex();

		if (delUnusedPoints)
		{
			geo->destroyPointOffsets(geo->getPointRange(group, reverseGroup),
				delDegeneratePrims ? GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE : GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES);
		}
		else
		{
			GA_PrimitiveGroupUPtr primGroupUPtr;
			GA_PrimitiveGroup* primGroup;
			if (delDegeneratePrims)
			{
				primGroupUPtr = geo->createDetachedPrimitiveGroup();
				primGroup = primGroupUPtr.get();
			}

			GA_Topology& topo = geo->getTopology();
			const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
			const GA_ATITopology* const vtxPrimRef = topo.getPrimitiveRef();
			const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

			GA_Offset start, end;
			for (GA_Iterator it(geo->getPointRange(group, reverseGroup)); it.blockAdvance(start, end); )
			{
				for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
				{
					for (GA_Offset vtxoff_next = pointVtxRef->getLink(ptoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
					{
						GA_Offset primoff = vtxPrimRef->getLink(vtxoff_next);

						geo->getPrimitive(primoff)->releaseVertex(vtxoff_next);
						topo.delVertex(vtxoff_next);

						if (delDegeneratePrims)
							primGroup->setElement(primoff, true);
					}
				}
			}
			if (delDegeneratePrims)
				geo->destroyPrimitives(geo->getPrimitiveRange(primGroup));
		}
	}





	void delVertex(const GA_VertexGroup* const group)
	{
		if (!group)
			return delVertex();

		GA_Topology& topo = geo->getTopology();
		const GA_ATITopology* const vtxPrimRef = topo.getPrimitiveRef();
		const GA_ATITopology* const vtxPointRef = topo.getPointRef();


		GA_PrimitiveGroupUPtr primGroupUPtr;
		GA_PrimitiveGroup* primGroup;
		if (delDegeneratePrims)
		{
			primGroupUPtr = geo->createDetachedPrimitiveGroup();
			primGroup = primGroupUPtr.get();
		}

		GA_PointGroupUPtr pointGroupUPtr;
		GA_PointGroup* pointGroup;
		if (delUnusedPoints)
		{
			pointGroupUPtr = geo->createDetachedPointGroup();
			pointGroup = pointGroupUPtr.get();
		}
#if 1
		GA_Offset start, end;
		for (GA_Iterator it(geo->getVertexRange(group, reverseGroup)); it.blockAdvance(start, end); )
		{
			for (GA_Offset vtxoff = start; vtxoff < end; ++vtxoff)
			{
				GA_Offset primoff = vtxPrimRef->getLink(vtxoff);

				geo->getPrimitive(primoff)->releaseVertex(vtxoff);
				topo.delVertex(vtxoff);
                //geo->destroyVertexOffset(vtxoff);

				if (delDegeneratePrims)
					primGroup->setElement(primoff, true);
				if (delUnusedPoints)
					pointGroup->setElement(vtxPointRef->getLink(vtxoff), true);
			}
		}
#else
		const GA_SplittableRange geoSplittableRange(geo->getVertexRange(group, reverseGroup));
		UTparallelFor(geoSplittableRange, [geo, &topo, vtxPrimRef, vtxPointRef, primGroup, pointGroup, delDegeneratePrims, delUnusedPoints](const GA_SplittableRange& r)
			{
				GA_Offset start, end;
				for (GA_Iterator it(r); it.blockAdvance(start, end); )
				{
					for (GA_Offset vtxoff = start; vtxoff < end; ++vtxoff)
					{
						GA_Offset primoff = vtxPrimRef->getLink(vtxoff);

						geo->getPrimitive(primoff)->releaseVertex(vtxoff);
						topo.delVertex(vtxoff);

						if (delDegeneratePrims)
							primGroup->setElement(primoff, true);
						if (delUnusedPoints)
							pointGroup->setElement(vtxPointRef->getLink(vtxoff), true);
					}
				}
			});
#endif

		if (delDegeneratePrims)
			geo->destroyDegeneratePrimitives(primGroup);
		if (delUnusedPoints)
			geo->destroyUnusedPoints(pointGroup);
	}


	void delVertex(const GA_EdgeGroup* const group)
	{
		if (!group)
			return delVertex();

		const GA_VertexGroupUPtr vtxGroupUPtr = geo->createDetachedVertexGroup();
		GA_VertexGroup* vtxGroup = vtxGroupUPtr.get();
		vtxGroup->combine(group);
		delVertex(vtxGroup);
	}


public:
    bool delDegeneratePrims = true;
    bool delUnusedPoints = true;
	
    bool reverseGroup = false;
    bool delGroup = true;
	
private:

    //exint subscribeRatio = 64;
    //exint minGrainSize = 1024;


}; // End of class GFE_DelVertex







#endif
