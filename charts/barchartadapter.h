#ifndef BARCHARTADAPTER_H
#define BARCHARTADAPTER_H

#include "interfaces/ichart.h"
#include "interfaces/idata.h"
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QDateTime>
#include <QDebug>
#include <cmath>

class BarChartAdapter : public IChart
{
public:
    QString getSupportedDataType() const override {
        return "bar";
    }

    std::shared_ptr<QChart> createChart(std::shared_ptr<IData> data) override {
        auto chart = std::make_shared<QChart>();
        chart->setTitle("Bar Chart");

        auto points = data->getPoints();
        if (points.isEmpty()) {
            chart->setTitle("No Data Available");
            return chart;
        }

        qDebug() << "BarChartAdapter: Creating chart with" << points.size() << "points";

        auto series = new QBarSeries();
        auto barSet = new QBarSet("Values");

        bool hasDates = (points.first().x() > 1000000000);
        int maxBars = 100;
        int step = qMax(1, points.size() / maxBars);

        QStringList categories;

        for (int i = 0; i < points.size(); i += step) {
            double sum = 0;
            int count = 0;
            for (int j = i; j < qMin(i + step, points.size()); ++j) {
                sum += points[j].y();
                count++;
            }
            double avgValue = sum / count;
            *barSet << avgValue;

            if (hasDates) {
                QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(points[i].x()));
                categories << dt.toString("dd.MM");
            } else {
                categories << QString::number(i + 1);
            }
        }

        qDebug() << "BarChartAdapter: Created" << barSet->count() << "bars";

        series->append(barSet);

        if (barSet->count() > 50) {
            series->setBarWidth(0.3);
        } else if (barSet->count() > 20) {
            series->setBarWidth(0.5);
        } else {
            series->setBarWidth(0.8);
        }

        chart->addSeries(series);
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        axisX->setTitleText(hasDates ? "Date" : "Index");
        if (categories.size() > 10) {
            axisX->setLabelsAngle(-45);
        }
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Value");

        double minY = 0;
        double maxY = 0;
        for (int i = 0; i < barSet->count(); ++i) {
            double val = barSet->at(i);
            if (val < minY) minY = val;
            if (val > maxY) maxY = val;
        }
        double padding = (maxY - minY) * 0.1;
        if (padding == 0) padding = 1.0;
        axisY->setRange(minY - padding, maxY + padding);

        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        return chart;
    }
};

#endif
