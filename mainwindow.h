#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include "services/chartservice.h"
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class QChartView;
class IOCContainer;
class IData;
class QChart;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<IOCContainer> container,
                        QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selectionChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void onFileSelected(const QItemSelection &selected, const QItemSelection &deselected);
    void onChartTypeChanged(int index);
    void onGrayscaleToggled(bool checked);
    void onPrintToPDF();

private:
    void setupChartArea();
    void loadAndDisplayChart(const QString& filePath);
    void clearChartArea();
    void showErrorMessage(const QString& title, const QString& message);

    std::shared_ptr<IOCContainer> m_container;
    std::shared_ptr<ChartService> m_chartService;

    Ui::MainWindow *ui;

    QFileSystemModel *rightPartModel;
    QFileSystemModel *leftPartModel;
    QTreeView *treeView;
    QTableView *tableView;

    QSplitter *rightSplitter;
    QWidget *chartPanel;
    QVBoxLayout *chartPanelLayout;
    QHBoxLayout *chartControlsLayout;
    QComboBox *chartTypeCombo;
    QCheckBox *grayscaleCheckBox;
    QPushButton *printButton;
    QVBoxLayout *chartLayout;

    QString m_currentFilePath;
    bool m_isChartDisplayed;
    QVector<QPointF> m_currentData;

    std::shared_ptr<QChart> m_currentChart;
    QChartView* m_currentChartView;

    static constexpr int MIN_CHART_HEIGHT = 400;
};

#endif
