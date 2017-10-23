#include "qopenvdbreader.h"
#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>
#include <QDebug>

QOpenVDBGridReader::QOpenVDBGridReader()
    :m_grid(new QOpenVDBGrid())
{
    openvdb::initialize();
}

QString QOpenVDBGridReader::filename() const
{
    return m_filename;
}

QOpenVDBGrid *QOpenVDBGridReader::grid() const
{
    return m_grid;
}

void QOpenVDBGridReader::setFilename(QString filename)
{
    if (m_filename == filename)
        return;

    Q_ASSERT(m_grid);

    openvdb::io::File file(filename.toStdString());
    // Open the file.  This reads the file header, but not any grids.
    file.open();
    // Loop over all grids in the file and retrieve a shared pointer
    // to the one named "LevelSetSphere".  (This can also be done
    // more simply by calling file.readGrid("LevelSetSphere").)

    openvdb::GridBase::Ptr baseGrid;
    for (openvdb::io::File::NameIterator nameIter = file.beginName();
        nameIter != file.endName(); ++nameIter)
    {
        // Read in only the grid we are interested in. TODO: read all
        if (m_grid->grid() == nullptr) {
            baseGrid = file.readGrid(nameIter.gridName());
            break;
        } else {
            std::cout << "skipping grid " << nameIter.gridName() << std::endl;
        }
    }
    m_grid->setGrid(baseGrid);
    file.close();

    qDebug() << "Read Grid" << filename;
    m_filename = filename;
    Q_EMIT filenameChanged(filename);
    Q_EMIT gridChanged(m_grid);
}
