
#pragma once

#ifndef __GFE_Connectivity_h__
#define __GFE_Connectivity_h__

//#include "GFE/GFE_Connectivity.h"

#include "GA/GA_Detail.h"


#include "GFE/GFE_GeoFilter.h"

#include "GFE/GFE_Adjacency.h"
#include "GFE/GFE_AttributePromote.h"
#include "GFE/GFE_AttributeCast.h"




class GFE_Connectivity : public GFE_AttribCreateFilter {

public:
    GFE_Connectivity(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribCreateFilter(geo, cookparms)
        , groupParserSeam(geo, groupParser.getGOP(), cookparms)
    {
    }

    GFE_Connectivity(
        const SOP_NodeVerb::CookParms& cookparms,
        GA_Detail* const geo
    )
        : GFE_AttribCreateFilter(cookparms, geo)
        , groupParserSeam(cookparms, geo, groupParser.getGOP())
    {
    }

    void
        setComputeParm(
            const bool connectivityConstraint = false,
            const bool outTopoAttrib = false
            //,const exint subscribeRatio = 64,
            //const exint minGrainSize = 1024
        )
    {
        setHasComputed();

        this->connectivityConstraint = connectivityConstraint;

        setOutTopoAttrib(outTopoAttrib);
        //this->subscribeRatio = subscribeRatio;
        //this->minGrainSize = minGrainSize;
    }

    void
        findOrCreateTuple(
            const GA_AttributeOwner attribOwner = GA_ATTRIB_PRIMITIVE,
            const GA_StorageClass storageClass = GA_STORECLASS_INT,
            const GA_Storage storage = GA_STORE_INVALID,
            const bool detached = false,
            const UT_StringHolder& name = "__topo_connectivity"
        )
    {
        getOutAttribArray().clear();

        const GA_AttributeOwner connectivityOwner = connectivityConstraint ? GA_ATTRIB_PRIMITIVE : GA_ATTRIB_POINT;
        const bool isDetached = connectivityOwner != attribOwner || storageClass != GA_STORECLASS_INT;
#if 0
        getOutAttribArray().findOrCreateTuple(connectivityOwner, GA_STORECLASS_INT, storage, isDetached || detached, name);
        if (isDetached)
            getOutAttribArray().findOrCreateTuple(attribOwner, storageClass, storage, detached, name);
#else
        getOutAttribArray().findOrCreateTuple(false, connectivityOwner, GA_STORECLASS_INT, storage, isDetached ? TEMP_ConnectivityAttribName : name);
        if (isDetached)
            getOutAttribArray().findOrCreateTuple(detached, attribOwner, storageClass, storage, name);
#endif
    }








private:


    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        if (getOutAttribArray().isEmpty())
        {
            return true;
        }

        if (!getInAttribArray().isEmpty())
        {
            //return false;
        }

        connectivity();

        if (getOutAttribArray().size() > 1)
        {
            GA_Attribute* const inAttribPtr = getOutAttribArray()[0];
            GA_Attribute* const outAttribPtr = getOutAttribArray()[1];
            GA_Attribute* finalAttribPtr = nullptr;
            const GA_AttributeOwner outAttribOwner = outAttribPtr->getOwner();
            const GA_AttributeOwner connectivityOwner = connectivityConstraint ? GA_ATTRIB_PRIMITIVE : GA_ATTRIB_POINT;
            //const bool isDetached = connectivityOwner != outAttrib->get || storageClass != GA_STORECLASS_INT;

            const GA_StorageClass outAttribStorageClass = outAttribPtr->getStorageClass();
            if (outAttribOwner != connectivityOwner)
            {
                finalAttribPtr = GFE_AttributePromote::attribPromote(geo, inAttribPtr, outAttribOwner);
                geo->getAttributes().destroyAttribute(inAttribPtr);
            }
            if (outAttribStorageClass != GA_STORECLASS_INT)
            {
                GFE_AttributeCast::changeAttribStorageClass(geo, outAttribOwner, TEMP_ConnectivityAttribName, outAttribStorageClass);
            }
            geo->getAttributes().destroyAttribute(outAttribPtr);
            GFE_Attribute::forceRenameAttribute(geo, finalAttribPtr, outAttribPtr->getName());
            getOutAttribArray()[0] = finalAttribPtr;
            getOutAttribArray().pop_back();
        }

        return true;
    }




    void
        connectivityPoint(
            const GA_RWHandleT<GA_Size>& attrib_h,
            const GA_ROHandleT<UT_ValArray<GA_Offset>>& adjElemsAttrib_h
        )
    {
        //double timeTotal = 0;
        //double timeTotal1 = 0;
        //auto timeStart = std::chrono::steady_clock::now();
        //auto timeEnd = std::chrono::steady_clock::now();
        //timeStart = std::chrono::steady_clock::now();
        //timeEnd = std::chrono::steady_clock::now();
        //std::chrono::duration<double> diff;

        //::std::vector<::std::vector<GA_Offset>> adjElems;

        const GA_PointGroup* const geoGroup = groupParser.getPointGroup();
        GA_Detail* geo = this->geo;

        GA_Size nelems = geo->getNumPoints();

        ::std::vector<GA_Offset> elemHeap;
        elemHeap.reserve(__min(pow(2, 15), nelems));

        ::std::vector<GA_Offset> classnumArray(nelems, UNREACHED_NUMBER);

        UT_PackedArrayOfArrays<GA_Offset> packedArray;
        adjElemsAttrib_h->getAIFNumericArray()->getPackedArrayFromIndices(adjElemsAttrib_h.getAttribute(), 0, nelems, packedArray);

        const UT_Array<GA_Size>& rawOffsets = packedArray.rawOffsets();
        UT_Array<GA_Offset>& rawData = packedArray.rawData();

        if (rawData.size() < pow(2, 12))
        {
            for (GA_Offset elemoff = 0; elemoff < rawData.size(); ++elemoff)
            {
                rawData[elemoff] = geo->pointIndex(rawData[elemoff]);
            }
        }
        else
        {
            UTparallelFor(UT_BlockedRange<GA_Size>(0, rawData.size()), [geo, &rawData](const UT_BlockedRange<GA_Size>& r) {
                for (GA_Offset elemoff = r.begin(); elemoff != r.end(); ++elemoff)
                {
                    rawData[elemoff] = geo->pointIndex(rawData[elemoff]);
                }
            }, 16, 1024);
        }


        //timeEnd = std::chrono::steady_clock::now();
        //diff = timeEnd - timeStart;
        //timeTotal += diff.count();
        //timeStart = std::chrono::steady_clock::now();


        GA_Size classnum = 0;
        for (GA_Size elemoff = 0; elemoff < nelems; ++elemoff)
        {
            if (classnumArray[elemoff] != UNREACHED_NUMBER)
                continue;
            classnumArray[elemoff] = classnum;
            elemHeap.emplace_back(elemoff);
            while (!elemHeap.empty())
            {
                GA_Size lastElem = elemHeap[elemHeap.size() - 1];
                elemHeap.pop_back();
                GA_Size rawOffsetEnd = rawOffsets[lastElem + 1];
                for (GA_Size i = rawOffsets[lastElem]; i < rawOffsetEnd; ++i)
                {
                    GA_Offset rawDataVal = rawData[i];
                    if (classnumArray[rawDataVal] != UNREACHED_NUMBER)
                        continue;
                    classnumArray[rawDataVal] = classnum;
                    elemHeap.emplace_back(rawDataVal);
                }
            }
            ++classnum;
        }

        const GA_SplittableRange geoSplittableRange0(geo->getPointRange());
        //const GA_SplittableRange geoSplittableRange0(geo->getPrimitiveRange());
        UTparallelFor(geoSplittableRange0, [geo, &attrib_h, &classnumArray](const GA_SplittableRange& r) {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_h.set(elemoff, classnumArray[geo->pointIndex(elemoff)]);
                }
            }
        }, 16, 1024);
        //geo->setAttributeFromArray(attrib_h.getAttribute(), GA_Range(), UT_Array<GA_Size>(classnumArray));



        //timeEnd = std::chrono::steady_clock::now();
        //diff = timeEnd - timeStart;
        //timeTotal1 += diff.count();
        //timeStart = std::chrono::steady_clock::now();

        //geo->setDetailAttributeF("time", timeTotal * 1000);
        //geo->setDetailAttributeF("time1", timeTotal1 * 1000);
    }



    void
        connectivityPrim(
            const GA_RWHandleT<GA_Size>& attrib_h,
            const GA_ROHandleT<UT_ValArray<GA_Offset>>& adjElemsAttrib_h
        )
    {
        //double timeTotal = 0;
        //double timeTotal1 = 0;
        //auto timeStart = std::chrono::steady_clock::now();
        //auto timeEnd = std::chrono::steady_clock::now();
        //timeStart = std::chrono::steady_clock::now();
        //timeEnd = std::chrono::steady_clock::now();
        //std::chrono::duration<double> diff;

        //::std::vector<::std::vector<GA_Offset>> adjElems;

        const GA_PrimitiveGroup* const geoGroup = groupParser.getPrimitiveGroup();
        GA_Detail* geo = this->geo;

        GA_Size nelems = geo->getNumPrimitives();

        ::std::vector<GA_Offset> elemHeap;
        elemHeap.reserve(__min(pow(2, 15), nelems));

        ::std::vector<GA_Offset> classnumArray(nelems, UNREACHED_NUMBER);

        UT_PackedArrayOfArrays<GA_Offset> packedArray;
        adjElemsAttrib_h->getAIFNumericArray()->getPackedArrayFromIndices(adjElemsAttrib_h.getAttribute(), 0, nelems, packedArray);

        const UT_Array<GA_Size>& rawOffsets = packedArray.rawOffsets();
        UT_Array<GA_Offset>& rawData = packedArray.rawData();


        if (rawData.size() < pow(2, 12))
        {
            for (GA_Offset elemoff = 0; elemoff < rawData.size(); ++elemoff)
            {
                rawData[elemoff] = geo->primitiveIndex(rawData[elemoff]);
            }
        }
        else
        {
            UTparallelFor(UT_BlockedRange<GA_Size>(0, rawData.size()), [geo, &rawData](const UT_BlockedRange<GA_Size>& r) {
                for (GA_Offset elemoff = r.begin(); elemoff != r.end(); ++elemoff)
                {
                    rawData[elemoff] = geo->primitiveIndex(rawData[elemoff]);
                }
            }, 16, 1024);
        }


        GA_Size classnum = 0;
        for (GA_Size elemoff = 0; elemoff < nelems; ++elemoff)
        {
            if (classnumArray[elemoff] != UNREACHED_NUMBER)
                continue;
            classnumArray[elemoff] = classnum;
            elemHeap.emplace_back(elemoff);
            while (!elemHeap.empty())
            {
                GA_Size lastElem = elemHeap[elemHeap.size() - 1];
                elemHeap.pop_back();
                GA_Size rawOffsetEnd = rawOffsets[lastElem + 1];
                for (GA_Size i = rawOffsets[lastElem]; i < rawOffsetEnd; ++i)
                {
                    GA_Offset rawDataVal = rawData[i];
                    if (classnumArray[rawDataVal] != UNREACHED_NUMBER)
                        continue;
                    classnumArray[rawDataVal] = classnum;
                    elemHeap.emplace_back(rawDataVal);
                }
            }
            ++classnum;
        }

        const GA_SplittableRange geoSplittableRange0(geo->getPrimitiveRange());
        //const GA_SplittableRange geoSplittableRange0(geo->getPrimitiveRange());
        UTparallelFor(geoSplittableRange0, [geo, &attrib_h, &classnumArray](const GA_SplittableRange& r) {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_h.set(elemoff, classnumArray[geo->primitiveIndex(elemoff)]);
                }
            }
        }, 16, 1024);
        //geo->setAttributeFromArray(attrib_h.getAttribute(), GA_Range(), UT_Array<GA_Size>(classnumArray));
    }



    void
        connectivity()
    {
        GA_Attribute* adjElemsAttrib = nullptr;
        if (connectivityConstraint)
        {
            adjElemsAttrib = GFE_Adjacency::addAttribPrimPrimEdge(geo, groupParser.getPrimitiveGroup(), groupParserSeam.getVertexGroup());
        }
        else
        {
            adjElemsAttrib = GFE_Adjacency::addAttribPointPointEdge(geo, groupParser.getPointGroup(), groupParserSeam.getPointGroup());
        }
        const GA_ROHandleT<UT_ValArray<GA_Offset>> adjElemsAttrib_h(adjElemsAttrib);

        if (connectivityConstraint)
        {
            connectivityPrim(getOutAttribArray()[0], adjElemsAttrib_h);
        }
        else
        {
            connectivityPoint(getOutAttribArray()[0], adjElemsAttrib_h);
        }
    }



public:
    GFE_GroupParser groupParserSeam;
    bool connectivityConstraint = false; // false means point  and  true means edge 

private:
    const UT_StringHolder TEMP_ConnectivityAttribName = "__TEMP_GFE_ConnectivityAttrib";
#if 0
    const exint UNREACHED_NUMBER = SYS_EXINT_MAX;
#else
    const exint UNREACHED_NUMBER = -1;
#endif
};





















namespace GFE_Connectivity_Namespace {



#if 0
    #define UNREACHED_NUMBER SYS_EXINT_MAX
#else
    #define UNREACHED_NUMBER -1
#endif


static void
connectivityPoint(
    const GA_Detail* const geo,
    const GA_RWHandleT<GA_Size>& attrib_h,
    const GA_ROHandleT<UT_ValArray<GA_Offset>>& adjElemsAttrib_h,
    const GA_PointGroup* const geoGroup = nullptr
)
{
    //double timeTotal = 0;
    //double timeTotal1 = 0;
    //auto timeStart = std::chrono::steady_clock::now();
    //auto timeEnd = std::chrono::steady_clock::now();
    //timeStart = std::chrono::steady_clock::now();
    //timeEnd = std::chrono::steady_clock::now();
    //std::chrono::duration<double> diff;

    //::std::vector<::std::vector<GA_Offset>> adjElems;
    GA_Size nelems = geo->getNumPoints();

    ::std::vector<GA_Offset> elemHeap;
    elemHeap.reserve(__min(pow(2, 15), nelems));

    ::std::vector<GA_Offset> classnumArray(nelems, UNREACHED_NUMBER);

    UT_PackedArrayOfArrays<GA_Offset> packedArray;
    adjElemsAttrib_h->getAIFNumericArray()->getPackedArrayFromIndices(adjElemsAttrib_h.getAttribute(), 0, nelems, packedArray);

    const UT_Array<GA_Size>& rawOffsets = packedArray.rawOffsets();
    UT_Array<GA_Offset>& rawData = packedArray.rawData();

    if (rawData.size() < pow(2, 12))
    {
        for (GA_Offset elemoff = 0; elemoff < rawData.size(); ++elemoff)
        {
            rawData[elemoff] = geo->pointIndex(rawData[elemoff]);
        }
    }
    else
    {
        UTparallelFor(UT_BlockedRange<GA_Size>(0, rawData.size()), [geo, &rawData](const UT_BlockedRange<GA_Size>& r) {
            for (GA_Offset elemoff = r.begin(); elemoff != r.end(); ++elemoff)
            {
                rawData[elemoff] = geo->pointIndex(rawData[elemoff]);
            }
        }, 16, 1024);
    }


    //timeEnd = std::chrono::steady_clock::now();
    //diff = timeEnd - timeStart;
    //timeTotal += diff.count();
    //timeStart = std::chrono::steady_clock::now();


    GA_Size classnum = 0;
    for (GA_Size elemoff = 0; elemoff < nelems; ++elemoff)
    {
        if (classnumArray[elemoff] != UNREACHED_NUMBER)
            continue;
        classnumArray[elemoff] = classnum;
        elemHeap.emplace_back(elemoff);
        while (!elemHeap.empty())
        {
            GA_Size lastElem = elemHeap[elemHeap.size() - 1];
            elemHeap.pop_back();
            GA_Size rawOffsetEnd = rawOffsets[lastElem + 1];
            for (GA_Size i = rawOffsets[lastElem]; i < rawOffsetEnd; ++i)
            {
                GA_Offset rawDataVal = rawData[i];
                if (classnumArray[rawDataVal] != UNREACHED_NUMBER)
                    continue;
                classnumArray[rawDataVal] = classnum;
                elemHeap.emplace_back(rawDataVal);
            }
        }
        ++classnum;
    }

    const GA_SplittableRange geoSplittableRange0(geo->getPointRange());
    //const GA_SplittableRange geoSplittableRange0(geo->getPrimitiveRange());
    UTparallelFor(geoSplittableRange0, [geo, &attrib_h, &classnumArray](const GA_SplittableRange& r) {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                attrib_h.set(elemoff, classnumArray[geo->pointIndex(elemoff)]);
            }
        }
    }, 16, 1024);
    //geo->setAttributeFromArray(attrib_h.getAttribute(), GA_Range(), UT_Array<GA_Size>(classnumArray));



    //timeEnd = std::chrono::steady_clock::now();
    //diff = timeEnd - timeStart;
    //timeTotal1 += diff.count();
    //timeStart = std::chrono::steady_clock::now();

    //geo->setDetailAttributeF("time", timeTotal * 1000);
    //geo->setDetailAttributeF("time1", timeTotal1 * 1000);
}



static void
connectivityPrim(
    const GA_Detail* const geo,
    const GA_RWHandleT<GA_Size>& attrib_h,
    const GA_ROHandleT<UT_ValArray<GA_Offset>>& adjElemsAttrib_h,
    const GA_PrimitiveGroup* const geoGroup = nullptr
)
{
    //double timeTotal = 0;
    //double timeTotal1 = 0;
    //auto timeStart = std::chrono::steady_clock::now();
    //auto timeEnd = std::chrono::steady_clock::now();
    //timeStart = std::chrono::steady_clock::now();
    //timeEnd = std::chrono::steady_clock::now();
    //std::chrono::duration<double> diff;

    //::std::vector<::std::vector<GA_Offset>> adjElems;
    GA_Size nelems = geo->getNumPrimitives();

    ::std::vector<GA_Offset> elemHeap;
    elemHeap.reserve(__min(pow(2, 15), nelems));

    ::std::vector<GA_Offset> classnumArray(nelems, UNREACHED_NUMBER);

    UT_PackedArrayOfArrays<GA_Offset> packedArray;
    adjElemsAttrib_h->getAIFNumericArray()->getPackedArrayFromIndices(adjElemsAttrib_h.getAttribute(), 0, nelems, packedArray);

    const UT_Array<GA_Size>& rawOffsets = packedArray.rawOffsets();
    UT_Array<GA_Offset>& rawData = packedArray.rawData();


    if (rawData.size() < pow(2, 12))
    {
        for (GA_Offset elemoff = 0; elemoff < rawData.size(); ++elemoff)
        {
            rawData[elemoff] = geo->primitiveIndex(rawData[elemoff]);
        }
    }
    else
    {
        UTparallelFor(UT_BlockedRange<GA_Size>(0, rawData.size()), [geo, &rawData](const UT_BlockedRange<GA_Size>& r) {
            for (GA_Offset elemoff = r.begin(); elemoff != r.end(); ++elemoff)
            {
                rawData[elemoff] = geo->primitiveIndex(rawData[elemoff]);
            }
        }, 16, 1024);
    }


    GA_Size classnum = 0;
    for (GA_Size elemoff = 0; elemoff < nelems; ++elemoff)
    {
        if (classnumArray[elemoff] != UNREACHED_NUMBER)
            continue;
        classnumArray[elemoff] = classnum;
        elemHeap.emplace_back(elemoff);
        while (!elemHeap.empty())
        {
            GA_Size lastElem = elemHeap[elemHeap.size() - 1];
            elemHeap.pop_back();
            GA_Size rawOffsetEnd = rawOffsets[lastElem + 1];
            for (GA_Size i = rawOffsets[lastElem]; i < rawOffsetEnd; ++i)
            {
                GA_Offset rawDataVal = rawData[i];
                if (classnumArray[rawDataVal] != UNREACHED_NUMBER)
                    continue;
                classnumArray[rawDataVal] = classnum;
                elemHeap.emplace_back(rawDataVal);
            }
        }
        ++classnum;
    }

    const GA_SplittableRange geoSplittableRange0(geo->getPrimitiveRange());
    //const GA_SplittableRange geoSplittableRange0(geo->getPrimitiveRange());
    UTparallelFor(geoSplittableRange0, [geo, &attrib_h, &classnumArray](const GA_SplittableRange& r) {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                attrib_h.set(elemoff, classnumArray[geo->primitiveIndex(elemoff)]);
            }
        }
    }, 16, 1024);
    //geo->setAttributeFromArray(attrib_h.getAttribute(), GA_Range(), UT_Array<GA_Size>(classnumArray));
}





//return addAttribConnectivityPoint(geo, name, geoGroup, geoSeamGroup, 0, 0, storage, reuse, subscribeRatio, minGrainSize);

static GA_Attribute*
addAttribConnectivityPoint(
    GA_Detail* const geo,
    const GA_PointGroup* const geoGroup = nullptr,
    const GA_PointGroup* const geoSeamGroup = nullptr,
    const GA_Storage storage = GA_STORE_INVALID,
    const UT_StringHolder& name = "__topo_connectivity",
    const UT_Options* const creation_args = nullptr,
    const GA_AttributeOptions* const attribute_options = nullptr,
    const GA_ReuseStrategy& reuse = GA_ReuseStrategy()
    //,const exint subscribeRatio = 32,
    //const exint minGrainSize = 1024
)
{
    GA_Attribute* attribPtr = geo->findPointAttribute(name);
    if (attribPtr)
        return attribPtr;

    const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;

    attribPtr = geo->getAttributes().createTupleAttribute(GA_ATTRIB_POINT, GFE_TOPO_SCOPE, name, finalStorage, 1, GA_Defaults(-1), creation_args, attribute_options, reuse);

    const GA_ROHandleT<UT_ValArray<GA_Offset>> adjElemsAttrib_h =
        GFE_Adjacency::addAttribPointPointEdge(geo, geoGroup, geoSeamGroup, finalStorage);
    //    GFE_Adjacency::addAttribPointPointEdge(geo, geoGroup, geoSeamGroup, finalStorage,
    //        "__topo_nebs", nullptr, nullptr, GA_ReuseStrategy(), subscribeRatio, minGrainSize);

    connectivityPoint(geo, attribPtr, adjElemsAttrib_h, geoGroup);
    return attribPtr;
}









//return addAttribConnectivityPrim(geo, name, geoGroup, geoSeamGroup, 0, 0, storage, reuse, subscribeRatio, minGrainSize);

static GA_Attribute*
addAttribConnectivityPrim(
    GA_Detail* const geo,
    const GA_PrimitiveGroup* geoGroup = nullptr,
    const GA_VertexGroup* geoSeamGroup = nullptr,
    const GA_Storage storage = GA_STORE_INVALID,
    const UT_StringHolder& name = "__topo_connectivity",
    const UT_Options* const creation_args = nullptr,
    const GA_AttributeOptions* const attribute_options = nullptr,
    const GA_ReuseStrategy& reuse = GA_ReuseStrategy()
    //,const exint subscribeRatio = 32,
    //const exint minGrainSize = 1024
)
{
    GA_Attribute* attribPtr = geo->findAttribute(GA_ATTRIB_PRIMITIVE, GFE_TOPO_SCOPE, name);
    if (attribPtr)
        return attribPtr;
    const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;

    const GA_ROHandleT<UT_ValArray<GA_Offset>> adjElemsAttrib_h =
        GFE_Adjacency::addAttribPrimPrimEdge(geo, geoGroup, geoSeamGroup, finalStorage);
    //    GFE_Adjacency::addAttribPrimPrimEdge(geo, geoGroup, geoSeamGroup, finalStorage,
    //        "__topo_nebs", nullptr, nullptr, GA_ReuseStrategy(), subscribeRatio, minGrainSize);

    attribPtr = geo->getAttributes().createTupleAttribute(GA_ATTRIB_PRIMITIVE, GFE_TOPO_SCOPE, name, finalStorage, 1, GA_Defaults(-1), creation_args, attribute_options, reuse);

    connectivityPrim(geo, attribPtr, adjElemsAttrib_h, geoGroup);
    return attribPtr;
}



static GA_Attribute*
addAttribConnectivity(
    GA_Detail* const geo,
    const GA_Group* geoGroup = nullptr,
    const GA_Group* geoSeamGroup = nullptr,
    const bool connectivityConstraint = false,
    const GA_AttributeOwner attribOwner = GA_ATTRIB_PRIMITIVE,
    const GA_Storage storage = GA_STORE_INVALID,
    const UT_StringHolder& name = "__topo_connectivity"
    //,const exint subscribeRatio = 32,
    //const exint minGrainSize = 1024
)
{
    GA_Attribute* attribPtr = nullptr;
    if (connectivityConstraint)
    {
        if (attribOwner != GA_ATTRIB_PRIMITIVE)
        {
            attribPtr = geo->getAttributes().findAttribute(attribOwner, GFE_TOPO_SCOPE, name);
            if (attribPtr)
                return attribPtr;
        }
        attribPtr = addAttribConnectivityPrim(geo,
            UTverify_cast<const GA_PrimitiveGroup*>(geoGroup),
            UTverify_cast<const GA_VertexGroup*>(geoSeamGroup),
            storage, name
            //,nullptr, nullptr, GA_ReuseStrategy()
            //,subscribeRatio, minGrainSize
            );
        if (attribOwner != GA_ATTRIB_PRIMITIVE)
        {
            //attribPtr = GU_Promote::promote(*geo, attribPtr, attribOwner, true, GU_Promote::GU_PROMOTE_FIRST);
            GA_Attribute* const origAttribPtr = attribPtr;
            attribPtr = GFE_AttributePromote::attribPromote(geo, attribPtr, attribOwner);
            geo->getAttributes().destroyAttribute(origAttribPtr);
        }
    }
    else
    {
        if (attribOwner != GA_ATTRIB_POINT)
        {
            attribPtr = geo->getAttributes().findAttribute(attribOwner, GFE_TOPO_SCOPE, name);
            if (attribPtr)
                return attribPtr;
        }
        attribPtr = addAttribConnectivityPoint(geo,
            UTverify_cast<const GA_PointGroup*>(geoGroup),
            UTverify_cast<const GA_PointGroup*>(geoSeamGroup),
            storage, name
            //,nullptr, nullptr, GA_ReuseStrategy()
            //,subscribeRatio, minGrainSize
            );
        if (attribOwner != GA_ATTRIB_POINT)
        {
            //attribPtr = GU_Promote::promote(*geo, attribPtr, attribOwner, true, GU_Promote::GU_PROMOTE_FIRST);
            GA_Attribute* const origAttribPtr = attribPtr;
            attribPtr = GFE_AttributePromote::attribPromote(geo, attribPtr, attribOwner);
            geo->getAttributes().destroyAttribute(origAttribPtr);
        }
    }
    return attribPtr;
}



} // End of namespace GFE_Connectivity

#endif