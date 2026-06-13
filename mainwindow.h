#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selectionChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void onChartTypeChanged(int index);
    void onGrayscaleToggled(bool checked);
    void onPrintToPDF();

private:
    void setupChartArea();
    void loadAndDisplayChart(const QString& filePath);
    void clearChartArea();
    void showErrorMessage(const QString& title, const QString& message);
    QString m_currentFilePath;
    bool m_isChartDisplayed;
    QVector<QPointF> m_currentData;
    static constexpr int MIN_CHART_HEIGHT = 400;
    QFileSystemModel *rightPartModel;
    QFileSystemModel *leftPartModel;
    QTreeView *treeView;
    QTableView *tableView;
    QSplitter *rightSplitter;
    QWidget *chartPanel;
    QVBoxLayout *chartPanelLayout;
    QVBoxLayout *chartLayout;
    QHBoxLayout *chartControlsLayout;
    QComboBox *chartTypeCombo;
    QCheckBox *grayscaleCheckBox;
    QPushButton *printButton;
};

#endif