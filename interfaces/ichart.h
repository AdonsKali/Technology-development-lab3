#ifndef ICHART_H
#define ICHART_H

#include <QtCharts/QChart>
#include <memory>
#include "idata.h"

class IChart
{
public:
    virtual ~IChart() = default;
    virtual QString getSupportedDataType() const = 0;
    virtual std::shared_ptr<QChart> createChart(std::shared_ptr<IData> data) = 0;
};

#endif
