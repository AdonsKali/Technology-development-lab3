#ifndef BARCHARTADAPTER_H
#define BARCHARTADAPTER_H

#include "interfaces/ichart.h"
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

class BarChartAdapter : public IChart
{
public:
    QString getSupportedDataType() const override {
        return "bar";
    }

    std::shared_ptr<QChart> createChart(std::shared_ptr<IData> data) override {
        auto chart = std::make_shared<QChart>();
        chart->setTitle("Bar Chart");

        auto series = new QBarSeries();
        auto barSet = new QBarSet("Values");

        auto points = data->getPoints();
        if (points.isEmpty()) {
            chart->setTitle("No Data Available");
            return chart;
        }

        QStringList categories;
        for (int i = 0; i < points.size(); ++i) {
            *barSet << points[i].y();
            categories << QString::number(i);
        }

        barSet->setColor(Qt::blue);
        barSet->setBorderColor(Qt::darkBlue);

        series->append(barSet);
        series->setBarWidth(0.8);

        chart->addSeries(series);

        auto axisX = new QBarCategoryAxis();
        axisX->append(categories);
        axisX->setTitleText("Index");
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        auto axisY = new QValueAxis();
        axisY->setTitleText("Value");
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        return chart;
    }
};

#endif 
