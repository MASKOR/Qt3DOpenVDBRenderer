#ifndef QOPENVDBGRID_H
#define QOPENVDBGRID_H

#include <QObject>
#include <QVector3D>
#include <QQmlListProperty>
#include <openvdb/Grid.h>
#include <openvdb/Types.h>


class QOpenVDBGridPrivate;
class QOpenVDBGrid : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint32 height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(quint32 width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QVector3D minimum READ minimum NOTIFY minimumChanged)
    Q_PROPERTY(QVector3D maximum READ maximum NOTIFY maximumChanged)
    Q_PROPERTY(QVector3D centroid READ centroid NOTIFY centroidChanged)
    Q_PROPERTY(QVector3D offset READ offset NOTIFY offsetChanged)
public:
    QOpenVDBGrid(QObject *parent = NULL);
    ////
    /// \brief QOpenVDBGrid takes ownership of the given grid
    ///
    QOpenVDBGrid(openvdb::GridBase::Ptr grid);
    ~QOpenVDBGrid();

    void updateAttributes();

    quint32 height() const;
    quint32 width() const;

    QVector3D minimum() const;
    QVector3D maximum() const;
    QVector3D centroid() const;
    QVector3D offset() const;

    std::shared_ptr<openvdb::GridBase> grid() const;
    void setGrid(std::shared_ptr<openvdb::GridBase> grid);

public Q_SLOTS:
    void setHeight(quint32 height);
    void setWidth(quint32 width);

Q_SIGNALS:
    void heightChanged(quint32 height);
    void widthChanged(quint32 width);
    void minimumChanged(QVector3D minimum);
    void maximumChanged(QVector3D maximum);
    void centroidChanged(QVector3D centroid);
    void offsetChanged(QVector3D offset);
    void gridChanged(std::shared_ptr<openvdb::GridBase> offset);

private:
    QOpenVDBGridPrivate *m_priv;
};



#endif
