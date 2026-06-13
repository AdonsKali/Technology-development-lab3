#ifndef CHARTSERVICE_H
#define CHARTSERVICE_H

#include <memory>
#include <QMap>
#include "interfaces/ichart.h"
#include "interfaces/ichartstyle.h"

class ChartService
{
private:
    QMap<QString, std::shared_ptr<IChart>> m_charts;
    std::shared_ptr<IChartStyle> m_currentStyle;
    
public:
    void registerChart(const QString& dataType, std::shared_ptr<IChart> chart) {
        m_charts[dataType] = chart;
    }
    
    void setStyle(std::shared_ptr<IChartStyle> style) {
        m_currentStyle = style;
    }
    
    std::shared_ptr<QChart> createChart(std::shared_ptr<IData> data) {
        if (!data) {
            return nullptr;
        }
        
        QString dataType = data->getType();
        if (!m_charts.contains(dataType)) {
            if (!m_charts.contains("line")) {
                return nullptr;
            }
            dataType = "line";
        }
        
        auto chart = m_charts[dataType]->createChart(data);
        if (m_currentStyle && chart) {
            m_currentStyle->applyStyle(chart);
        }
        
        return chart;
    }
    
    std::shared_ptr<IChartStyle> getCurrentStyle() const {
        return m_currentStyle;
    }
};

#endif
