//Copyright 2022 wm8

#ifndef LAB10_DATA_H
#define LAB10_DATA_H
#include <string>
#include <queue>
#include "rocksdb/db.h"
#include <boost/log/trivial.hpp>
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::DBOptions;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
struct Value
{
 public:
  Value(size_t id, std::string k, std::string v)
  {
    handle_id = id;
    key = k;
    value = v;
  }
  size_t handle_id;
  std::string key;
  std::string value;
};
struct Data
{
  boost::log::trivial::severity_level log_level;
  unsigned int thread_count;
  std::string output_path;
  std::string input_path;
  DBOptions options;
  DB* inpDb;
  DB* outDb;
  std::vector<ColumnFamilyHandle*> inpHandles;
  std::vector<ColumnFamilyHandle*> outHandles;
  std::queue<Value> values;
  std::mutex mutex;
};
#endif  // LAB10_DATA_H
