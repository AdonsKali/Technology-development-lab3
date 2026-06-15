#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QTableView>
#include <QHeaderView>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QDebug>
#include <QApplication>
#include <QPrinter>
#include <QPainter>
#include <QCategoryAxis>
#include <QDateTimeAxis>
#include "data/sqldataadapter.h"
#include "data/jsondataadapter.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isChartDisplayed(false)
{
    ui->setupUi(this);

    this->setGeometry(100, 100, 1600, 800);
    this->setStatusBar(new QStatusBar(this));
    this->statusBar()->showMessage("Выбранный путь : ");

    QString homePath = QDir::homePath();

    leftPartModel = new QFileSystemModel(this);
    leftPartModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    leftPartModel->setRootPath(homePath);

    rightPartModel = new QFileSystemModel(this);
    rightPartModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    rightPartModel->setNameFilters(QStringList() << "*.db" << "*.sqlite" << "*.json");
    rightPartModel->setNameFilterDisables(false);
    rightPartModel->setRootPath(homePath);

    treeView = new QTreeView();
    treeView->setModel(leftPartModel);
    treeView->setHeaderHidden(true);
    treeView->setRootIndex(leftPartModel->index(homePath));
    treeView->setMinimumWidth(250);
    treeView->expandAll();
    treeView->header()->resizeSection(0, 200);

    tableView = new QTableView();
    tableView->setModel(rightPartModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setMinimumWidth(350);

    tableView->setColumnWidth(0, 200);
    tableView->setColumnWidth(1, 80);
    tableView->setColumnWidth(2, 100);

    rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(tableView);

    setupChartArea();
    rightSplitter->addWidget(chartPanel);
    rightSplitter->setStretchFactor(0, 1);
    rightSplitter->setStretchFactor(1, 2);

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(treeView);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 3);

    setCentralWidget(mainSplitter);

    QItemSelectionModel *selectionModel = treeView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &MainWindow::on_selectionChangedSlot);

    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onFileSelected);

    QModelIndex topLeft = leftPartModel->index(homePath);
    QItemSelection toggleSelection;
    toggleSelection.select(topLeft, topLeft);
    selectionModel->select(toggleSelection, QItemSelectionModel::Toggle);
}

void MainWindow::setupChartArea()
{

    chartPanel = new QWidget(this);
    chartPanelLayout = new QVBoxLayout(chartPanel);
    chartPanelLayout->setContentsMargins(5, 5, 5, 5);
    chartControlsLayout = new QHBoxLayout();
    chartControlsLayout->setContentsMargins(5, 5, 5, 5);
    QLabel* chartTypeLabel = new QLabel("Тип диаграммы:", this);
    chartTypeCombo = new QComboBox(this);
    chartTypeCombo->addItem("Line Chart");
    chartTypeCombo->addItem("Bar Chart");
    grayscaleCheckBox = new QCheckBox("Черно-белый график", this);
    printButton = new QPushButton("Печать графика", this);
    chartControlsLayout->addWidget(chartTypeLabel);
    chartControlsLayout->addWidget(chartTypeCombo);
    chartControlsLayout->addStretch();
    chartControlsLayout->addWidget(grayscaleCheckBox);
    chartControlsLayout->addWidget(printButton);
    chartLayout = new QVBoxLayout();
    chartLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* placeholder = new QLabel(this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("QLabel { color: #666; font: 10pt; background-color: #fafafa; border: 1px solid #ddd; border-radius: 5px; padding: 20px; }");
    chartLayout->addWidget(placeholder);
    chartPanelLayout->addLayout(chartControlsLayout);
    chartPanelLayout->addLayout(chartLayout);

    connect(chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onChartTypeChanged);
    connect(grayscaleCheckBox, &QCheckBox::toggled,
            this, &MainWindow::onGrayscaleToggled);
    connect(printButton, &QPushButton::clicked,
            this, &MainWindow::onPrintToPDF);
}

void MainWindow::on_selectionChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    QModelIndexList indexes = selected.indexes();
    if (indexes.count() >= 1) {
        QModelIndex ix = indexes.constFirst();
        QString filePath = leftPartModel->filePath(ix);
        this->statusBar()->showMessage("Выбранный путь : " + filePath);

        tableView->setRootIndex(rightPartModel->setRootPath(filePath));
    }
}

void MainWindow::onFileSelected(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    QModelIndexList indexes = selected.indexes();
    if (indexes.isEmpty()) return;
    QModelIndex ix = indexes.first();
    if (!ix.isValid()) return;

    QString filePath = rightPartModel->filePath(ix);

    if (filePath.endsWith(".db") || filePath.endsWith(".sqlite") || filePath.endsWith(".json")) {
        qDebug() << "Selected file:" << filePath;
        statusBar()->showMessage("Загрузка: " + QFileInfo(filePath).fileName());
        loadAndDisplayChart(filePath);
    }
}

void MainWindow::loadAndDisplayChart(const QString& filePath)
{
    m_currentFilePath = filePath;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    try {
        std::shared_ptr<IData> data;

        if (filePath.endsWith(".db") || filePath.endsWith(".sqlite")) {
            data = std::make_shared<SQLDataAdapter>();
            qDebug() << "Loading SQLite database...";
        } else if (filePath.endsWith(".json")) {
            data = std::make_shared<JSONDataAdapter>();
            qDebug() << "Loading JSON file...";
        } else {
            QApplication::restoreOverrideCursor();
            showErrorMessage("Error", "Unsupported file format");
            return;
        }

        if (!data->load(filePath)) {
            QApplication::restoreOverrideCursor();
            showErrorMessage("Error", "Failed to load data from file");
            return;
        }

        m_currentData = data->getPoints();

        if (m_currentData.isEmpty()) {
            QApplication::restoreOverrideCursor();
            showErrorMessage("Warning", "File contains no data");
            return;
        }

        qDebug() << "Loaded points:" << m_currentData.size();

        clearChartArea();

        QChart* chart = new QChart();

        if (chartTypeCombo->currentText() == "Line Chart") {
            chart->setTitle("Line Chart - " + QFileInfo(filePath).fileName());
            QLineSeries* series = new QLineSeries();
            int maxPoints = m_currentData.size();
            int step = 1;
            if (maxPoints > 2000) {
                step = maxPoints / 2000;
                qDebug() << "Downsampling: showing every" << step << "th point";
            }
            QStringList dateLabels;

            for (int i = 0; i < maxPoints; i += step) {
                series->append(i / step, m_currentData[i].y());

                qint64 timestamp = static_cast<qint64>(m_currentData[i].x());
                QDateTime dt = QDateTime::fromSecsSinceEpoch(timestamp);
                if (dt.isValid() && timestamp > 0) {
                    dateLabels << dt.toString("dd.MM.yy");
                } else {
                    dateLabels << QString::number(i);
                }
            }

            chart->addSeries(series);

            QValueAxis *axisY = new QValueAxis();
            axisY->setTitleText("Value");

            double minY = m_currentData[0].y();
            double maxY = m_currentData[0].y();
            for (const QPointF& point : m_currentData) {
                if (point.y() < minY) minY = point.y();
                if (point.y() > maxY) maxY = point.y();
            }
            double padding = (maxY - minY) * 0.05;
            if (padding == 0) padding = 1.0;
            axisY->setRange(minY - padding, maxY + padding);
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            QCategoryAxis *axisX = new QCategoryAxis();
            axisX->setTitleText("Date");
            axisX->setLabelsAngle(-45);

            int totalPoints = series->count();
            int labelCount = qMin(15, totalPoints);
            for (int i = 0; i < labelCount; ++i) {
                int position = (i * (totalPoints - 1)) / (labelCount - 1);
                if (position < dateLabels.size()) {
                    axisX->append(dateLabels[position], position);
                }
            }

            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);

            qDebug() << "Line chart created with" << series->count() << "points";

            } else {
            chart->setTitle("Bar Chart - " + QFileInfo(filePath).fileName());

            int totalPoints = m_currentData.size();
            int maxBars = 100;
            int step = 1;
            int barsToShow = totalPoints;

            if (totalPoints > maxBars) {
                step = ceil((double)totalPoints / maxBars);
                barsToShow = ceil((double)totalPoints / step);
                qDebug() << "Bar chart: showing" << barsToShow << "bars (every" << step << "th point)";
            }

            QBarSeries* series = new QBarSeries();
            if (barsToShow > 80) {
                series->setBarWidth(0.3);
            } else if (barsToShow > 50) {
                series->setBarWidth(0.4);
            } else {
                series->setBarWidth(0.5);
            }

            QBarSet* barSet = new QBarSet("Values");
            QStringList categories;

            for (int i = 0; i < totalPoints; i += step) {
                double sum = 0;
                int count = 0;
                for (int j = i; j < qMin(i + step, totalPoints); ++j) {
                    sum += m_currentData[j].y();
                    count++;
                }
                double avgValue = sum / count;
                *barSet << avgValue;

                qint64 timestamp = static_cast<qint64>(m_currentData[i].x());
                QDateTime dt = QDateTime::fromSecsSinceEpoch(timestamp);
                if (dt.isValid() && timestamp > 0) {
                    categories << dt.toString("dd.MM.yy");
                } else {
                    categories << QString::number(i + 1);
                }
            }

            series->append(barSet);
            chart->addSeries(series);

            QBarCategoryAxis *axisX = new QBarCategoryAxis();
            axisX->setTitleText("Date");
            axisX->setLabelsAngle(-45);

            axisX->append(categories);

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

            qDebug() << "Bar chart created with" << barSet->count() << "bars (original points:" << totalPoints << ")";
        }

        if (grayscaleCheckBox->isChecked()) {
            chart->setTheme(QChart::ChartThemeDark);
            chart->setAnimationOptions(QChart::NoAnimation);
        } else {
            chart->setTheme(QChart::ChartThemeLight);
            chart->setAnimationOptions(QChart::SeriesAnimations);
        }

        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->setBackgroundBrush(QBrush(Qt::white));

        QChartView* chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumHeight(MIN_CHART_HEIGHT);



        chartLayout->addWidget(chartView);
        m_isChartDisplayed = true;

        QApplication::restoreOverrideCursor();
        statusBar()->showMessage(QString("Loaded: %1 (%2 points)").arg(QFileInfo(filePath).fileName()).arg(m_currentData.size()));

    } catch (const std::exception& e) {
        QApplication::restoreOverrideCursor();
        showErrorMessage("Error", QString("Exception: %1").arg(e.what()));
        qDebug() << "Exception:" << e.what();
    }
}

void MainWindow::clearChartArea()
{
    QLayoutItem* child;
    while ((child = chartLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

void MainWindow::onChartTypeChanged(int index)
{
    Q_UNUSED(index);
    if (!m_currentFilePath.isEmpty()) {
        loadAndDisplayChart(m_currentFilePath);
    }
}

void MainWindow::onGrayscaleToggled(bool checked)
{
    Q_UNUSED(checked);
    if (!m_currentFilePath.isEmpty()) {
        loadAndDisplayChart(m_currentFilePath);
    }
}

void MainWindow::onPrintToPDF()
{
    if (!m_isChartDisplayed || chartLayout->count() == 0) {
        showErrorMessage("Предупреждение", "Нет графика для печати");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить PDF",
                                                    QDir::homePath(), "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QChartView* chartView = qobject_cast<QChartView*>(chartLayout->itemAt(0)->widget());
    if (!chartView) {
        showErrorMessage("Ошибка", "Виджет графика не найден");
        return;
    }

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setColorMode(grayscaleCheckBox->isChecked() ? QPrinter::GrayScale : QPrinter::Color);
    printer.setPageSize(QPageSize::A4);
    printer.setResolution(300);

    QPainter painter(&printer);
    chartView->render(&painter);
    painter.end();

    QMessageBox::information(this, "Успех", QString("PDF сохранен: %1").arg(fileName));
    statusBar()->showMessage("PDF сохранен: " + fileName, 3000);
}

void MainWindow::showErrorMessage(const QString& title, const QString& message)
{
    QMessageBox::critical(this, title, message);
    statusBar()->showMessage("Ошибка: " + message, 5000);
}

MainWindow::~MainWindow()
{
    delete ui;
}
