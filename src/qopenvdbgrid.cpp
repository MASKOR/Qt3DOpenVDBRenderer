#include "qopenvdbgrid.h"
#include "openvdb/openvdb.h"

class QOpenVDBGridPrivate
{
public:
    QOpenVDBGridPrivate(QOpenVDBGrid *p)
        : m_parent(p)
        , m_grid()
        , m_width(0)
        , m_height(1)
        , m_minimum()
        , m_maximum()
        , m_centroid()
        , m_offset()
        , m_dirtyMinMax(true)
        , m_dirtyCentroid(true)
    {}
    QOpenVDBGrid *m_parent;
    std::shared_ptr<openvdb::GridBase> m_grid;

    quint32 m_height;
    quint32 m_width;

    QVector3D m_minimum;
    QVector3D m_maximum;
    QVector3D m_centroid;
    QVector3D m_offset;
    bool m_dirtyMinMax;
    bool m_dirtyCentroid;

    void updateMinMax()
    {

    }
    void updateCentroid()
    {

    }
};

QOpenVDBGrid::QOpenVDBGrid(QObject *parent)
    :QObject(parent),
     m_priv(new QOpenVDBGridPrivate(this))
{
    openvdb::initialize();
}

QOpenVDBGrid::~QOpenVDBGrid()
{
    delete m_priv;
}

void QOpenVDBGrid::updateAttributes()
{

}

quint32 QOpenVDBGrid::height() const
{
    return m_priv->m_height;
}

quint32 QOpenVDBGrid::width() const
{
    return m_priv->m_width;
}

QVector3D QOpenVDBGrid::minimum() const
{
    if(m_priv->m_dirtyMinMax)
    {
        m_priv->updateMinMax();
    }
    return m_priv->m_minimum;
}

QVector3D QOpenVDBGrid::maximum() const
{
    if(m_priv->m_dirtyMinMax)
    {
        m_priv->updateMinMax();
    }
    return m_priv->m_maximum;
}

QVector3D QOpenVDBGrid::centroid() const
{
    if(m_priv->m_dirtyCentroid)
    {
        m_priv->updateCentroid();
    }
    return m_priv->m_centroid;
}

QVector3D QOpenVDBGrid::offset() const
{
    return m_priv->m_offset;
}

std::shared_ptr<openvdb::v4_0_2::GridBase> QOpenVDBGrid::grid() const
{
    return m_priv->m_grid;//std::shared_ptr<openvdb::GridBase>(m_priv->m_grid);
}

void QOpenVDBGrid::setGrid(std::shared_ptr<openvdb::v4_0_2::GridBase> grid)
{
    if (m_priv->m_grid == grid)
        return;
    m_priv->m_grid = grid;
    Q_EMIT gridChanged(grid);
}

void QOpenVDBGrid::setHeight(quint32 height)
{

    if (m_priv->m_height == height)
        return;
    m_priv->m_height = height;
    Q_EMIT heightChanged(height);
}

void QOpenVDBGrid::setWidth(quint32 width)
{

    if (m_priv->m_width == width)
        return;
    m_priv->m_width = width;
    Q_EMIT widthChanged(width);
}
