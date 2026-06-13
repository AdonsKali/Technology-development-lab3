#include "mainwindow.h"
#include <QSplitter>
#include <QTreeView>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QTableView>
#include <QHeaderView>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setGeometry(100, 100, 1500, 500);
    this->setStatusBar(new QStatusBar(this));
    this->statusBar()->showMessage("Выбранный путь : ");

    QString homePath = QDir::homePath();
    leftPartModel = new QFileSystemModel(this);
    leftPartModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    leftPartModel->setRootPath(homePath);
    
    rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(tableView);
    setupChartArea();
    rightSplitter->addWidget(chartPanel);
    rightSplitter->setStretchFactor(0, 1);
    rightSplitter->setStretchFactor(1, 2);
    
    treeView = new QTreeView();
    treeView->setModel(leftPartModel);
    treeView->setHeaderHidden(true);
    treeView->setRootIndex(leftPartModel->index(homePath));
    treeView->expandAll();
    treeView->header()->resizeSection(0, 200);

    tableView = new QTableView();
    tableView->setModel(rightPartModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setStretchLastSection(true);

    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(treeView);
    splitter->addWidget(tableView);
    setCentralWidget(splitter);

    QItemSelectionModel *selectionModel = treeView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &MainWindow::on_selectionChangedSlot);
    
    QModelIndex topLeft = leftPartModel->index(homePath);
    QItemSelection toggleSelection;
    toggleSelection.select(topLeft, topLeft);
    selectionModel->select(toggleSelection, QItemSelectionModel::Toggle);
}

void MainWindow::on_selectionChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    
    QModelIndexList indexes = selected.indexes();
    if (indexes.count() >= 1) {
        QModelIndex ix = indexes.constFirst();
        QString filePath = leftPartModel->filePath(ix);
        statusBar()->showMessage("Выбранный путь : " + filePath);
        tableView->setRootIndex(rightPartModel->setRootPath(filePath));
    }
}

void MainWindow::setupChartArea()
{
    chartPanel = new QWidget(this);
    chartPanelLayout = new QVBoxLayout(chartPanel);
    chartPanelLayout->setContentsMargins(5, 5, 5, 5);
    chartLayout = new QVBoxLayout();
    chartLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* placeholder = new QLabel("Выберите файл для отображения графика", this);
    placeholder->setAlignment(Qt::AlignCenter);
    chartLayout->addWidget(placeholder);
    chartPanelLayout->addLayout(chartLayout);
}

MainWindow::~MainWindow() {}