#ifndef PDFPRINTER_H
#define PDFPRINTER_H

#include <QPrinter>
#include <QPainter>
#include <QDebug>
#include <QFileInfo>
#include <QtCharts/QChartView>

class PDFPrinter
{
private:
    QString m_fileName;

public:
    PDFPrinter() {}

    void setFileName(const QString& fileName) {
        m_fileName = fileName;
    }

    bool print(QChartView* chartView, QPrinter::ColorMode colorMode = QPrinter::Color) {
        if (!chartView) {
            qDebug() << "Invalid chart view";
            return false;
        }

        if (m_fileName.isEmpty()) {
            qDebug() << "No file name specified";
            return false;
        }
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(m_fileName);
        printer.setColorMode(colorMode);
        printer.setPageSize(QPageSize::A4);
        printer.setResolution(300);
        printer.setFullPage(false);

        QPainter painter;
        if (!painter.begin(&printer)) {
            qDebug() << "Failed to start painter on printer";
            return false;
        }
        QRectF pageRect = printer.pageRect(QPrinter::Point);
        QSizeF chartSize = chartView->sizeHint();
        if (chartSize.isEmpty()) {
            chartSize = QSizeF(800, 600);
        }
        qreal scaleX = pageRect.width() / chartSize.width();
        qreal scaleY = pageRect.height() / chartSize.height();
        qreal scale = qMin(scaleX, scaleY) * 0.9;
        qreal translateX = (pageRect.width() - chartSize.width() * scale) / 2;
        qreal translateY = (pageRect.height() - chartSize.height() * scale) / 2;

        painter.translate(translateX, translateY);
        painter.scale(scale, scale);

        chartView->render(&painter);

        painter.end();

        qDebug() << "PDF saved to:" << m_fileName;
        return true;
    }
};

#endif
