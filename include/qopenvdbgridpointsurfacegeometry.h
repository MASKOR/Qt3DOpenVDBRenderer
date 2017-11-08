#ifndef QOPENVDBGRIDPOINTSURFACEGEOMETRY_H
#define QOPENVDBGRIDPOINTSURFACEGEOMETRY_H

#include <Qt3DRender/qgeometry.h>
#include "qopenvdbgrid.h"
#include <openvdb/Grid.h>

class QOpenVDBGridPointSurfaceGeometryPrivate;
class QOpenVDBGridPointSurfaceGeometry : public Qt3DRender::QGeometry
{
    Q_OBJECT
    Q_ENUMS(PointGenerationMethod)

    Q_PROPERTY(QOpenVDBGrid *grid READ grid WRITE setGrid NOTIFY gridChanged)
    Q_PROPERTY(PointGenerationMethod generationMethod READ generationMethod WRITE setGenerationMethod NOTIFY generationMethodChanged)
public:
    enum PointGenerationMethod {
        MarchingCubesCenter,
        VoxelCenter,
        Surrounding
    };
    explicit QOpenVDBGridPointSurfaceGeometry(QNode *parent = NULL);
    ~QOpenVDBGridPointSurfaceGeometry();
    void updateVertices();

    QOpenVDBGrid *grid() const;
    // For datagenerator (invoke from other thread)
    Q_INVOKABLE void setVertexCount(int vertexCount);

    PointGenerationMethod generationMethod() const;

public Q_SLOTS:
    void setGrid(QOpenVDBGrid *grid);

    void setGenerationMethod(PointGenerationMethod generationMethod);

private Q_SLOTS:
    void updateAttributes();
Q_SIGNALS:
    void gridChanged(QOpenVDBGrid *grid);

    void generationMethodChanged(PointGenerationMethod generationMethod);

private:
    QOpenVDBGridPointSurfaceGeometryPrivate *m_p;
};


#endif
