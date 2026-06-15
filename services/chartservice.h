#ifndef CHARTSERVICE_H
#define CHARTSERVICE_H

#include <memory>
#include <QMap>
#include <QString>
#include <QDebug>
#include "interfaces/ichart.h"
#include "interfaces/ichartstyle.h"

class ChartService
{
private:
    QMap<QString, std::shared_ptr<IChart>> m_charts;
    std::shared_ptr<IChartStyle> m_currentStyle;
    QString m_currentChartType;

public:
    void registerChart(const QString& dataType, std::shared_ptr<IChart> chart) {
        m_charts[dataType] = chart;
        qDebug() << "ChartService: registered chart for type" << dataType;
    }

    void setStyle(std::shared_ptr<IChartStyle> style) {
        m_currentStyle = style;
    }

    void setChartType(const QString& chartType) {
        m_currentChartType = chartType;
        qDebug() << "ChartService: chart type set to" << chartType;
    }

    std::shared_ptr<QChart> createChart(std::shared_ptr<IData> data) {
        if (!data) {
            qDebug() << "ChartService: data is null";
            return nullptr;
        }

        QString dataType = data->getType();
        qDebug() << "ChartService: data type =" << dataType;
        qDebug() << "ChartService: points count =" << data->getPoints().size();

        QString chartTypeToUse = m_currentChartType.isEmpty() ? dataType : m_currentChartType;
        qDebug() << "ChartService: using chart type =" << chartTypeToUse;

        if (!m_charts.contains(chartTypeToUse)) {
            qDebug() << "ChartService: no chart for type" << chartTypeToUse << ", using 'line'";
            if (!m_charts.contains("line")) {
                qDebug() << "ChartService: no 'line' chart registered!";
                return nullptr;
            }
            chartTypeToUse = "line";
        }

        auto chart = m_charts[chartTypeToUse]->createChart(data);

        if (m_currentStyle && chart) {
            qDebug() << "ChartService: applying style" << m_currentStyle->getName();
            m_currentStyle->applyStyle(chart);
        }

        return chart;
    }

    std::shared_ptr<IChartStyle> getCurrentStyle() const {
        return m_currentStyle;
    }

    QString getCurrentChartType() const {
        return m_currentChartType;
    }
};

#endif
