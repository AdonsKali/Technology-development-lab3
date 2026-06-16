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
#include <QFileInfo>
#include "interfaces/idatafactory.h"
#include "ioc/container.h"
#include "styles/colorstyle.h"
#include "styles/grayscalestyle.h"



MainWindow::MainWindow(std::shared_ptr<IOCContainer> container, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_container(container)
    , m_isChartDisplayed(false)
{
    m_chartService = m_container->GetObject<ChartService>();
    ui->setupUi(this);

    this->setWindowTitle("Print charts");
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
    qDebug() << "=== loadAndDisplayChart START ===";
    qDebug() << "FilePath:" << filePath;

    m_currentFilePath = filePath;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    try {
        qDebug() << "Getting IDataFactory from container...";
        auto dataFactory = m_container->GetObject<IDataFactory>();
        if (!dataFactory) {
            qDebug() << "ERROR: dataFactory is NULL!";
            throw std::runtime_error("DataFactory not found");
        }

        qDebug() << "Checking supports...";
        if (!dataFactory->supports(filePath)) {
            throw std::runtime_error("Unsupported file format");
        }

        qDebug() << "Creating data adapter...";
        auto data = dataFactory->create(filePath);
        if (!data) {
            throw std::runtime_error("Failed to create data adapter");
        }
        qDebug() << "Data adapter created, type:" << data->getType();

        qDebug() << "Loading data...";
        if (!data->load(filePath)) {
            throw std::runtime_error("Failed to load data");
        }

        m_currentData = data->getPoints();
        qDebug() << "Data loaded, points count:" << m_currentData.size();

        qDebug() << "Creating chart via ChartService...";
        auto chart = m_chartService->createChart(data);
        if (!chart) {
            throw std::runtime_error("Failed to create chart");
        }
        qDebug() << "Chart created successfully";

        m_currentChart = chart;

        clearChartArea();
        m_currentChartView = new QChartView(m_currentChart.get());
        m_currentChartView->setRenderHint(QPainter::Antialiasing);
        m_currentChartView->setMinimumHeight(MIN_CHART_HEIGHT);

        chartLayout->addWidget(m_currentChartView);
        m_isChartDisplayed = true;

        qDebug() << "=== loadAndDisplayChart SUCCESS ===";

    } catch (const std::exception& e) {
        qDebug() << "=== EXCEPTION ===" << e.what();
        showErrorMessage("Error", e.what());
    }

    QApplication::restoreOverrideCursor();
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
    if (!m_currentFilePath.isEmpty()) {
        QString chartType = (index == 0) ? "line" : "bar";
        qDebug() << "onChartTypeChanged: index=" << index << "chartType=" << chartType;

        if (m_chartService) {
            m_chartService->setChartType(chartType);
        }

        loadAndDisplayChart(m_currentFilePath);
    }
}

void MainWindow::onGrayscaleToggled(bool checked)
{
    if (!m_currentFilePath.isEmpty()) {
        if (m_container) {
            if (checked) {
                auto grayscaleStyle = m_container->GetObject<GrayscaleStyle>();
                if (grayscaleStyle && m_chartService) {
                    m_chartService->setStyle(grayscaleStyle);
                }
            } else {
                auto colorStyle = m_container->GetObject<ColorStyle>();
                if (colorStyle && m_chartService) {
                    m_chartService->setStyle(colorStyle);
                }
            }
        }
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
