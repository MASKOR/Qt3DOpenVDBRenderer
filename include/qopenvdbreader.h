#ifndef QOPENVDBGRIDREADER_H
#define QOPENVDBGRIDREADER_H

#include <QObject>
#include "qopenvdbgrid.h"

class QOpenVDBGridReader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename WRITE setFilename NOTIFY filenameChanged)
    Q_PROPERTY(QOpenVDBGrid *grid READ grid NOTIFY gridChanged) //TODO: may be multiple
public:
    QOpenVDBGridReader();

    QString filename() const;

    QOpenVDBGrid *grid() const;

public Q_SLOTS:
    void setFilename(QString filename);

Q_SIGNALS:
    void filenameChanged(QString filename);
    void gridChanged(QOpenVDBGrid *grid);

private:
    QString m_filename;
    //TODO: these can be multiples
    QOpenVDBGrid *m_grid;
};

#endif
