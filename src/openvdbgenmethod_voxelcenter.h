#ifndef OPENVDBGENMETHOD_VOXELCENTER_H
#define OPENVDBGENMETHOD_VOXELCENTER_H

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
class CenterPointsGenerator
{
public:
    using GridType = openvdb::Grid<TreeType>;
    using GridTypeConstPtr = typename GridType::ConstPtr;
    typedef openvdb::tree::LeafManager<const TreeType> LeafArray;
    typedef typename TreeType::ValueType ValueType;

    CenterPointsGenerator(LeafArray&, GridTypeConstPtr);

    void runParallel();
    void runSerial();

    const std::vector<ValueType>& points() const { return m_points; }

    inline CenterPointsGenerator(const CenterPointsGenerator<TreeType>&, tbb::split);
    inline void operator()(const tbb::blocked_range<size_t>&);
    inline void join(const CenterPointsGenerator<TreeType>&);

private:
    LeafArray& m_leafArray;
    GridTypeConstPtr m_grid;
    std::vector<ValueType> m_points;
};


template <class TreeType>
CenterPointsGenerator<TreeType>::CenterPointsGenerator(LeafArray& leafs, GridTypeConstPtr grid)
    : m_leafArray(leafs)
    , m_grid(grid)
    , m_points()
{
}


template <class TreeType>
inline CenterPointsGenerator<TreeType>::CenterPointsGenerator(const CenterPointsGenerator<TreeType>& rhs, tbb::split)
    : m_leafArray(rhs.m_leafArray)
    , m_grid(rhs.m_grid)
    , m_points()
{
}


template <class TreeType>
void CenterPointsGenerator<TreeType>::runParallel()
{
    tbb::parallel_reduce(m_leafArray.getRange(), *this);
}


template <class TreeType>
void CenterPointsGenerator<TreeType>::runSerial()
{
    (*this)(m_leafArray.getRange());
}

template <class TreeType>
inline void CenterPointsGenerator<TreeType>::operator()(const tbb::blocked_range<size_t>& range)
{
    typename GridType::ConstAccessor accessor = m_grid->getConstAccessor();
    openvdb::Coord xyz;
    qreal density[8];

    typename TreeType::LeafNodeType::ValueOnCIter iter;

    for (size_t n = range.begin(); n < range.end(); ++n) {
        iter = m_leafArray.leaf(n).cbeginValueOn();
        for (; iter; ++iter) {
            xyz = iter.getCoord();
            bool isLess = false, isMore = false;

            // Sample values at each corner of the voxel
            for (unsigned int d = 0; d < 8; ++d) {
                openvdb::Coord valueCoord(
                    xyz.x() +  (d & 1),
                    xyz.y() + ((d & 2) >> 1),
                    xyz.z() + ((d & 4) >> 2));

                // inverse sign convention for level sets!
                density[d] = float(accessor.getValue(valueCoord));
                density[d] <= 0.0f ? isLess = true : isMore = true;
            }

            openvdb::math::ScaleTranslateMap map;// = *(levelSetGrid->transform().map<openvdb::math::ScaleTranslateMap>());

            openvdb::math::Gradient< openvdb::math::ScaleTranslateMap, openvdb::math::CD_2ND> grad;
            openvdb::math::internal::ReturnValue<openvdb::FloatGrid::ConstAccessor>::Vec3Type n = grad.result(map, accessor, xyz);

            // If there is a crossing, surface this voxel
            if (isLess && isMore) {
                openvdb::Vec3d wPos = m_grid->indexToWorld(openvdb::Vec3d(xyz.x()+0.5, xyz.y()+0.5, xyz.z()+0.5));
                int startIdx = m_points.size();
                m_points.resize(startIdx+6);
                float* arr = &(m_points.data()[startIdx]);
                arr[0] = wPos.x();
                arr[1] = wPos.y();
                arr[2] = wPos.z();
                arr[3] = n.x();
                arr[4] = n.y();
                arr[5] = n.z();
            }
        }
    }
}

template <class TreeType>
inline void CenterPointsGenerator<TreeType>::join(const CenterPointsGenerator<TreeType>& rhs)
{
    m_points.insert(m_points.end(), rhs.m_points.begin(), rhs.m_points.end());
}

#endif
