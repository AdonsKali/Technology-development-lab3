#ifndef LINECHARTADAPTER_H
#define LINECHARTADAPTER_H

#include "interfaces/ichart.h"
#include "interfaces/idata.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QDateTime>
#include <QDebug>

class LineChartAdapter : public IChart
{
public:
    std::shared_ptr<QChart> createChart(std::shared_ptr<IData> data) override {
        auto chart = std::make_shared<QChart>();
        chart->setTitle("Line Chart");
        chart->setAnimationOptions(QChart::SeriesAnimations);

        auto points = data->getPoints();
        if (points.isEmpty()) {
            chart->setTitle("No Data Available");
            return chart;
        }

        qDebug() << "LineChartAdapter: Creating chart with" << points.size() << "points";

        auto series = new QLineSeries();
        chart->addSeries(series);

        bool hasDates = (points.first().x() > 1000000000);
        qDebug() << "LineChartAdapter: hasDates =" << hasDates;

        if (hasDates) {
            for (const QPointF& point : points) {
                QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(point.x()));
                series->append(dt.toMSecsSinceEpoch(), point.y());
            }

            QDateTimeAxis *axisX = new QDateTimeAxis();
            axisX->setTitleText("Date");
            axisX->setFormat("dd.MM.yyyy");
            axisX->setLabelsAngle(-45);

            QDateTime minDate = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(points.first().x()));
            QDateTime maxDate = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(points.last().x()));
            axisX->setRange(minDate, maxDate);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);
        } else {
            for (int i = 0; i < points.size(); ++i) {
                series->append(i, points[i].y());
            }

            QValueAxis *axisX = new QValueAxis();
            axisX->setTitleText("Index");
            axisX->setRange(0, points.size() - 1);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);
        }

        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Value");

        double minY = points[0].y();
        double maxY = points[0].y();
        for (const QPointF& point : points) {
            if (point.y() < minY) minY = point.y();
            if (point.y() > maxY) maxY = point.y();
        }
        double padding = (maxY - minY) * 0.05;
        if (padding == 0) padding = 1.0;
        axisY->setRange(minY - padding, maxY + padding);

        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        QPen pen(Qt::blue);
        pen.setWidth(2);
        series->setPen(pen);
        series->setPointsVisible(false);

        qDebug() << "LineChartAdapter: Chart created with" << series->count() << "points";

        return chart;
    }

    QString getSupportedDataType() const override {
        return "line";
    }
};

#endif
