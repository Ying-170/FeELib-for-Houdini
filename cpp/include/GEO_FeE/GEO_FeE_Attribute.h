
#pragma once

#ifndef __GEO_FeE_Attribute_h__
#define __GEO_FeE_Attribute_h__

//#include <GEO_FeE/GEO_FeE_Attribute.h>

#include <GA/GA_Detail.h>

#include <GA_FeE/GA_FeE_Attribute.h>
#include <GA_FeE/GA_FeE_Group.h>
#include <GEO_FeE/GEO_FeE_Group.h>


namespace GEO_FeE_Attribute {

    

    template<typename T>
    static void
        setAttribValue1(
            GA_Detail* geo,
            GA_Attribute* attrib,
            GA_SplittableRange& geoSplittableRange,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 128
        )
    {
        const GA_RWHandleT<T> attribH(attrib);
        UTparallelFor(geoSplittableRange, [&geo, &attribH](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attribH.set(elemoff, T(1));
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    static void
        setAttribStringValue1(
            GA_Detail* geo,
            GA_Attribute* attrib,
            GA_SplittableRange& geoSplittableRange,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 128
        )
    {
        const GA_RWHandleS attribH(attrib);
        UTparallelFor(geoSplittableRange, [&geo, &attribH](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attribH.set(elemoff, "1");
                    }
                }
            }, subscribeRatio, minGrainSize);
    }



    static bool
        attribCast(
            GEO_Detail* geo,
            const GA_Group* group,
            const GA_StorageClass newStorageClass,
            const UT_StringHolder& newName,
            const GA_Precision precision = GA_PRECISION_32,
            const bool detached = false,
            const bool delOriginal = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 128
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);

        if (newStorageClass == GA_STORECLASS_OTHER)//group
            return false;

        if (!group->isElementGroup())
            return false;

        const bool useNewName = newName.isstring() && newName.length() != 0;
        //const GA_GroupType classType = group->classType();
        const GA_AttributeOwner attribClass = GA_FeE_Type::attributeOwner_groupType(group->classType());

        GA_SplittableRange geoSplittableRange = GA_FeE_Group::getSplittableRangeByAnyGroup(geo, group);

        switch (newStorageClass)
        {
        case GA_STORECLASS_INT:
        {
            GA_Attribute* attrib = geo->addIntTuple(attribClass, newName, 1, GA_Defaults(0), 0, 0, GA_FeE_Type::getPreferredStorageI(precision));
            switch (precision)
            {
            case GA_PRECISION_8:
                setAttribValue1<int8> (geo, attrib, geoSplittableRange, subscribeRatio, minGrainSize);
                return true; break;
            case GA_PRECISION_16:
                setAttribValue1<int16>(geo, attrib, geoSplittableRange, subscribeRatio, minGrainSize);
                return true; break;
            case GA_PRECISION_32:
                setAttribValue1<int32>(geo, attrib, geoSplittableRange, subscribeRatio, minGrainSize);
                return true; break;
            case GA_PRECISION_64:
                setAttribValue1<int64>(geo, attrib, geoSplittableRange, subscribeRatio, minGrainSize);
                return true; break;
            default:                              break;
            }
        }
            break;
        case GA_STORECLASS_REAL:
        {
            GA_Attribute* attrib = geo->addFloatTuple(attribClass, newName, 1, GA_Defaults(0), 0, 0, GA_FeE_Type::getPreferredStorageI(precision));
            switch (precision)
            {
            case GA_PRECISION_16:
                setAttribValue1<fpreal16>(geo, attrib, geoSplittableRange, subscribeRatio, minGrainSize);
                return true; break;
            case GA_PRECISION_32:
                setAttribValue1<fpreal32>(geo, attrib, geoSplittableRange, subscribeRatio, minGrainSize);
                return true; break;
            case GA_PRECISION_64:
                setAttribValue1<fpreal64>(geo, attrib, geoSplittableRange, subscribeRatio, minGrainSize);
                return true; break;
            default:                               break;
            }
        }
            break;
        case GA_STORECLASS_STRING:
        {
            GA_Attribute* attrib = geo->addStringTuple(attribClass, newName, 1, 0, 0);
            setAttribStringValue1(geo, attrib, geoSplittableRange, subscribeRatio, minGrainSize);
            return true;
        }
            break;
        case GA_STORECLASS_DICT:
            break;

        case GA_STORECLASS_OTHER:
            return false;
            break;
        default:
            break;
        }
        //return geo->changeAttributeStorage(attribClass, group->getName(), newStorage);
        UT_ASSERT_MSG(0, "Unhandled Precision!");
        return false;
    }






    //GEO_FeE_Attribute::normalizeElementAttrib(outGeo0, geo0Group, geo0AttribClass, attribPtr,
    //    doNormalize, uniScale,
    //    subscribeRatio, minGrainSize);

    static void
        normalizeElementAttrib(
            const GA_Detail* geo,
            const GA_Group* geoGroup,
            const GA_AttributeOwner attribOwner,
            GA_Attribute* attribPtr,
            const bool doNormalize = 1,
            const fpreal64 uniScale = 1,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(geoGroup);
        const GA_SplittableRange geoSplittableRange = GEO_FeE_Group::getSplittableRangeByAnyGroup(geo, geoGroup, attribOwner);
        GA_FeE_Attribute::normalizeElementAttrib(geoSplittableRange, attribPtr,
            doNormalize, uniScale,
            subscribeRatio, minGrainSize);
    }












} // End of namespace GEO_FeE_Attribute

#endif
