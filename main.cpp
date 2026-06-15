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
#include "data/datafactory.h"
#include "interfaces/idatafactory.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto container = std::make_shared<IOCContainer>();

    auto sqlAdapter = std::make_shared<SQLDataAdapter>();
    container->RegisterInstance<SQLDataAdapter>(sqlAdapter);

    auto jsonAdapter = std::make_shared<JSONDataAdapter>();
    container->RegisterInstance<JSONDataAdapter>(jsonAdapter);

    auto dataFactory = std::make_shared<DataFactory>(container);
    container->RegisterInstance<IDataFactory>(dataFactory);

    auto chartService = std::make_shared<ChartService>();
    container->RegisterInstance<ChartService>(chartService);

    container->RegisterInstance<PDFPrinter>(std::make_shared<PDFPrinter>());

    auto lineChart = std::make_shared<LineChartAdapter>();
    container->RegisterInstance<LineChartAdapter>(lineChart);

    auto barChart = std::make_shared<BarChartAdapter>();
    container->RegisterInstance<BarChartAdapter>(barChart);

    container->RegisterInstance<ColorStyle>(std::make_shared<ColorStyle>());

    container->RegisterInstance<GrayscaleStyle>(std::make_shared<GrayscaleStyle>());

    if (chartService) {
        auto lineChart = container->GetObject<LineChartAdapter>();
        auto barChart = container->GetObject<BarChartAdapter>();

        if (lineChart) {
            chartService->registerChart("line", lineChart);
            chartService->registerChart("sqlite", lineChart); 
            chartService->registerChart("json", lineChart);
            qDebug() << "Line chart registered";
        }

        if (barChart) {
            chartService->registerChart("bar", barChart);  
            qDebug() << "Bar chart registered";
        }

        auto colorStyle = container->GetObject<ColorStyle>();
        if (colorStyle) {
            chartService->setStyle(colorStyle);
            qDebug() << "Style set to ColorStyle";
        }
    }


    MainWindow w(container);
    w.show();

    return a.exec();
}
