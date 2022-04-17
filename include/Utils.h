//Copyright 2022 wm8

#ifndef LAB10_UTILS_H
#define LAB10_UTILS_H
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/filesystem.hpp>
#include "Data.h"
#include "rocksdb/db.h"
#include "picosha2.h"
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
namespace logging = boost::log;
namespace po = boost::program_options;
using std::string;
using std::exception;

bool ParseArgs(Data* data, int argc, char **argv);
void InitLogging();
std::vector<ColumnFamilyDescriptor>* getTables(string db_path);
void deleteDB(string& path, std::vector<ColumnFamilyDescriptor>* tables, Data* data);
void readTask(unsigned int id, Data* data);
void createTables(std::vector<ColumnFamilyDescriptor>* tables, Data* data);
void writeTask(Data* data);
void run(int argc, char **argv);
void showDB(Data* data, int length, DB* db, std::vector<ColumnFamilyHandle*> handles, std::vector<ColumnFamilyDescriptor>* tables);
#endif  // LAB10_UTILS_H
