#ifndef QOPENVDBGRIDGEOMETRY_H
#define QOPENVDBGRIDGEOMETRY_H

#include <Qt3DRender/qgeometry.h>
#include "qopenvdbgrid.h"
#include <openvdb/Grid.h>

class QOpenVDBGridGeometryPrivate;
class QOpenVDBGridGeometry : public Qt3DRender::QGeometry
{
    Q_OBJECT
    Q_PROPERTY(QOpenVDBGrid *grid READ grid WRITE setGrid NOTIFY gridChanged)

public:
    explicit QOpenVDBGridGeometry(QNode *parent = NULL);
    ~QOpenVDBGridGeometry();
    void updateVertices();

    QOpenVDBGrid *grid() const;


public Q_SLOTS:
    void setGrid(QOpenVDBGrid *grid);
private Q_SLOTS:
    void updateAttributes();
Q_SIGNALS:
    void gridChanged(QOpenVDBGrid *grid);

private:
    QOpenVDBGridGeometryPrivate *m_p;
};


#endif
