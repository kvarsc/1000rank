#pragma once
#include <string>
#include <vector>
#include "hdr/sqlite3.h"
#include "hdr/sqlite_modern_cpp.h"

class DatabaseManager {
public:
    DatabaseManager(const std::string& db_path);
    ~DatabaseManager();

private:
    sqlite::database db;
};