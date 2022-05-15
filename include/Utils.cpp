//Copyright 2022 wm8

#include "Utils.h"
#include <iomanip>
#include <thread>
#include "NullLogger.h"
bool ParseArgs(Data* data, int argc, char **argv) {
  po::options_description desc("Options:");
  desc.add_options()("log-level,ll", po::value<string>(),
                     "уровень ллоггирования")(
      "thread-count,tc", po::value<unsigned int>(), "кол-во потоков")(
      "input", po::value<std::string>(), "входной файл")(
      "help", "this message")("output,o", po::value<string>(),
                              "полный путь выходного файла");
  po::positional_options_description positionalDescription;
  positionalDescription.add("input", -1);
  po::variables_map map;
  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(desc)
                  .positional(positionalDescription)
                  .run(),
              map);
    po::store(po::parse_command_line(argc, argv, desc), map);
    po::notify(map);
    if (map.count("help")) {
      std::cout << "Usage: options_description [options]\n";
      std::cout << desc;
      return false;
    }
    if (!map.count("thread-count") || !map.count("input") ||
        !map.count("output")) {
      BOOST_LOG_TRIVIAL(fatal) << "Not enough args";
      return false;
    }
    if (map.count("log-level")) {
      const string log_level = map["log-level"].as<std::string>();
      if (log_level == "info") {
        data->log_level = logging::trivial::info;
      } else if (log_level == "warning") {
        data->log_level = logging::trivial::warning;
      } else if (log_level == "error") {
        data->log_level = logging::trivial::error;
      } else {
        BOOST_LOG_TRIVIAL(fatal) << "Unknown log level\n";
        return false;
      }
    } else {
      data->log_level = logging::trivial::error;
    }
    data->thread_count = map["thread-count"].as<unsigned int>();
    data->output_path = map["output"].as<std::string>();
    data->input_path = map["input"].as<std::string>();
  } catch (exception& e) {
    MESSAGE_LOG(data->log_level) << "error: " << e.what();
    return false;
  }
  return true;
}
void InitLogging(/*std::string logfile*/) {
  /*boost::filesystem::absolute("");
  logging::add_file_log(logfile, logging::keywords::auto_flush = true);*/
  logging::add_console_log(std::cout);
}

std::vector<ColumnFamilyDescriptor>* getTables(string db_path) {
  auto* column_families = new std::vector<ColumnFamilyDescriptor>();
  auto* column_names = new std::vector<string>();
  DBOptions options;
  options.create_if_missing = true;
  options.create_missing_column_families = true;
  options.keep_log_file_num = 1;
  options.info_log_level = ::rocksdb::InfoLogLevel::FATAL_LEVEL;
  options.recycle_log_file_num = 1;
  Status s = DB::ListColumnFamilies(options, db_path, column_names);
  if (s.IsIOError())
    column_families->emplace_back(ROCKSDB_NAMESPACE::kDefaultColumnFamilyName,
                                  ColumnFamilyOptions());
  assert(s.ok() || (!s.ok() && s.IsIOError()));
  for (string name : *column_names)
    column_families->emplace_back(name, ColumnFamilyOptions());
  delete column_names;
  return column_families;
}
void writeTask(Data* data) {
  while (!data->values.empty()) {
    data->mutex.lock();
    auto pair = data->values.front();
    data->values.pop();
    data->mutex.unlock();
    string hash;
    picosha2::hash256_hex_string(pair.key + pair.value, hash);
    data->outDb->Put(WriteOptions(), data->outHandles[pair.handle_id], pair.key,
                     hash);
  }
}
void createTables(std::vector<ColumnFamilyDescriptor>* tables, Data* data) {
  for (size_t i = 1; i != tables->size(); i++) {
    ColumnFamilyHandle* cf;
    Status s = data->outDb->CreateColumnFamily(ColumnFamilyOptions(),
                                               (*tables)[i].name, &cf);
    data->outHandles.push_back(cf);
    if (!s.ok())
      MESSAGE_LOG(data->log_level)
          << (*tables)[i].name << ": " << s.ToString() << std::endl;
    assert(s.ok());
  }
}
void readTask(unsigned int id, Data* data) {
  for (size_t i = id; i < data->inpHandles.size(); i += data->thread_count) {
    rocksdb::Iterator* it =
        data->inpDb->NewIterator(rocksdb::ReadOptions(), data->inpHandles[i]);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      data->mutex.lock();
      data->values.push(Value(i, it->key().ToString(), it->value().ToString()));
      data->mutex.unlock();
    }
    delete it;
  }
}
void deleteDB(string& path, std::vector<ColumnFamilyDescriptor>* tables,
              Data* data) {
  DB* db;
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(DBOptions(), path, *tables, &handles, &db);
  if (!s.ok()) MESSAGE_LOG(data->log_level) << s.ToString() << std::endl;
  assert(s.ok());
  for (size_t i = 1; i != handles.size(); i++) {
    s = db->DropColumnFamily(handles[i]);
    if (!s.ok()) MESSAGE_LOG(data->log_level) << s.ToString() << std::endl;
    assert(s.ok());
  }
  tables = new std::vector<ColumnFamilyDescriptor>();
  tables->emplace_back(ROCKSDB_NAMESPACE::kDefaultColumnFamilyName,
                       ColumnFamilyOptions());
  for (auto handle : handles) db->DestroyColumnFamilyHandle(handle);
  delete db;
}
void run(int argc, char **argv) {
  Data* data = new Data();
  InitLogging();
  if (!ParseArgs(data, argc, argv)) return;
  DBOptions options;
  options.create_if_missing = true;
  options.create_missing_column_families = true;
  options.keep_log_file_num = 1;
  options.info_log_level = ::rocksdb::InfoLogLevel::FATAL_LEVEL;
  options.recycle_log_file_num = 1;

  auto tables1 = getTables(data->input_path);
  auto tables2 = getTables(data->output_path);
  if (tables2->size() > 1) {
    MESSAGE_LOG(data->log_level) << "Hash db already exists\nDeleting...\n";
    deleteDB(data->output_path, tables2, data);
  }
  Status s =
      DB::Open(options, data->input_path, *tables1, &(data->inpHandles),
                      &(data->inpDb));
  if (!s.ok()) MESSAGE_LOG(data->log_level) << s.ToString() << std::endl;
  assert(s.ok());
  s = DB::Open(options, data->output_path, *tables2, &(data->outHandles),
               &(data->outDb));
  if (!s.ok()) MESSAGE_LOG(data->log_level) << s.ToString() << std::endl;
  assert(s.ok());
  // Read
  MESSAGE_LOG(data->log_level) << "Reading...\n";
  std::vector<std::thread> threads;
  for (size_t i = 0; i != data->thread_count; ++i)
    threads.emplace_back(readTask, i, data);
  for (unsigned int i = 0; i != data->thread_count; i++)
    if (threads[i].joinable()) threads[i].join();

  // Write
  if (tables2->size() <= 1) {
    tables2 = new std::vector<ColumnFamilyDescriptor>(*tables1);
    createTables(tables2, data);
  }
  MESSAGE_LOG(data->log_level) << "Writing...\n";
  threads.clear();
  for (unsigned int i = 0; i != data->thread_count; i++)
    threads.emplace_back(writeTask, data);
  for (unsigned int i = 0; i != data->thread_count; i++)
    if (threads[i].joinable()) threads[i].join();

  // Show
  MESSAGE_LOG(data->log_level) << "Input DB:\n";
  showDB(data, 30, data->inpDb, data->inpHandles, tables1);
  MESSAGE_LOG(data->log_level) << "Output DB:\n";
  showDB(data, 85, data->outDb, data->outHandles, tables2);

  // Clear
  MESSAGE_LOG(data->log_level) << "Clearing...\n";
  delete tables1;
  delete tables2;
  for (auto handle : data->inpHandles)
    data->inpDb->DestroyColumnFamilyHandle(handle);
  for (auto handle : data->outHandles)
    data->outDb->DestroyColumnFamilyHandle(handle);
  delete data->inpDb;
  delete data->outDb;
  delete data;
  MESSAGE_LOG(data->log_level) << "Done!\n";
}
void showDB(Data* data, int length, DB* db,
            std::vector<ColumnFamilyHandle*> handles,
            std::vector<ColumnFamilyDescriptor>* tables) {
  std::vector<rocksdb::Iterator*> its;
  std::string line = "|";
  for (size_t i = 0; i != tables->size(); ++i)
    line += string(length + 2, '-') + '|';
  for (auto &handle : handles) {
    auto it = db->NewIterator(ReadOptions(), handle);
    it->SeekToFirst();
    its.push_back(it);
  }
  std::stringstream ss;
  // head
  ss << line << "\n| ";
  for (size_t i = 0; i != tables->size(); i++)
    ss << std::setw(length) << std::left << (*tables)[i].name << " | ";
  ss << "\n" << line << "\n";

  size_t its_count_finished = 0;
  while (its_count_finished != its.size()) {
    ss << "| ";
    for (size_t i = 0; i != tables->size(); i++) {
      string key_value = "";
      if (its[i]->Valid()) {
        key_value =
            its[i]->key().ToString() + ": " + its[i]->value().ToString();
        its[i]->Next();
        if (!its[i]->Valid()) its_count_finished++;
      }
      ss << std::setw(length) << std::left << key_value << " | ";
    }
    ss << "\n";
  }
  ss << line << "\n";
  MESSAGE_LOG(data->log_level) << ss.str();
  for (auto &it : its) delete it;
}
