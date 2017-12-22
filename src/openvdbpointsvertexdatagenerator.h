#ifndef OVDBPVCVDG
#define OVDBPVCVDG

#include "qopenvdbgridpointsurfacegeometry.h"
#include "openvdbgenmethod_marchingcubescenter.h"
#include "openvdbgenmethod_surrounding.h"
#include "openvdbgenmethod_voxelcenter.h"
#include <openvdb/tools/LevelSetUtil.h>
#include <QByteArray>
#include <Qt3DRender/QBufferDataGenerator>
#include <openvdb/openvdb.h>

class OpenVDBPointsVertexDataGenerator : public Qt3DRender::QBufferDataGenerator /*Qt3DRender::QBufferFunctor*/
{
public:
    OpenVDBPointsVertexDataGenerator(QOpenVDBGridPointSurfaceGeometry *geometry)
        : m_geometry(geometry)
    {
    }

    QByteArray operator ()() Q_DECL_OVERRIDE
    {
        openvdb::FloatGrid::ConstPtr levelSetGrid = openvdb::gridConstPtrCast<openvdb::FloatGrid>(m_geometry->grid()->grid());
        openvdb::CoordBBox bbox;

        // Get min & max and checks if the grid is empty
        if (levelSetGrid->tree().evalLeafBoundingBox(bbox)) {

            openvdb::tree::LeafManager<const openvdb::FloatGrid::TreeType> leafs(levelSetGrid->tree());

            QByteArray result;
            switch(m_geometry->generationMethod())
            {
            case QOpenVDBGridPointSurfaceGeometry::PointGenerationMethod::MarchingCubesCenter:
                {
                    CenterPointsGenerator<openvdb::FloatTree> generator(leafs, levelSetGrid);
                    generator.runParallel();
                    const std::vector<float> &posnorm = generator.points();
                    result.append(reinterpret_cast<const char*>(posnorm.data()), sizeof(float)*posnorm.size());
                }
                break;
            case QOpenVDBGridPointSurfaceGeometry::PointGenerationMethod::VoxelCenter:
                {
                    CenterPointsGenerator<openvdb::FloatTree> generator(leafs, levelSetGrid);
                    generator.runParallel();
                    const std::vector<float> &posnorm = generator.points();
                    result.append(reinterpret_cast<const char*>(posnorm.data()), sizeof(float)*posnorm.size());
                }
                break;
            case QOpenVDBGridPointSurfaceGeometry::PointGenerationMethod::Surrounding:
                {
                    const openvdb::FloatGrid::TreeType& tree = levelSetGrid->tree();
                    const openvdb::Index64 maxVoxelPoints = 26000000;
//                    openvdb::Vec3s colorMap[4];
//                    colorMap[0] = openvdb::Vec3s(0.3, 0.9, 0.3); // green
//                    colorMap[1] = openvdb::Vec3s(0.9, 0.3, 0.3); // red
//                    colorMap[2] = openvdb::Vec3s(0.9, 0.9, 0.3); // yellow
//                    colorMap[3] = openvdb::Vec3s(0.3, 0.3, 0.9); // blue

                    openvdb::Index64 voxelsPerLeaf = openvdb::FloatGrid::TreeType::LeafNodeType::NUM_VOXELS;

                    // Level set rendering
                    if (tree.activeLeafVoxelCount() > maxVoxelPoints) {
                        voxelsPerLeaf = std::max<openvdb::Index64>(1, (maxVoxelPoints / tree.leafCount()));
                    }

                    openvdb::FloatGrid::ValueType minValue, maxValue;
                    openvdb::tree::LeafManager<const openvdb::FloatGrid::TreeType> leafs(tree);

//                    {
//                        openvdb::tools::MinMaxVoxel<const openvdb::FloatGrid::TreeType> minmax(leafs);
//                        minmax.runParallel();
//                        minValue = minmax.minVoxel();
//                        maxValue = minmax.maxVoxel();
//                    }

                    std::vector<size_t> indexMap(leafs.leafCount());
                    size_t voxelCount = 0;
                    for (openvdb::Index64 l = 0, L = leafs.leafCount(); l < L; ++l) {
                        indexMap[l] = voxelCount;
                        voxelCount += std::min(leafs.leaf(l).onVoxelCount(), voxelsPerLeaf);
                    }

                    std::vector<GLfloat>
                        points(voxelCount * 3),
                        //colors(voxelCount * 3),
                        normals(voxelCount * 3);
                    std::vector<GLuint> indices(voxelCount);

                    PointGenerator<const openvdb::FloatGrid::TreeType> pointGen(
                        points, indices, leafs, indexMap, levelSetGrid->transform(), voxelsPerLeaf);
                    pointGen.runParallel();
                    //TO DO: generate color for surrounding voxel points
//                    PointAttributeGenerator<openvdb::FloatGrid> attributeGen(
//                        points, colors, normals, *levelSetGrid, minValue, maxValue, colorMap, true);
                    PointAttributeGenerator<openvdb::FloatGrid> attributeGen(
                        points, normals, *levelSetGrid, true);
                    attributeGen.runParallel();
                    std::vector<GLfloat>::const_iterator iterPos(points.cbegin());
                    std::vector<GLfloat>::const_iterator iterNorm(normals.cbegin());
                    int idx = 0;

                    std::vector<float> posnorm;
                    while( iterPos != points.end() )
                    {
                        posnorm.push_back(*iterPos); iterPos++;
                        posnorm.push_back(*iterPos); iterPos++;
                        posnorm.push_back(*iterPos); iterPos++;
                        posnorm.push_back(*iterNorm); iterNorm++;
                        posnorm.push_back(*iterNorm); iterNorm++;
                        posnorm.push_back(*iterNorm); iterNorm++;
                        //TO DO: add color
                        idx++;
                    }
                    result.append(reinterpret_cast<const char*>(posnorm.data()), sizeof(float)*posnorm.size());
                }
                break;
            }
            //Note: setting vertexCount from here is not optimal. Doing so directly causes trouble.
            // I'm not sure if this is the correct relationship between generator and geometry.
            // A direct method call will cause trouble when multiple GeometryRenderers are used.
            int vertexCount = result.size()/sizeof(float)/6;
            QMetaObject::invokeMethod(m_geometry, "setVertexCount", Qt::QueuedConnection, Q_ARG(int, vertexCount) );

            return result;
        }
        return QByteArray();
    }

    bool operator ==(const Qt3DRender::QBufferDataGenerator &other) const Q_DECL_OVERRIDE
    {
        const OpenVDBPointsVertexDataGenerator *otherFunctor = Qt3DRender::functor_cast<OpenVDBPointsVertexDataGenerator>(&other);
        if (otherFunctor != NULL)
            return false;// otherFunctor->m_geometry == m_geometry;
        return false;
    }

    QT3D_FUNCTOR(OpenVDBPointsVertexDataGenerator)

private:
    QOpenVDBGridPointSurfaceGeometry *m_geometry;
};

#endif
