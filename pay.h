#ifndef PAY_H
#define PAY_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QMessageBox>


class PayApp : public QMainWindow {
    Q_OBJECT

public:
    PayApp(QWidget *parent = nullptr);

private slots:
    void loadCsvFiles();
    
    // Adicione estas duas declarações:
    void saveEncryptedDataToFile();
    void loadEncryptedDataFromFile();

private:
    QTableView *table;
    QStandardItemModel *model;
};

#endif // PAY_H
