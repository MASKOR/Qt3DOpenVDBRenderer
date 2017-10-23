#include "qopenvdbgridgeometry.h"
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/qbufferdatagenerator.h>
#include <QHash>
#include <iomanip>
#include <openvdb/openvdb.h>
#include <openvdb/io/Stream.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetRebuild.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/PointScatter.h>
#include <openvdb/tree/LeafManager.h>
#include <openvdb/Grid.h>
#include <QSharedPointer>

class QOpenVDBGridGeometryPrivate
{
public:
    QOpenVDBGridGeometryPrivate()
        :m_vertexBuffer(NULL),
         m_grid(NULL){}
    Qt3DRender::QBuffer *m_vertexBuffer;
    QOpenVDBGrid *m_grid;
};

QOpenVDBGridGeometry::QOpenVDBGridGeometry(Qt3DCore::QNode *parent)
    :m_p(new QOpenVDBGridGeometryPrivate)
{
    openvdb::initialize();
    m_p->m_vertexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, this);
}

QOpenVDBGridGeometry::~QOpenVDBGridGeometry()
{
    delete m_p;
}

//TO DO: support not only LevelSets/FloatGrids, but also "Fog" or MRT like Volume Rendering.
//Qt3DRender::QAttribute::VertexBaseType pclTypeToAttributeType(const std::string &inp)
//{
//    //TODO: openvdb::GridBase::type() ?
//    switch(inp)
//    {
//    case QPointfield::INT8:
//        return Qt3DRender::QAttribute::Byte;
//    case QPointfield::INT16:
//        return Qt3DRender::QAttribute::Short;
//    case QPointfield::INT32:
//        return Qt3DRender::QAttribute::Int;
//    case QPointfield::UINT8:
//        return Qt3DRender::QAttribute::UnsignedByte;
//    case QPointfield::UINT16:
//        return Qt3DRender::QAttribute::UnsignedShort;
//    case QPointfield::UINT32:
//        return Qt3DRender::QAttribute::UnsignedInt;
//    case QPointfield::FLOAT32:
//        return Qt3DRender::QAttribute::Float;
//    case QPointfield::FLOAT64:
//        return Qt3DRender::QAttribute::Double;
//    default:
//        Q_ASSERT(false);
//        return Qt3DRender::QAttribute::Float;
//    }
//}

void QOpenVDBGridGeometry::updateVertices()
{
    updateAttributes();
    //QMetaObject::invokeMethod(this, "updateAttributes", Qt::QueuedConnection);
    m_p->m_vertexBuffer->setDataGenerator(Qt3DRender::QBufferDataGeneratorPtr(new OpenVDBMeshDataGenerator(this)));
}

QOpenVDBGrid *QOpenVDBGridGeometry::grid() const
{
    return m_p->m_grid;
}

void QOpenVDBGridGeometry::updateAttributes()
{
    // completely rebuild attribute list and remove all previous attributes
    QVector<Qt3DRender::QAttribute *> atts = attributes();
    Q_FOREACH(Qt3DRender::QAttribute *attr, atts)
    {
        if(attr->attributeType() == Qt3DRender::QAttribute::VertexAttribute)
        {
            removeAttribute(attr);
            attr->deleteLater();
        }
        else
        {
            qDebug() << "skipped index";
        }
    }

    m_p->m_grid->updateAttributes();

    Qt3DRender::QAttribute* attrib = new Qt3DRender::QAttribute(this);
    attrib->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    attrib->setDataType(Qt3DRender::QAttribute::Float);
    attrib->setDataSize(3);
    attrib->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    attrib->setBuffer(m_p->m_vertexBuffer);
    attrib->setByteStride(sizeof(float)*6);
    attrib->setByteOffset(0);
    //attrib->setCount(1369734);
    addAttribute(attrib);
    setBoundingVolumePositionAttribute(attrib);

    attrib = new Qt3DRender::QAttribute(this);
    attrib->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());
    attrib->setDataType(Qt3DRender::QAttribute::Float);
    attrib->setDataSize(3);
    attrib->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    attrib->setBuffer(m_p->m_vertexBuffer);
    attrib->setByteStride(sizeof(float)*6);
    attrib->setByteOffset(sizeof(float)*3);
    //attrib->setCount(1369734);
    addAttribute(attrib);
}

void QOpenVDBGridGeometry::setGrid(QOpenVDBGrid *grid)
{
    if (m_p->m_grid == grid)
        return;

    m_p->m_grid = grid;
    updateVertices();
    Q_EMIT gridChanged(grid);
}
