#ifndef ICHARTSTYLE_H
#define ICHARTSTYLE_H

#include <QtCharts/QChart>
#include <QPrinter>
#include <memory>

class IChartStyle
{
public:
    virtual ~IChartStyle() = default;
    virtual void applyStyle(std::shared_ptr<QChart> chart) = 0;
    virtual QPrinter::ColorMode getColorMode() const = 0;
    virtual QString getName() const = 0;
};

#endif
