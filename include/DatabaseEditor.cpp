//Copyright 2022 wm8


#include "DatabaseEditor.h"

#include <iostream>
#include <queue>
DatabaseEditor::DatabaseEditor(std::string path) {
  int mode = -1;
  db_path = path;
  std::string t;
  options.create_if_missing = true;
  options.create_missing_column_families = true;
  options.keep_log_file_num = 1;
  options.info_log_level = ::rocksdb::InfoLogLevel::FATAL_LEVEL;
  options.recycle_log_file_num = 1;
  std::cout << "Modes:\n1 - add (key, value)\n2 - show table\n"
               "3 - show all tables\n4 - list of tables\n"
               "5 - create empty table\n6 - create random table\n";
  do {
    std::cout << "Select mode: ";
    std::cin >> mode;
    switch (mode) {
      case 0:
        std::cout << "Modes:\n1 - add (key, value)\n2 - show table\n"
                     "3 - show all tables\n4 - list of tables\n"
                     "5 - create empty table\n6 - create random table\n";
        break;
      case 1:
        addValue();
        break;
      case 2:
        showTable();
        break;
      case 3:
        showAllTables();
        break;
      case 4: {
        size_t pos;
        auto* columns = getTables("", pos);
        for (auto column : *columns) std::cout << column.name << std::endl;
      } break;
      case 5:
        createTable();
        break;
      case 6:
        createRandomTable();
        break;
      case 7:
        deleteAll();
        break;
      default:
        break;
    }
  } while (mode >= 0 && mode < 8);
}
void DatabaseEditor::addValue() {
  DB* db;
  string temp, value;
  std::cout << "Enter table name (0 - for default): ";
  std::cin >> temp;
  if (temp == "0") temp = ROCKSDB_NAMESPACE::kDefaultColumnFamilyName;
  size_t pos;
  auto* column_families = getTables(temp, pos);
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(DBOptions(), db_path, *column_families, &handles, &db);
  if (!s.ok()) {
    std::cerr << s.ToString() << std::endl;
    for (auto handle : handles) db->DestroyColumnFamilyHandle(handle);
    delete column_families;
    delete db;
  }
  std::cout << "Enter key: ";
  std::cin >> temp;
  std::cout << "Enter value: ";
  std::cin >> value;
  db->Put(WriteOptions(), handles[pos], temp, value);
  std::cout << "Done!\n";
  for (auto handle : handles) db->DestroyColumnFamilyHandle(handle);
  delete column_families;
  delete db;
}
void DatabaseEditor::showTable() {
  std::string name;
  std::cout << "Enter table name: ";
  std::cin >> name;
  DB* db;
  size_t pos;
  auto* column_families = getTables(name, pos);
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(DBOptions(), db_path, *column_families, &handles, &db);
  assert(s.ok());
  std::cout << "Table name: " << name << std::endl;
  rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), handles[pos]);
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::cout << "\t" << it->key().ToString() << ": " << it->value().ToString()
              << std::endl;
  }
  delete it;
  for (auto &handle : handles) db->DestroyColumnFamilyHandle(handle);
  delete column_families;
  delete db;
}
std::vector<ColumnFamilyDescriptor>* DatabaseEditor::getTables
    (string _name, size_t& position) {
  auto* column_families = new std::vector<ColumnFamilyDescriptor>();
  auto* column_names = new std::vector<string>();
  Status s = DB::ListColumnFamilies(options, db_path, column_names);
  if(!s.ok())
  {
    DB* db;
    DB::Open(options, db_path, &db);
    delete db;
    return getTables(_name, position);
  }
  bool already_in_list = _name == "";
  size_t i = 0;
  for (string name : *column_names) {
    if (name == _name) {
      already_in_list = true;
      position = i;
    }
    column_families->emplace_back(name, ColumnFamilyOptions());
    i++;
  }
  if (!already_in_list) {
    position = i;
    column_families->emplace_back(_name, ColumnFamilyOptions());
  }
  delete column_names;
  return column_families;
}
void DatabaseEditor::showAllTables() {
  DB* db;
  size_t pos;
  auto* column_families = getTables("", pos);
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(DBOptions(), db_path, *column_families, &handles, &db);
  assert(s.ok());
  for (size_t i = 0; i != column_families->size(); i++) {
    std::cout << "Table name: " << (*column_families)[i].name << std::endl;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), handles[i]);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      std::cout << "\t" << it->key().ToString() << ": "
                << it->value().ToString() << std::endl;
    }
    delete it;
  }
  for (auto handle : handles) db->DestroyColumnFamilyHandle(handle);
  delete column_families;
  delete db;
}
void DatabaseEditor::createTable() {
  string st;
  std::cout << "Enter table name: ";
  std::cin >> st;
  DB* db;
  size_t pos;
  auto* column_families = getTables("", pos);
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(DBOptions(), db_path, *column_families, &handles, &db);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  ColumnFamilyHandle* cf;
  s = db->CreateColumnFamily(ColumnFamilyOptions(), st, &cf);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  delete cf;
  for (auto handle : handles) db->DestroyColumnFamilyHandle(handle);
  delete column_families;
  delete db;
}
void DatabaseEditor::createRandomTable() {
  int key_count;
  std::cout << "Enter keys count: ";
  std::cin >> key_count;
  Randomizer r(854);
  DB* db;
  size_t pos;
  auto* column_families = getTables("", pos);
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(DBOptions(), db_path, *column_families, &handles, &db);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  ColumnFamilyHandle* cf;
  s = db->CreateColumnFamily(ColumnFamilyOptions(), r.get(), &cf);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  handles.push_back(cf);
  int table_id = handles.size() - 1;
  for (int i = 0; i != key_count; i++)
    db->Put(WriteOptions(), handles[table_id], r.get(), r.get());
  for (auto handle : handles) db->DestroyColumnFamilyHandle(handle);
  delete column_families;
  delete db;
}
void DatabaseEditor::deleteAll() {
  DB* db;
  size_t pos;
  auto* column_families = getTables("", pos);
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(DBOptions(), db_path, *column_families, &handles, &db);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  for (size_t i = 1; i != handles.size(); i++) db->DropColumnFamily(handles[i]);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  for (auto handle : handles) db->DestroyColumnFamilyHandle(handle);
  delete column_families;
  delete db;
}
