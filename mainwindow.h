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