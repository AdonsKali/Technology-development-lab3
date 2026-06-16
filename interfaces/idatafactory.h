#ifndef IDATAFACTORY_H
#define IDATAFACTORY_H

#include "interfaces/idata.h"
#include <memory>
#include <QString>

class IDataFactory {
public:
    virtual ~IDataFactory() = default;
    virtual std::shared_ptr<IData> create(const QString& filePath) = 0;
    virtual bool supports(const QString& filePath) const = 0;
};

#endif
