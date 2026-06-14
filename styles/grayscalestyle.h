#ifndef GRAYSCALESTYLE_H
#define GRAYSCALESTYLE_H
#include "interfaces/ichartstyle.h"
#include <QtCharts/QAbstractSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QBarSet>

class GrayscaleStyle : public IChartStyle
{
public:
    void applyStyle(std::shared_ptr<QChart> chart) override {
        if (!chart) return;

        chart->setTheme(QChart::ChartThemeDark);
        chart->setAnimationOptions(QChart::NoAnimation);
        QPalette palette = chart->palette();
        palette.setColor(QPalette::Window, QColor(240, 240, 240));
        palette.setColor(QPalette::WindowText, Qt::black);
        chart->setPalette(palette);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);

        QList<QColor> grayColors;
        grayColors << QColor(50, 50, 50)   
                   << QColor(100, 100, 100) 
                   << QColor(150, 150, 150) 
                   << QColor(200, 200, 200);

        int colorIndex = 0;
        for (auto series : chart->series()) {
            if (colorIndex < grayColors.size()) {
                QPen pen(grayColors[colorIndex]);
                pen.setWidth(2);
                if (auto lineSeries = qobject_cast<QLineSeries*>(series)) {
                    lineSeries->setPen(pen);
                } else if (auto scatterSeries = qobject_cast<QScatterSeries*>(series)) {
                    scatterSeries->setColor(grayColors[colorIndex]);
                    scatterSeries->setMarkerSize(8);
                } else if (auto barSeries = qobject_cast<QBarSeries*>(series)) {
                    for (auto barSet : barSeries->barSets()) {
                        barSet->setColor(grayColors[colorIndex]);
                        barSet->setBorderColor(grayColors[colorIndex]);
                    }
                }
            }
            colorIndex++;
        }
    }

    QPrinter::ColorMode getColorMode() const override {
        return QPrinter::GrayScale;
    }

    QString getName() const override {
        return "Grayscale Style";
    }
};

#endif
