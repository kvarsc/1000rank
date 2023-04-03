#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(const std::string& db_path) : db(db_path) {
    // You can add any initialization code here if needed
}

DatabaseManager::~DatabaseManager() {
    // Destructor will be called automatically, closing the connection to the database
}