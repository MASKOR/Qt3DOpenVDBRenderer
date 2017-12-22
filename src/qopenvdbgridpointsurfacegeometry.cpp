#include "qopenvdbgridpointsurfacegeometry.h"
#include "openvdbpointsvertexdatagenerator.h"
#include <QOpenGLFunctions>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/qbufferdatagenerator.h>
#include <QHash>
#include <iomanip>
#include <openvdb/openvdb.h>
#include <QSharedPointer>

class QOpenVDBGridPointSurfaceGeometryPrivate
{
public:
    QOpenVDBGridPointSurfaceGeometryPrivate()
        :m_vertexBuffer(NULL),
         m_grid(NULL),
         m_generationMethod(QOpenVDBGridPointSurfaceGeometry::MarchingCubesCenter)
    {}
    Qt3DRender::QBuffer *m_vertexBuffer;
    QOpenVDBGrid *m_grid;
    QOpenVDBGridPointSurfaceGeometry::PointGenerationMethod m_generationMethod;
};

QOpenVDBGridPointSurfaceGeometry::QOpenVDBGridPointSurfaceGeometry(Qt3DCore::QNode *parent)
    :m_p(new QOpenVDBGridPointSurfaceGeometryPrivate)
{
    openvdb::initialize();
    m_p->m_vertexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, this);
}

QOpenVDBGridPointSurfaceGeometry::~QOpenVDBGridPointSurfaceGeometry()
{
    delete m_p;
}

void QOpenVDBGridPointSurfaceGeometry::updateVertices()
{
    updateAttributes();
    //QMetaObject::invokeMethod(this, "updateAttributes", Qt::QueuedConnection);
    m_p->m_vertexBuffer->setDataGenerator(Qt3DRender::QBufferDataGeneratorPtr(new OpenVDBPointsVertexDataGenerator(this)));
}

QOpenVDBGrid *QOpenVDBGridPointSurfaceGeometry::grid() const
{
    return m_p->m_grid;
}

void QOpenVDBGridPointSurfaceGeometry::setVertexCount(int vertexCount)
{

    QVector<Qt3DRender::QAttribute *> atts = attributes();
    Q_FOREACH(Qt3DRender::QAttribute *attr, atts)
    {
        if(attr->attributeType() == Qt3DRender::QAttribute::VertexAttribute)
        {
            attr->setCount(vertexCount);
        }
    }
}

void QOpenVDBGridPointSurfaceGeometry::updateAttributes()
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

    Qt3DRender::QAttribute* attrib = new Qt3DRender::QAttribute(nullptr);
    attrib->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    attrib->setDataType(Qt3DRender::QAttribute::Float);
    attrib->setDataSize(3);
    attrib->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    attrib->setBuffer(m_p->m_vertexBuffer);
    attrib->setByteStride(sizeof(float)*6);
    attrib->setByteOffset(0);
    //attrib->setCount(1234);
    addAttribute(attrib);
    setBoundingVolumePositionAttribute(attrib);

    attrib = new Qt3DRender::QAttribute(nullptr);
    attrib->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());
    attrib->setDataType(Qt3DRender::QAttribute::Float);
    attrib->setDataSize(3);
    attrib->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    attrib->setBuffer(m_p->m_vertexBuffer);
    attrib->setByteStride(sizeof(float)*6);
    attrib->setByteOffset(sizeof(float)*3);
    //attrib->setCount(1234);
    addAttribute(attrib);
}

void QOpenVDBGridPointSurfaceGeometry::setGrid(QOpenVDBGrid *grid)
{
    if (m_p->m_grid == grid)
        return;

    m_p->m_grid = grid;
    updateVertices();
    Q_EMIT gridChanged(grid);
}

QOpenVDBGridPointSurfaceGeometry::PointGenerationMethod QOpenVDBGridPointSurfaceGeometry::generationMethod() const
{
    return m_p->m_generationMethod;
}

void QOpenVDBGridPointSurfaceGeometry::setGenerationMethod(QOpenVDBGridPointSurfaceGeometry::PointGenerationMethod generationMethod)
{
    if (m_p->m_generationMethod == generationMethod)
        return;

    m_p->m_generationMethod = generationMethod;
    updateVertices();
    Q_EMIT generationMethodChanged(m_p->m_generationMethod);
}
