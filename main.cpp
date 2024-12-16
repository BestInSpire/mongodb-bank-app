#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdint>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/exception/exception.hpp>

using namespace bsoncxx::builder::stream;

int main() {
    // MongoDB driver instance
    mongocxx::instance instance{};

    // Veritabanına bağlan
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);

    // "bank" adlı veritabanını ve "accounts" adlı koleksiyonu kullan
    auto db = client["bank"];
    auto accounts = db["accounts"];

    while (true) {
        std::cout << "\n--- Banka Uygulaması ---\n";
        std::cout << "1. Yeni hesap olustur\n";
        std::cout << "2. Bakiye goruntule\n";
        std::cout << "3. Para transferi yap\n";
        std::cout << "4. Cikis\n";
        std::cout << "Secim: ";

        int choice;
        std::cin >> choice;

        if (choice == 1) {
            // Yeni hesap oluştur
            std::string username;
            double initial_balance;
            std::cout << "Kullanici adi: ";
            std::cin >> username;
            std::cout << "Baslangic bakiyesi: ";
            std::cin >> initial_balance;

            bsoncxx::builder::stream::document doc;
            doc << "username" << username
                    << "balance" << initial_balance;

            try {
                accounts.insert_one(doc.view());
                std::cout << "Hesap basariyla olusturuldu.\n";
            } catch (const mongocxx::exception &e) {
                std::cerr << "Veritabanina kayit sirasinda hata: " << e.what() << "\n";
            }
        } else if (choice == 2) {
            // Bakiye görüntüle
            std::string username;
            std::cout << "Kullanici adi: ";
            std::cin >> username;

            auto query = document{} << "username" << username << finalize;
            auto maybe_result = accounts.find_one(query.view());

            if (maybe_result) {
                auto view = maybe_result->view();
                double balance = view["balance"].get_double().value;
                std::cout << username << " adli kullanicinin bakiyesi: " << balance << "\n";
            } else {
                std::cout << "Boyle bir hesap bulunamadi.\n";
            }
        } else if (choice == 3) {
            // Para transferi
            std::string from_user, to_user;
            double amount;
            std::cout << "Gonderen kullanici adi: ";
            std::cin >> from_user;
            std::cout << "Alici kullanici adi: ";
            std::cin >> to_user;
            std::cout << "Transfer tutari: ";
            std::cin >> amount;

            // Gönderen hesabı kontrol et
            auto from_query = document{} << "username" << from_user << finalize;
            auto from_result = accounts.find_one(from_query.view());

            if (!from_result) {
                std::cout << "Gonderen hesap bulunamadi.\n";
                continue;
            }

            // Alıcı hesabı kontrol et
            auto to_query = document{} << "username" << to_user << finalize;
            auto to_result = accounts.find_one(to_query.view());

            if (!to_result) {
                std::cout << "Alici hesap bulunamadi.\n";
                continue;
            }

            double from_balance = (*from_result)["balance"].get_double().value;
            double to_balance = (*to_result)["balance"].get_double().value;

            if (from_balance < amount) {
                std::cout << "Yetersiz bakiye.\n";
                continue;
            }

            // İşlemi yap
            double new_from_balance = from_balance - amount;
            double new_to_balance = to_balance + amount;

            // Gönderen hesabı güncelle
            accounts.update_one(
                document{} << "username" << from_user << finalize,
                document{} << "$set" << open_document
                << "balance" << new_from_balance << close_document << finalize
            );

            // Alıcı hesabı güncelle
            accounts.update_one(
                document{} << "username" << to_user << finalize,
                document{} << "$set" << open_document
                << "balance" << new_to_balance << close_document << finalize
            );

            std::cout << "Transfer basarili.\n";
        } else if (choice == 4) {
            std::cout << "Program sonlandiriliyor...\n";
            break;
        } else {
            std::cout << "Gecersiz secim!\n";
        }
    }

    return 0;
