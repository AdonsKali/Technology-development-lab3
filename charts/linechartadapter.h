
#ifndef LINECHARTADAPTER_H
#define LINECHARTADAPTER_H

#include "interfaces/ichart.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class LineChartAdapter : public IChart
{
public:
    QString getSupportedDataType() const override {
        return "line";
    }

    std::shared_ptr<QChart> createChart(std::shared_ptr<IData> data) override {
        auto chart = std::make_shared<QChart>();
        chart->setTitle("Line Chart");

        auto series = new QLineSeries();
        auto points = data->getPoints();

        if (points.isEmpty()) {
            chart->setTitle("No Data Available");
            return chart;
        }

        // Добавляем точки в серию
        for (const QPointF& point : points) {
            series->append(point);
        }

        // Настройка внешнего вида линии
        QPen pen(Qt::blue);
        pen.setWidth(2);
        series->setPen(pen);

        // Включаем отображение точек
        series->setPointsVisible(true);
        series->setPointLabelsVisible(false);

        chart->addSeries(series);
        chart->createDefaultAxes();

        // Настраиваем оси
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
};

#endif // LINECHARTADAPTER_H
