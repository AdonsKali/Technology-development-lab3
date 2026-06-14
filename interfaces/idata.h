#ifndef IDATA_H
#define IDATA_H

#include <QString>
#include <QPointF>
#include <QVector>

class IData
{
public:
    virtual ~IData() = default;
    virtual QString getType() const = 0;
    virtual QVector<QPointF> getPoints() const = 0;
    virtual bool load(const QString& source) = 0;
};

#endif
