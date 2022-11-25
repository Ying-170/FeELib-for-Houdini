
#pragma once

#ifndef __GA_FeE_Connectivity_h__
#define __GA_FeE_Connectivity_h__

//#include <GU_FeE/GA_FeE_Connectivity.h>

#include <GEO/GEO_Detail.h>

#include <GA_FeE/GA_FeE_Adjacency.h>

#include <chrono>


namespace GA_FeE_Connectivity
{


#if 0
    #define UNREACHED_NUMBER SYS_EXINT_MAX
#else
    #define UNREACHED_NUMBER -1
#endif


static void
connectivityPoint(
    const GA_Detail* geo,
    const GA_RWHandleT<GA_Size>& attribHandle,
    const GA_ROHandleT<UT_ValArray<GA_Offset>>& adjElemsAttribHandle,
    const GA_PointGroup* geoGroup = nullptr
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
    adjElemsAttribHandle->getAIFNumericArray()->getPackedArrayFromIndices(adjElemsAttribHandle.getAttribute(), 0, nelems, packedArray);

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
        UTparallelFor(UT_BlockedRange<GA_Size>(0, rawData.size()), [&geo, &rawData](const UT_BlockedRange<GA_Size>& r) {
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
    UTparallelFor(geoSplittableRange0, [&geo, &attribHandle, &classnumArray](const GA_SplittableRange& r) {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                attribHandle.set(elemoff, classnumArray[geo->pointIndex(elemoff)]);
            }
        }
    }, 16, 1024);



    //timeEnd = std::chrono::steady_clock::now();
    //diff = timeEnd - timeStart;
    //timeTotal1 += diff.count();
    //timeStart = std::chrono::steady_clock::now();

    //geo->setDetailAttributeF("time", timeTotal * 1000);
    //geo->setDetailAttributeF("time1", timeTotal1 * 1000);
}



static void
connectivityPrim(
    const GA_Detail* geo,
    const GA_RWHandleT<GA_Size>& attribHandle,
    const GA_ROHandleT<UT_ValArray<GA_Offset>>& adjElemsAttribHandle,
    const GA_PrimitiveGroup* geoGroup = nullptr
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
    adjElemsAttribHandle->getAIFNumericArray()->getPackedArrayFromIndices(adjElemsAttribHandle.getAttribute(), 0, nelems, packedArray);

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
        UTparallelFor(UT_BlockedRange<GA_Size>(0, rawData.size()), [&geo, &rawData](const UT_BlockedRange<GA_Size>& r) {
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
    UTparallelFor(geoSplittableRange0, [&geo, &attribHandle, &classnumArray](const GA_SplittableRange& r) {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                attribHandle.set(elemoff, classnumArray[geo->primitiveIndex(elemoff)]);
            }
        }
    }, 16, 1024);
}





//return addAttribConnectivityPoint(geo, name, geoGroup, geoSeamGroup, 0, 0, storage, reuse, subscribeRatio, minGrainSize);

//addAttribConnectivity
static GA_Attribute*
addAttribConnectivityPoint(
    GEO_Detail* geo,
    const UT_StringHolder& name = "connectivity",
    const GA_PointGroup* geoGroup = nullptr,
    const GA_PointGroup* geoSeamGroup = nullptr,
    const UT_Options* creation_args = 0,
    const GA_AttributeOptions* attribute_options = 0,
    const GA_Storage& storage = GA_STORE_INT64,
    const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
    const exint& subscribeRatio = 64,
    const exint& minGrainSize = 64
)
{
    GA_Attribute* attribPtr = geo->findPointAttribute(name);
    if (attribPtr)
        return attribPtr;

    attribPtr = geo->addIntTuple(GA_ATTRIB_POINT, name, 1, GA_Defaults(-1), creation_args, attribute_options, storage, reuse);
    const GA_ROHandleT<UT_ValArray<GA_Offset>> adjElemsAttribHandle = GA_FeE_Adjacency::addAttribPointPointEdge(geo, "nebs", geoGroup, geoSeamGroup, storage, subscribeRatio, minGrainSize);
    connectivityPoint(geo, attribPtr, adjElemsAttribHandle, geoGroup);
    return attribPtr;
}

//return addAttribConnectivityPoint(geo, name, geoGroup, geoSeamGroup, storage, reuse, subscribeRatio, minGrainSize);

//addAttribConnectivity
static GA_Attribute*
addAttribConnectivityPoint(
    GEO_Detail* geo,
    const UT_StringHolder& name = "connectivity",
    const GA_PointGroup* geoGroup = nullptr,
    const GA_PointGroup* geoSeamGroup = nullptr,
    const GA_Storage& storage = GA_STORE_INT64,
    const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
    const exint& subscribeRatio = 64,
    const exint& minGrainSize = 64
)
{
    return addAttribConnectivityPoint(geo, name, geoGroup, geoSeamGroup, 0, 0, storage, reuse, subscribeRatio, minGrainSize);
}


//return addAttribConnectivityPoint(geo, name, geoGroup, geoSeamGroup, storage, subscribeRatio, minGrainSize);

//addAttribConnectivity
static GA_Attribute*
addAttribConnectivityPoint(
    GEO_Detail* geo,
    const UT_StringHolder& name = "connectivity",
    const GA_PointGroup* geoGroup = nullptr,
    const GA_PointGroup* geoSeamGroup = nullptr,
    const GA_Storage& storage = GA_STORE_INT64,
    const exint& subscribeRatio = 32,
    const exint& minGrainSize = 1024
)
{
    return addAttribConnectivityPoint(geo, name, geoGroup, geoSeamGroup, 0, 0, storage, GA_ReuseStrategy(), subscribeRatio, minGrainSize);
}











//return addAttribConnectivityPrim(geo, name, geoGroup, geoSeamGroup, 0, 0, storage, reuse, subscribeRatio, minGrainSize);

//addAttribConnectivity
static GA_Attribute*
addAttribConnectivityPrim(
    GEO_Detail* geo,
    const UT_StringHolder& name = "connectivity",
    const GA_PrimitiveGroup* geoGroup = nullptr,
    const GA_VertexGroup* geoSeamGroup = nullptr,
    const UT_Options* creation_args = 0,
    const GA_AttributeOptions* attribute_options = 0,
    const GA_Storage& storage = GA_STORE_INT64,
    const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
    const exint& subscribeRatio = 32,
    const exint& minGrainSize = 1024
)
{
    GA_Attribute* attribPtr = geo->findPrimitiveAttribute(name);
    if (attribPtr)
        return attribPtr;
    const GA_ROHandleT<UT_ValArray<GA_Offset>> adjElemsAttribHandle =
        GA_FeE_Adjacency::addAttribPrimPrimEdge(geo, "nebs", geoGroup, geoSeamGroup,
            storage, subscribeRatio, minGrainSize);
    
    attribPtr = geo->addIntTuple(GA_ATTRIB_PRIMITIVE, name, 1, GA_Defaults(-1), creation_args, attribute_options, storage, reuse);
    connectivityPrim(geo, attribPtr, adjElemsAttribHandle, geoGroup);
    return attribPtr;
}


//return addAttribConnectivityPrim(geo, name, geoGroup, geoSeamGroup, storage, reuse, subscribeRatio, minGrainSize);

//addAttribConnectivity
static GA_Attribute*
addAttribConnectivityPrim(
    GEO_Detail* geo,
    const UT_StringHolder& name = "connectivity",
    const GA_PrimitiveGroup* geoGroup = nullptr,
    const GA_VertexGroup* geoSeamGroup = nullptr,
    const GA_Storage& storage = GA_STORE_INT64,
    const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
    const exint& subscribeRatio = 32,
    const exint& minGrainSize = 1024
)
{
    return addAttribConnectivityPrim(geo, name, geoGroup, geoSeamGroup, 0, 0, storage, reuse, subscribeRatio, minGrainSize);
}


//return addAttribConnectivityPrim(geo, name, geoGroup, geoSeamGroup, storage, subscribeRatio, minGrainSize);

//addAttribConnectivity
static GA_Attribute*
addAttribConnectivityPrim(
    GEO_Detail* geo,
    const UT_StringHolder& name = "connectivity",
    const GA_PrimitiveGroup* geoGroup = nullptr,
    const GA_VertexGroup* geoSeamGroup = nullptr,
    const GA_Storage& storage = GA_STORE_INT64,
    const exint& subscribeRatio = 32,
    const exint& minGrainSize = 1024
)
{
    return addAttribConnectivityPrim(geo, name, geoGroup, geoSeamGroup, 0, 0, storage, GA_ReuseStrategy(), subscribeRatio, minGrainSize);
}





} // End of namespace GA_FeE_Connectivity

#endif