#include "pay.h"
#include <QTextStream>
#include <QStandardItem>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QInputDialog>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "nubankreader.h"
#include "Planner/plannerwidget.h"

void PayApp::loadCsvFiles() {
    // Seleciona arquivo CSV via diálogo
    QString fileName = QFileDialog::getOpenFileName(this, "Selecionar CSV Nubank", "", "CSV (*.csv)");
    if (fileName.isEmpty()) return;

    // Chama função que lê o CSV e retorna os dados já processados
    QVector<NubankEntry> entries = read_nubank_csv(fileName);

    if (entries.isEmpty()) {
        QMessageBox::warning(this, "Erro", "The file is empty");
        return;
    }

    // Suponha que você tenha um QStandardItemModel* model na classe PayApp para exibir os dados
    model->clear();
    model->setHorizontalHeaderLabels({"Data", "Valor", "ID", "Tipo", "Descrição", "Para"});

    // Preenche o modelo com os dados lidos
    for (const NubankEntry &e : entries) {
        QList<QStandardItem*> row;
        row << new QStandardItem(e.date.toString("dd/MM/yyyy"));
        row << new QStandardItem(QString::number(e.value, 'f', 2));
        row << new QStandardItem(e.id);
        row << new QStandardItem(e.type_transfer);
        row << new QStandardItem(e.description);
        row << new QStandardItem(e.to);
        model->appendRow(row);
    }
}

// Helper: Deriva chave e IV da senha
bool deriveKeyAndIV(const QString &password, QByteArray &key, QByteArray &iv) {
    const int keyLen = 32; // 256 bits
    const int ivLen = 16;  // AES block size
    QByteArray salt = "payappsalt"; // fixo ou random (para mais segurança)
    key.resize(keyLen);
    iv.resize(ivLen);

    int res = PKCS5_PBKDF2_HMAC(password.toUtf8().data(), password.length(),
                                reinterpret_cast<const unsigned char*>(salt.data()), salt.size(),
                                10000, EVP_sha256(), keyLen + ivLen,
                                reinterpret_cast<unsigned char*>(key.data()));

    if (res != 1)
        return false;

    iv = key.mid(keyLen, ivLen);
    key = key.left(keyLen);
    return true;
}

bool encryptData(const QByteArray &plaintext, QByteArray &ciphertext, const QByteArray &key, const QByteArray &iv) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(key.data()),
                           reinterpret_cast<const unsigned char*>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    ciphertext.resize(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outlen1 = 0;
    if (EVP_EncryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(ciphertext.data()), &outlen1,
                          reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int outlen2 = 0;
    if (EVP_EncryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(ciphertext.data()) + outlen1, &outlen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    ciphertext.resize(outlen1 + outlen2);
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

bool decryptData(const QByteArray &ciphertext, QByteArray &plaintext, const QByteArray &key, const QByteArray &iv) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(key.data()),
                           reinterpret_cast<const unsigned char*>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    plaintext.resize(ciphertext.size());
    int outlen1 = 0;
    if (EVP_DecryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(plaintext.data()), &outlen1,
                          reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int outlen2 = 0;
    if (EVP_DecryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(plaintext.data()) + outlen1, &outlen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    plaintext.resize(outlen1 + outlen2);
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

void PayApp::loadEncryptedDataFromFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir Arquivo Criptografado", "", "Encrypted (*.enc)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Erro", "Não foi possível abrir o arquivo.");
        return;
    }

    QByteArray ciphertext = file.readAll();
    file.close();

    QString password = QInputDialog::getText(this, "Senha", "Digite a senha para descriptografar:", QLineEdit::Password);
    if (password.isEmpty()) return;

    QByteArray key, iv;
    if (!deriveKeyAndIV(password, key, iv)) {
        QMessageBox::warning(this, "Erro", "Erro ao gerar chave criptográfica.");
        return;
    }

    QByteArray plaintext;
    if (!decryptData(ciphertext, plaintext, key, iv)) {
        QMessageBox::warning(this, "Erro", "Senha incorreta ou dados corrompidos.");
        return;
    }

    // Reconstrói o modelo
    QDataStream in(&plaintext, QIODevice::ReadOnly);
    int rows, cols;
    in >> rows >> cols;

    model->clear();
    model->setRowCount(rows);
    model->setColumnCount(cols);
    model->setHorizontalHeaderLabels({"Data", "Valor", "ID", "Tipo", "Descrição", "Para"});

    for (int row = 0; row < rows; ++row) {
        QList<QStandardItem*> items;
        for (int col = 0; col < cols; ++col) {
            QString value;
            in >> value;
            items.append(new QStandardItem(value));
        }
        model->appendRow(items);
    }

    QMessageBox::information(this, "Sucesso", "Dados carregados com sucesso.");
}


void PayApp::saveEncryptedDataToFile() {
    QString password = QInputDialog::getText(this, "Senha", "Defina uma senha para criptografar:", QLineEdit::Password);
    if (password.isEmpty()) return;

    QByteArray key, iv;
    if (!deriveKeyAndIV(password, key, iv)) {
        QMessageBox::warning(this, "Erro", "Erro ao gerar chave criptográfica.");
        return;
    }

    // Serializa os dados
    QByteArray plaintext;
    QDataStream out(&plaintext, QIODevice::WriteOnly);
    out << model->rowCount() << model->columnCount();

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QStandardItem *item = model->item(row, col);
            out << (item ? item->text() : "");
        }
    }

    // Criptografa
    QByteArray ciphertext;
    if (!encryptData(plaintext, ciphertext, key, iv)) {
        QMessageBox::warning(this, "Erro", "Erro ao criptografar os dados.");
        return;
    }

    // Salva em arquivo
    QString fileName = QFileDialog::getSaveFileName(this, "Salvar Arquivo Criptografado", "", "Encrypted (*.enc)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Erro", "Não foi possível salvar o arquivo.");
        return;
    }

    file.write(ciphertext);
    file.close();
    QMessageBox::information(this, "Sucesso", "Dados criptografados e salvos com sucesso.");
}


PayApp::PayApp(QWidget *parent) : QMainWindow(parent) {
    // Aba 1: Nubank + Criptografia
    auto *mainWidget = new QWidget;
    auto *layout = new QVBoxLayout(mainWidget);

    auto *btnSave = new QPushButton("Salvar Dados Criptografados");
    auto *btnLoadEncrypted = new QPushButton("Abrir Dados Criptografados");
    auto *btnLoad = new QPushButton("Adicionar CSV(s)");

    layout->addWidget(btnSave);
    layout->addWidget(btnLoadEncrypted);
    layout->addWidget(btnLoad);

    connect(btnSave, &QPushButton::clicked, this, &PayApp::saveEncryptedDataToFile);
    connect(btnLoadEncrypted, &QPushButton::clicked, this, &PayApp::loadEncryptedDataFromFile);
    connect(btnLoad, &QPushButton::clicked, this, &PayApp::loadCsvFiles);

    table = new QTableView(this);
    layout->addWidget(table);

    model = new QStandardItemModel(this);
    table->setModel(model);

    // Aba 2: Planner (Compras Planejadas)
    auto *plannerTab = new PlannerWidget;

    // Criando QTabWidget com ambas as abas
    auto *tabs = new QTabWidget;
    tabs->addTab(mainWidget, "Transações");
    tabs->addTab(plannerTab, "Planejamento");

    setCentralWidget(tabs);
    setWindowTitle("Pay App C++");
    resize(800, 600);
}