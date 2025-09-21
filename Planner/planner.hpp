#ifndef PURCHASE_MANAGER_HPP
#define PURCHASE_MANAGER_HPP

#include <string>
#include <vector>
#include <chrono>
#include <ctime>

// Tipo de repetição da compra
enum class RepeatInterval {
    Once,
    Daily,
    Weekly,
    Monthly
};

// Informação de quem é o destinatário/beneficiário
enum class OwnerType {
    Person,
    Company
};

struct Purchase {
    std::string name;
    std::string category;
    std::string subcategory;
    std::string observation;
    double value;
    OwnerType owner;
    RepeatInterval repeat;
    // Data de início da compra planejada
    std::tm startDate;
};

class PurchaseManager {
private:
    std::vector<Purchase> purchases;

public:
    void addPurchase(const Purchase& p);
    // Retorna o gasto acumulado até a data dada (inclusive)
    double accumulatedUpTo(const std::tm& date) const;
    // Lista de compras que “ativam” até uma data
    std::vector<Purchase> duePurchasesUpTo(const std::tm& date) const;
    
    // Para exibir gráfico ASCII do acumulado mês a mês num ano
    void printAccumulatedByMonth(int year) const;
};

#endif // PURCHASE_MANAGER_HPP
