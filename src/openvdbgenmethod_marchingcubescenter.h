#ifndef OPENVDBGENMETHOD_MARCHINGCUBESCENTER_H
#define OPENVDBGENMETHOD_MARCHINGCUBESCENTER_H

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

// Table for Marching Cubes Algorithm
// this table comes from Paul Baurke's web page at
//				http://paulbourke.net/geometry/polygonise/
static int edgeTable[256]={
0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0   };

inline openvdb::Vec3f lerp0(const openvdb::Coord &p1, const openvdb::Coord &p2, const float &d1, const float &d2)
{
    openvdb::Coord diff(p2 - p1);
    openvdb::Vec3f p;
    if(d1 != d2)
        p = openvdb::Vec3f(p1.x(), p1.y(), p1.z()) + openvdb::Vec3f(diff.x(), diff.y(), diff.z())/(d2 - d1)*(0.0f - d1);
    else
        p = openvdb::Vec3f(p1.x(), p1.y(), p1.z());
    return p;
}

template<typename TreeType>
class MarchingCubesPointsGenerator
{
public:
    using GridType = openvdb::Grid<TreeType>;
    using GridTypeConstPtr = typename GridType::ConstPtr;
    typedef openvdb::tree::LeafManager<const TreeType> LeafArray;
    typedef typename TreeType::ValueType ValueType;

    MarchingCubesPointsGenerator(LeafArray&, GridTypeConstPtr);

    void runParallel();
    void runSerial();

    const std::vector<ValueType>& points() const { return m_points; }

    inline MarchingCubesPointsGenerator(const MarchingCubesPointsGenerator<TreeType>&, tbb::split);
    inline void operator()(const tbb::blocked_range<size_t>&);
    inline void join(const MarchingCubesPointsGenerator<TreeType>&);

private:
    LeafArray& m_leafArray;
    GridTypeConstPtr m_grid;
    std::vector<ValueType> m_points;
};


template <class TreeType>
MarchingCubesPointsGenerator<TreeType>::MarchingCubesPointsGenerator(LeafArray& leafs, GridTypeConstPtr grid)
    : m_leafArray(leafs)
    , m_grid(grid)
    , m_points()
{
}


template <class TreeType>
inline MarchingCubesPointsGenerator<TreeType>::MarchingCubesPointsGenerator(const MarchingCubesPointsGenerator<TreeType>& rhs, tbb::split)
    : m_leafArray(rhs.m_leafArray)
    , m_grid(rhs.m_grid)
    , m_points()
{
}


template <class TreeType>
void MarchingCubesPointsGenerator<TreeType>::runParallel()
{
    tbb::parallel_reduce(m_leafArray.getRange(), *this);
}


template <class TreeType>
void MarchingCubesPointsGenerator<TreeType>::runSerial()
{
    (*this)(m_leafArray.getRange());
}

template <class TreeType>
inline void MarchingCubesPointsGenerator<TreeType>::operator()(const tbb::blocked_range<size_t>& range)
{
    typename GridType::ConstAccessor accessor = m_grid->getConstAccessor();
    openvdb::Coord xyz;
    qreal density[8];

    typename TreeType::LeafNodeType::ValueOnCIter iter;

    for (size_t n = range.begin(); n < range.end(); ++n) {
        iter = m_leafArray.leaf(n).cbeginValueOn();
        for (; iter; ++iter) {
            xyz = iter.getCoord();

            // Sample values at each corner of the voxel
            int cubeIndex = 0;
            openvdb::Coord cornerCoord[8];
            for (unsigned int d = 0; d < 8; ++d) {
                cornerCoord[d] = openvdb::Coord(
                    xyz.x() +  (d & 1),
                    xyz.y() + ((d & 2) >> 1),
                    xyz.z() + ((d & 4) >> 2));
                // inverse sign convention for level sets!
                density[d] = float(accessor.getValue(cornerCoord[d]));
                if(density[d] <= 0.0f ) cubeIndex |= (1 << d);
            }
            openvdb::math::ScaleTranslateMap map;// = *(levelSetGrid->transform().map<openvdb::math::ScaleTranslateMap>());

            openvdb::math::Gradient< openvdb::math::ScaleTranslateMap, openvdb::math::CD_2ND> grad;
            openvdb::math::internal::ReturnValue<openvdb::FloatGrid::ConstAccessor>::Vec3Type n = grad.result(map, accessor, xyz);

            // If there is a crossing, surface this voxel
            if (edgeTable[cubeIndex]) {
                std::vector<openvdb::Vec3f> pointsCurr;
                pointsCurr.reserve(8);
                if(edgeTable[cubeIndex] & 1) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[0], cornerCoord[1], density[0], density[1]))); }
                if(edgeTable[cubeIndex] & 2) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[1], cornerCoord[2], density[1], density[2]))); }
                if(edgeTable[cubeIndex] & 4) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[2], cornerCoord[3], density[2], density[3]))); }
                if(edgeTable[cubeIndex] & 8) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[3], cornerCoord[0], density[3], density[0]))); }
                if(edgeTable[cubeIndex] & 16) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[4], cornerCoord[5], density[4], density[5]))); }
                if(edgeTable[cubeIndex] & 32) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[5], cornerCoord[6], density[5], density[6]))); }
                if(edgeTable[cubeIndex] & 64) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[6], cornerCoord[7], density[6], density[7]))); }
                if(edgeTable[cubeIndex] & 128) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[7], cornerCoord[4], density[7], density[4]))); }
                if(edgeTable[cubeIndex] & 256) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[0], cornerCoord[4], density[0], density[4]))); }
                if(edgeTable[cubeIndex] & 512) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[1], cornerCoord[5], density[1], density[5]))); }
                if(edgeTable[cubeIndex] & 1024) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[2], cornerCoord[6], density[2], density[6]))); }
                if(edgeTable[cubeIndex] & 2048) { pointsCurr.push_back( m_grid->indexToWorld(lerp0(cornerCoord[3], cornerCoord[7], density[3], density[7]))); }
                if(pointsCurr.size() > 1)
                {
                    std::vector<openvdb::Vec3f>::const_iterator iter(pointsCurr.begin());
                    openvdb::Vec3f finalPoint(0.0f,0.0f,0.0f);
                    while( iter != pointsCurr.end() )
                    {
                        finalPoint += *iter;
//                        finalPoint = *iter;
//                        int startIdx = m_points.size();
//                        m_points.resize(startIdx+6);
//                        float* arr = &(m_points.data()[startIdx]);
//                        arr[0] = finalPoint.x()+i;
//                        arr[1] = finalPoint.y();
//                        arr[2] = finalPoint.z();
//                        arr[3] = n.x();
//                        arr[4] = n.y();
//                        arr[5] = n.z();
                        iter++;
                    }
                    int startIdx = m_points.size();
                    m_points.resize(startIdx+6);
                    float* arr = &(m_points.data()[startIdx]);
                    arr[0] = finalPoint.x() / static_cast<float>(pointsCurr.size());
                    arr[1] = finalPoint.y() / static_cast<float>(pointsCurr.size());
                    arr[2] = finalPoint.z() / static_cast<float>(pointsCurr.size());
                    arr[3] = n.x();
                    arr[4] = n.y();
                    arr[5] = n.z();
                }
//                else if(pointsCurr.size() == 1)
//                {
//                    int startIdx = m_points.size();
//                    m_points.resize(startIdx+6);
//                    float* arr = &(m_points.data()[startIdx]);
//                    arr[0] = pointsCurr[0].x();
//                    arr[1] = pointsCurr[0].y();
//                    arr[2] = pointsCurr[0].z();
//                    arr[3] = n.x();
//                    arr[4] = n.y();
//                    arr[5] = n.z();
//                }
            }
        }
    }
}

template <class TreeType>
inline void MarchingCubesPointsGenerator<TreeType>::join(const MarchingCubesPointsGenerator<TreeType>& rhs)
{
    m_points.insert(m_points.end(), rhs.m_points.begin(), rhs.m_points.end());
}

#endif
