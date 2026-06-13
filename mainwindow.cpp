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
            qDebug() << "Загрузка SQLite базы данных...";
        } else if (filePath.endsWith(".json")) {
            data = std::make_shared<JSONDataAdapter>();
            qDebug() << "Загрузка JSON файла...";
        } else {
            QApplication::restoreOverrideCursor();
            showErrorMessage("Ошибка", "Неподдерживаемый формат файла");
            return;
        }

        if (!data->load(filePath)) {
            QApplication::restoreOverrideCursor();
            showErrorMessage("Ошибка", "Не удалось загрузить данные из файла");
            return;
        }

        m_currentData = data->getPoints();

        if (m_currentData.isEmpty()) {
            QApplication::restoreOverrideCursor();
            showErrorMessage("Предупреждение", "Файл не содержит данных");
            return;
        }

        qDebug() << "Загружено точек:" << m_currentData.size();

        QChart* chart = new QChart();

        if (chartTypeCombo->currentText() == "Line Chart") {
            chart->setTitle("Line Chart");
            QLineSeries* series = new QLineSeries();
            int step = qMax(1, m_currentData.size() / 1000);
            for (int i = 0; i < m_currentData.size(); i += step) {
                series->append(m_currentData[i]);
            }
            chart->addSeries(series);
        } else {
            chart->setTitle("Bar Chart");
            QBarSeries* series = new QBarSeries();
            QBarSet* barSet = new QBarSet("Values");

            int maxPoints = qMin(100, m_currentData.size());
            for (int i = 0; i < maxPoints; ++i) {
                *barSet << m_currentData[i].y();
            }
            series->append(barSet);
            chart->addSeries(series);
        }

        chart->createDefaultAxes();

        if (grayscaleCheckBox->isChecked()) {
            chart->setTheme(QChart::ChartThemeDark);
            chart->setAnimationOptions(QChart::NoAnimation);
        } else {
            chart->setTheme(QChart::ChartThemeLight);
            chart->setAnimationOptions(QChart::SeriesAnimations);
        }
        clearChartArea();

        QChartView* chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing, true);
        chartView->setMinimumHeight(MIN_CHART_HEIGHT);

        chartLayout->addWidget(chartView);
        m_isChartDisplayed = true;

        QApplication::restoreOverrideCursor();
        statusBar()->showMessage(QString("Загружено: %1 (%2 точек)").arg(QFileInfo(filePath).fileName()).arg(m_currentData.size()));

    } catch (const std::exception& e) {
        QApplication::restoreOverrideCursor();
        showErrorMessage("Ошибка", QString("Исключение: %1").arg(e.what()));
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
