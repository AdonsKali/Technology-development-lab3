#ifndef LINECHARTADAPTER_H
#define LINECHARTADAPTER_H

#include "interfaces/ichart.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class LineChartAdapter : public IChart
{
public:

    std::shared_ptr<QChart> createChart(std::shared_ptr<IData> data) override {
        auto chart = std::make_shared<QChart>();
        chart->setTitle("Line Chart");

        auto series = new QLineSeries();
        auto points = data->getPoints();

        if (points.isEmpty()) {
            chart->setTitle("No Data Available");
            return chart;
        }
        for (const QPointF& point : points) {
            series->append(point);
        }
        QPen pen(Qt::blue);
        pen.setWidth(2);
        series->setPen(pen);
        series->setPointsVisible(true);
        series->setPointLabelsVisible(false);

        chart->addSeries(series);
        chart->createDefaultAxes();

        if (chart->axisX()) {
            chart->axisX()->setTitleText("Index");
            chart->axisX()->setLabelsVisible(true);
        }

        if (chart->axisY()) {
            chart->axisY()->setTitleText("Value");
            chart->axisY()->setLabelsVisible(true);
        }

        return chart;
    }

    QString getSupportedDataType() const override {
        return "line";
    }
};

#endif 
