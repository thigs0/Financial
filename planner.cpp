#include "Planner/planner.hpp"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>

void PurchaseManager::addPurchase(const Purchase& p) {
    purchases.push_back(p);
}

// Converte std::tm em std::chrono::system_clock::time_point
static std::chrono::system_clock::time_point tmToTimePoint(const std::tm& t) {
    return std::chrono::system_clock::from_time_t(std::mktime(const_cast<std::tm*>(&t)));
}

// Adiciona dias a uma data
static std::tm addDays(std::tm date, int days) {
    std::time_t tt = std::mktime(&date);
    tt += days * 86400;
    return *std::localtime(&tt);
}

// Adiciona semanas
static std::tm addWeeks(std::tm date, int weeks) {
    return addDays(date, weeks * 7);
}

// Adiciona meses com overflow ajustado
static std::tm addMonths(std::tm date, int months) {
    date.tm_mon += months;
    mktime(&date); // Normaliza
    return date;
}

// Compara se data1 <= data2
static bool isBeforeOrEqual(const std::tm& a, const std::tm& b) {
    return std::mktime(const_cast<std::tm*>(&a)) <= std::mktime(const_cast<std::tm*>(&b));
}

double PurchaseManager::accumulatedUpTo(const std::tm& date) const {
    double total = 0.0;
    for (const auto& p : purchases) {
        std::tm curr = p.startDate;
        switch (p.repeat) {
            case RepeatInterval::Once:
                if (isBeforeOrEqual(curr, date))
                    total += p.value;
                break;

            case RepeatInterval::Daily:
                while (isBeforeOrEqual(curr, date)) {
                    total += p.value;
                    curr = addDays(curr, 1);
                }
                break;

            case RepeatInterval::Weekly:
                while (isBeforeOrEqual(curr, date)) {
                    total += p.value;
                    curr = addWeeks(curr, 1);
                }
                break;

            case RepeatInterval::Monthly:
                while (isBeforeOrEqual(curr, date)) {
                    total += p.value;
                    curr = addMonths(curr, 1);
                }
                break;
        }
    }
    return total;
}

std::vector<Purchase> PurchaseManager::duePurchasesUpTo(const std::tm& date) const {
    std::vector<Purchase> due;
    for (const auto& p : purchases) {
        std::tm curr = p.startDate;
        switch (p.repeat) {
            case RepeatInterval::Once:
                if (isBeforeOrEqual(curr, date))
                    due.push_back(p);
                break;

            case RepeatInterval::Daily:
                while (isBeforeOrEqual(curr, date)) {
                    due.push_back(p);
                    curr = addDays(curr, 1);
                }
                break;

            case RepeatInterval::Weekly:
                while (isBeforeOrEqual(curr, date)) {
                    due.push_back(p);
                    curr = addWeeks(curr, 1);
                }
                break;

            case RepeatInterval::Monthly:
                while (isBeforeOrEqual(curr, date)) {
                    due.push_back(p);
                    curr = addMonths(curr, 1);
                }
                break;
        }
    }
    return due;
}

void PurchaseManager::printAccumulatedByMonth(int year) const {
    std::cout << "Gastos planejados acumulados mês a mês para o ano " << year << ":\n";

    std::vector<double> monthlyTotal(12, 0.0);

    for (const auto& p : purchases) {
        std::tm curr = p.startDate;

        while (curr.tm_year + 1900 <= year) {
            if (curr.tm_year + 1900 == year) {
                int month = curr.tm_mon;
                monthlyTotal[month] += p.value;
            }

            switch (p.repeat) {
                case RepeatInterval::Once:
                    break; // Não avança
                case RepeatInterval::Daily:
                    curr = addDays(curr, 1);
                    break;
                case RepeatInterval::Weekly:
                    curr = addWeeks(curr, 1);
                    break;
                case RepeatInterval::Monthly:
                    curr = addMonths(curr, 1);
                    break;
            }

            if (p.repeat == RepeatInterval::Once)
                break;
        }
    }

    static const char* months[] = {
        "Jan", "Fev", "Mar", "Abr", "Mai", "Jun",
        "Jul", "Ago", "Set", "Out", "Nov", "Dez"
    };

    for (int i = 0; i < 12; ++i) {
        std::cout << months[i] << ": R$ " << std::fixed << std::setprecision(2) << monthlyTotal[i] << "\n";
    }
}
