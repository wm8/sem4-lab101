//Copyright 2022 vlados2003

#ifndef LAB10_DATABASEEDITOR_H
#define LAB10_DATABASEEDITOR_H
#include <string>
#include <vector>
#include "Randomizer.h"
#include "rocksdb/db.h"
using ROCKSDB_NAMESPACE::ColumnFamilyDescriptor;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
using ROCKSDB_NAMESPACE::ColumnFamilyOptions;
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::DBOptions;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;
using std::string;
class DatabaseEditor {
 public:
  explicit DatabaseEditor(std::string path);
 private:
  void addValue();
  void showTable();
  void showAllTables();
  void createTable();
  void createRandomTable();
  void deleteAll();
  void changePath();
  std::vector<ColumnFamilyDescriptor>* getTables(string name, size_t& position);
  std::string db_path;
  Options options;
};

#endif  // LAB10_DATABASEEDITOR_H
