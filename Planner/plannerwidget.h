#ifndef PLANNERWIDGET_H
#define PLANNERWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDate>
#include <QDateEdit>
#include "planner.hpp"
 
QT_BEGIN_NAMESPACE
namespace QtCharts {
    class QChartView;
    class QBarSeries;
    class QChart;
    class QBarCategoryAxis;
    class QValueAxis;
    class QBarSet;
}
QT_END_NAMESPACE

class PlannerWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlannerWidget(QWidget *parent = nullptr);

private slots:
    void onAddPurchase();
    void updateAccumulatedTable();
    void updateChart();  // nova função para atualizar o gráfico

private:
    PurchaseManager purchaseManager;
    QTableWidget *tableAccumulated;

    QComboBox *cbCategory;
    // Outras entradas que você já tem
    QPushButton *btnAdd;

    // Gráfico
    QtCharts::QChartView *chartView;
    QtCharts::QChart *chart;
    QtCharts::QBarSeries *series;

    void loadCategories(const QString &filePath);
    std::tm qdateToTm(const QDate& date);
};

#endif // PLANNERWIDGET_H
