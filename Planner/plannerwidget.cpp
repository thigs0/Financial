#include "plannerwidget.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QInputDialog>
#include <QLocale>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

// Converter QDate pra std::tm
std::tm PlannerWidget::qdateToTm(const QDate& date) {
    std::tm tm = {};
    tm.tm_year = date.year() - 1900;
    tm.tm_mon = date.month() - 1;
    tm.tm_mday = date.day();
    return tm;
}

PlannerWidget::PlannerWidget(QWidget *parent)
    : QWidget(parent),
      tableAccumulated(nullptr),
      chartView(nullptr),
      chart(nullptr),
      series(nullptr)
{
    auto *layout = new QVBoxLayout(this);

    // Categoria Combo Box
    cbCategory = new QComboBox(this);
    loadCategories("categories.txt");
    cbCategory->setEditable(true);
    cbCategory->setInsertPolicy(QComboBox::InsertAtBottom);

    btnAdd = new QPushButton("Adicionar Compra", this);

    tableAccumulated = new QTableWidget(12, 2, this);
    tableAccumulated->setHorizontalHeaderLabels({"Mês", "Acumulado"});

    // Montar o gráfico
    series = new QtCharts::QBarSeries(this);
    chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("Gastos Planejados por Mês");
    chart->legend()->setVisible(false);
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    // Eixos
    auto *axisX = new QtCharts::QBarCategoryAxis();
    QStringList months;
    months << "Jan" << "Fev" << "Mar" << "Abr" << "Mai" << "Jun"
           << "Jul" << "Ago" << "Set" << "Out" << "Nov" << "Dez";
    axisX->append(months);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto *axisY = new QtCharts::QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chartView = new QtCharts::QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Layout: categoria, botão, tabela e gráfico
    layout->addWidget(cbCategory);
    layout->addWidget(btnAdd);
    layout->addWidget(tableAccumulated);
    layout->addWidget(chartView);

    connect(btnAdd, &QPushButton::clicked, this, &PlannerWidget::onAddPurchase);

    updateAccumulatedTable();
    updateChart();
}

void PlannerWidget::loadCategories(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Mensagem opcional, mas não fatal
        return;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            cbCategory->addItem(line);
        }
    }
    file.close();
}

void PlannerWidget::onAddPurchase() {
    bool ok;
    QString name = QInputDialog::getText(this, "Nova Compra", "Nome:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    QString category = cbCategory->currentText();
    if (cbCategory->findText(category) == -1) {
        cbCategory->addItem(category);
        QFile file("categories.txt");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << "\n" << category;
            file.close();
        }
    }

    QString subcategory = QInputDialog::getText(this, "Nova Compra", "Subcategoria:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString observation = QInputDialog::getText(this, "Nova Compra", "Observação:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    double value = QInputDialog::getDouble(this, "Nova Compra", "Valor:", 0, 0, 1e9, 2, &ok);
    if (!ok) return;

    QStringList ownerOptions = {"Pessoa", "Empresa"};
    QString ownerStr = QInputDialog::getItem(this, "Nova Compra", "Destinatário:", ownerOptions, 0, false, &ok);
    if (!ok) return;
    OwnerType owner = (ownerStr == "Pessoa") ? OwnerType::Person : OwnerType::Company;

    QStringList repeatOptions = {"Uma vez", "Diária", "Semanal", "Mensal"};
    QString repeatStr = QInputDialog::getItem(this, "Nova Compra", "Repetição:", repeatOptions, 0, false, &ok);
    if (!ok) return;

    RepeatInterval repeat;
    if (repeatStr == "Uma vez") repeat = RepeatInterval::Once;
    else if (repeatStr == "Diária") repeat = RepeatInterval::Daily;
    else if (repeatStr == "Semanal") repeat = RepeatInterval::Weekly;
    else repeat = RepeatInterval::Monthly;

    QDate startDate = QDate::currentDate();
    std::tm tmStart = qdateToTm(startDate);

    Purchase p;
    p.name = name.toStdString();
    p.category = category.toStdString();
    p.subcategory = subcategory.toStdString();
    p.observation = observation.toStdString();
    p.value = value;
    p.owner = owner;
    p.repeat = repeat;
    p.startDate = tmStart;

    purchaseManager.addPurchase(p);

    QMessageBox::information(this, "Compra adicionada", "Compra planejada adicionada com sucesso.");

    updateAccumulatedTable();
    updateChart();
}

void PlannerWidget::updateAccumulatedTable() {
    int year = QDate::currentDate().year();
    tableAccumulated->setRowCount(12);
    for (int month = 1; month <= 12; ++month) {
        std::tm date = {};
        date.tm_year = year - 1900;
        date.tm_mon = month - 1;
        date.tm_mday = 31;

        double accumulated = purchaseManager.accumulatedUpTo(date);

        QTableWidgetItem *monthItem = new QTableWidgetItem(QLocale().monthName(month));
        QTableWidgetItem *accumItem = new QTableWidgetItem(QString::number(accumulated, 'f', 2));

        tableAccumulated->setItem(month - 1, 0, monthItem);
        tableAccumulated->setItem(month - 1, 1, accumItem);
    }
}

void PlannerWidget::updateChart() {
    // Limpa o series anterior
    series->clear();

    int year = QDate::currentDate().year();
    // Coleta os dados do PurchaseManager
    QVector<double> data;  // 12 valores, um para cada mês
    data.reserve(12);
    for (int month = 1; month <= 12; ++month) {
        std::tm date = {};
        date.tm_year = year - 1900;
        date.tm_mon = month - 1;
        date.tm_mday = 31;
        double acc = purchaseManager.accumulatedUpTo(date);
        data.append(acc);
    }

    // Cria um QBarSet com os valores
    auto *barSet = new QtCharts::QBarSet("Acumulado");
    for (double v : data) {
        *barSet << v;
    }
    series->append(barSet);

    // Ajusta eixo Y para mostrar todos os valores apropriadamente
    double maxVal = 0.0;
    for (double v : data) {
        if (v > maxVal) maxVal = v;
    }
    // Definir range do eixo Y
    auto *axisY = qobject_cast<QtCharts::QValueAxis*>(chart->axisY());
    if (axisY) {
        axisY->setRange(0, maxVal * 1.1);  // um pouquinho acima do max para folga
    }

    // Atualiza o view
    chartView->repaint();
}
