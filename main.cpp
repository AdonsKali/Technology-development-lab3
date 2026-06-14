#include "mainwindow.h"
#include "ioc/container.h"
#include "services/chartservice.h"
#include "data/sqldataadapter.h"
#include "data/jsondataadapter.h"
#include "charts/linechartadapter.h"
#include "charts/barchartadapter.h"
#include "styles/colorstyle.h"
#include "styles/grayscalestyle.h"
#include "services/pdfprinter.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    IOCContainer container;
    container.RegisterInstance<ChartService>(std::make_shared<ChartService>());

    container.RegisterInstance<IData, SQLDataAdapter>();
    container.RegisterInstance<IData, JSONDataAdapter>();

    container.RegisterInstance<IChart, LineChartAdapter>();
    container.RegisterInstance<IChart, BarChartAdapter>();

    container.RegisterInstance<IChartStyle, ColorStyle>();
    container.RegisterInstance<IChartStyle, GrayscaleStyle>();

    container.RegisterInstance<PDFPrinter>(std::make_shared<PDFPrinter>());

    auto chartService = container.GetObject<ChartService>();
    if (chartService) {
        chartService->registerChart("line", container.GetObject<LineChartAdapter>());
        chartService->registerChart("bar", container.GetObject<BarChartAdapter>());
        chartService->registerChart("sqlite", container.GetObject<LineChartAdapter>());
        chartService->registerChart("json", container.GetObject<LineChartAdapter>());
        chartService->setStyle(container.GetObject<ColorStyle>());
        qDebug() << "ChartService configured successfully";
    } else {
        qDebug() << "Failed to get ChartService";
        return 1;
    }
    MainWindow w;
    w.show();

    return a.exec();
}
