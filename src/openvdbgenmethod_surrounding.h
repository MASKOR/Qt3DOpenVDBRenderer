//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// This file contains modified code from the OpenVDB library, which is available under MPL 2.0 license.
//
// The original code can be found at https://github.com/dreamworksanimation/openvdb
// https://github.com/dreamworksanimation/openvdb/blob/master/openvdb/tools/LevelSetUtil.h
// and https://github.com/dreamworksanimation/openvdb/blob/980d7fdb00d7c9ca5d1e72f871d5509b30e78f9d/openvdb/viewer/RenderModules.cc
//
///////////////////////////////////////////////////////////////////////////

#ifndef OPENVDBGENMETHOD_SURROUNDING_H
#define OPENVDBGENMETHOD_SURROUNDING_H

#include <QOpenGLFunctions>
#include <openvdb/openvdb.h>
#include <openvdb/io/Stream.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetRebuild.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/PointScatter.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tree/LeafManager.h>
#include <openvdb/Grid.h>

template<typename TreeType>
class PointGenerator
{
public:
    typedef openvdb::tree::LeafManager<TreeType> LeafManagerType;

    PointGenerator(
        std::vector<GLfloat>& points,
        std::vector<GLuint>& indices,
        LeafManagerType& leafs,
        std::vector<size_t>& indexMap,
        const openvdb::math::Transform& transform,
        openvdb::Index64 voxelsPerLeaf = TreeType::LeafNodeType::NUM_VOXELS)
        : mPoints(points)
        , mIndices(indices)
        , mLeafs(leafs)
        , mIndexMap(indexMap)
        , mTransform(transform)
        , mVoxelsPerLeaf(voxelsPerLeaf)
    {
    }

    void runParallel()
    {
        tbb::parallel_for(mLeafs.getRange(), *this);
    }


    inline void operator()(const typename LeafManagerType::RangeType& range) const
    {
        using openvdb::Index64;

        typedef typename TreeType::LeafNodeType::ValueOnCIter ValueOnCIter;

        openvdb::Vec3d pos;
        size_t index = 0;
        Index64 activeVoxels = 0;

        for (size_t n = range.begin(); n < range.end(); ++n) {

            index = mIndexMap[n];
            ValueOnCIter it = mLeafs.leaf(n).cbeginValueOn();

            activeVoxels = mLeafs.leaf(n).onVoxelCount();

            if (activeVoxels <= mVoxelsPerLeaf) {

                for ( ; it; ++it) {
                    pos = mTransform.indexToWorld(it.getCoord());
                    insertPoint(pos, index);
                    ++index;
                }

            } else if (1 == mVoxelsPerLeaf) {

                 pos = mTransform.indexToWorld(it.getCoord());
                 insertPoint(pos, index);

            } else {

                std::vector<openvdb::Coord> coords;
                coords.reserve(static_cast<size_t>(activeVoxels));
                for ( ; it; ++it) { coords.push_back(it.getCoord()); }

                pos = mTransform.indexToWorld(coords[0]);
                insertPoint(pos, index);
                ++index;

                pos = mTransform.indexToWorld(coords[static_cast<size_t>(activeVoxels-1)]);
                insertPoint(pos, index);
                ++index;

                Index64 r = Index64(std::floor(double(mVoxelsPerLeaf) / activeVoxels));
                for (Index64 i = 1, I = mVoxelsPerLeaf - 2; i < I; ++i) {
                    pos = mTransform.indexToWorld(coords[static_cast<size_t>(i * r)]);
                    insertPoint(pos, index);
                    ++index;
                }
            }
        }
    }

private:
    void insertPoint(const openvdb::Vec3d& pos, size_t index) const
    {
        mIndices[index] = GLuint(index);
        const size_t element = index * 3;
        mPoints[element    ] = static_cast<GLfloat>(pos[0]);
        mPoints[element + 1] = static_cast<GLfloat>(pos[1]);
        mPoints[element + 2] = static_cast<GLfloat>(pos[2]);
    }

    std::vector<GLfloat>& mPoints;
    std::vector<GLuint>& mIndices;
    LeafManagerType& mLeafs;
    std::vector<size_t>& mIndexMap;
    const openvdb::math::Transform& mTransform;
    const openvdb::Index64 mVoxelsPerLeaf;
}; // PointGenerator

template<typename GridType>
class PointAttributeGenerator
{
public:
    typedef typename GridType::ValueType ValueType;

    PointAttributeGenerator(
        std::vector<GLfloat>& points,
//        std::vector<GLfloat>& colors,
        const GridType& grid,
//        ValueType minValue,
//        ValueType maxValue,
//        openvdb::Vec3s (&colorMap)[4],
        bool isLevelSet = false)
        : mPoints(points)
//        , mColors(colors)
        , mNormals(NULL)
        , mGrid(grid)
        , mAccessor(grid.tree())
//        , mMinValue(minValue)
//        , mMaxValue(maxValue)
//        , mColorMap(colorMap)
        , mIsLevelSet(isLevelSet)
        , mZeroValue(openvdb::zeroVal<ValueType>())
    {
        init();
    }

    PointAttributeGenerator(
        std::vector<GLfloat>& points,
        //std::vector<GLfloat>& colors,
        std::vector<GLfloat>& normals,
        const GridType& grid,
        //ValueType minValue,
        //ValueType maxValue,
        //openvdb::Vec3s (&colorMap)[4],
        bool isLevelSet = false)
        : mPoints(points)
//        , mColors(colors)
        , mNormals(&normals)
        , mGrid(grid)
        , mAccessor(grid.tree())
//        , mMinValue(minValue)
//        , mMaxValue(maxValue)
//        , mColorMap(colorMap)
        , mIsLevelSet(isLevelSet)
//        , mZeroValue(openvdb::zeroVal<ValueType>())
    {
        init();
    }

    void runParallel()
    {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, (mPoints.size() / 3)), *this);
    }

    inline void operator()(const tbb::blocked_range<size_t>& range) const
    {
        openvdb::Coord ijk;
        openvdb::Vec3d pos, tmpNormal, normal(0.0, -1.0, 0.0);
        //openvdb::Vec3s color(0.9f, 0.3f, 0.3f);
        float w = 0.0;

        size_t e1, e2, e3, voxelNum = 0;
        for (size_t n = range.begin(); n < range.end(); ++n) {
            e1 = 3 * n;
            e2 = e1 + 1;
            e3 = e2 + 1;

            pos[0] = mPoints[e1];
            pos[1] = mPoints[e2];
            pos[2] = mPoints[e3];

            pos = mGrid.worldToIndex(pos);
            ijk[0] = int(pos[0]);
            ijk[1] = int(pos[1]);
            ijk[2] = int(pos[2]);

//            const ValueType& value = mAccessor.getValue(ijk);

//            if (value < mZeroValue) { // is negative
//                if (mIsLevelSet) {
//                    color = mColorMap[1];
//                } else {
//                    w = (float(value) - mOffset[1]) * mScale[1];
//                    color = openvdb::Vec3s(w * mColorMap[0] + (1.0 - w) * mColorMap[1]);
//                }
//            } else {
//                if (mIsLevelSet) {
//                    color = mColorMap[2];
//                } else {
//                    w = (float(value) - mOffset[0]) * mScale[0];
//                    color = openvdb::Vec3s(w * mColorMap[2] + (1.0 - w) * mColorMap[3]);
//                }
//            }

//            mColors[e1] = color[0];
//            mColors[e2] = color[1];
//            mColors[e3] = color[2];

            if (mNormals) {

                if ((voxelNum % 2) == 0) {
                    tmpNormal = openvdb::Vec3d(openvdb::math::ISGradient<
                        openvdb::math::CD_2ND>::result(mAccessor, ijk));

                    double length = tmpNormal.length();
                    if (length > 1.0e-7) {
                        tmpNormal *= 1.0 / length;
                        normal = tmpNormal;
                    }
                }
                ++voxelNum;

                (*mNormals)[e1] = static_cast<GLfloat>(normal[0]);
                (*mNormals)[e2] = static_cast<GLfloat>(normal[1]);
                (*mNormals)[e3] = static_cast<GLfloat>(normal[2]);
            }
        }
    }

private:

    void init()
    {
//        mOffset[0] = static_cast<float>(std::min(mZeroValue, mMinValue));
//        mScale[0] = static_cast<float>(
//            1.0 / (std::abs(std::max(mZeroValue, mMaxValue) - mOffset[0])));
//        mOffset[1] = static_cast<float>(std::min(mZeroValue, mMinValue));
//        mScale[1] = static_cast<float>(
//            1.0 / (std::abs(std::max(mZeroValue, mMaxValue) - mOffset[1])));
    }

    std::vector<GLfloat>& mPoints;
//    std::vector<GLfloat>& mColors;
    std::vector<GLfloat>* mNormals;

    const GridType& mGrid;
    openvdb::tree::ValueAccessor<const typename GridType::TreeType> mAccessor;

//    ValueType mMinValue, mMaxValue;
//    openvdb::Vec3s (&mColorMap)[4];
    const bool mIsLevelSet;

    ValueType mZeroValue;
//    float mOffset[2], mScale[2];
}; // PointAttributeGenerator

#endif
