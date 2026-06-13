#ifndef COLORSTYLE_H
#define COLORSTYLE_H

#include "interfaces/ichartstyle.h"

class ColorStyle : public IChartStyle
{
public:
    void applyStyle(std::shared_ptr<QChart> chart) override {
        if (!chart) return;
        
        chart->setTheme(QChart::ChartThemeLight);
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setBackgroundBrush(QBrush(Qt::white));
        chart->setTitleBrush(QBrush(Qt::black));
        
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->legend()->setBrush(QBrush(Qt::black));
    }
    
    QPrinter::ColorMode getColorMode() const override {
        return QPrinter::Color;
    }
    
    QString getName() const override {
        return "Color Style";
    }
};

#endif
