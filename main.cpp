#include <iostream>
#include <memory>

#include <rocksdb/db.h>

#include <ndn-cxx/name.hpp>

std::unique_ptr<rocksdb::DB> OpenDB() {
  rocksdb::DB* db;
  rocksdb::Options options;
  options.create_if_missing = true;
  auto status = rocksdb::DB::Open(options, "/tmp/testdb", &db);
  if (status.ok())
    return std::unique_ptr<rocksdb::DB>(db);
  else
    return nullptr;
}

int main() {
  auto db = OpenDB();

  ndn::Name n1("/a/b1/1");
  ndn::Name n2("/a/b1/2");
  ndn::Name n3("/a/b2/0");

  auto insert_name = [&db](const ndn::Name& n) {
    const auto& block = n.wireEncode();
    std::string uri = n.toUri();
    rocksdb::Slice key{reinterpret_cast<const char*>(block.value()),
                       block.value_size()};
    db->Put({}, key, uri);
  };

  insert_name(n1);
  insert_name(n2);
  insert_name(n3);

  auto get_name = [&db](const ndn::Name& n) {
    const auto& block = n.wireEncode();
    rocksdb::Slice key{reinterpret_cast<const char*>(block.value()),
                       block.value_size()};
    std::string value;
    db->Get({}, key, &value);
    return value;
  };

  std::cout << "n2: " << get_name(n2) << std::endl;

  {
    std::unique_ptr<rocksdb::Iterator> iter(db->NewIterator({}));
    std::cout << "iterate through: " << std::endl;
    for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
      auto key = iter->key();
      auto value = iter->value();
      std::cout << key.ToString(true) << ':' << value.ToString() << std::endl;
    }
  }

  auto lower_bound = [&db](const ndn::Name& prefix) {
    const auto& block = prefix.wireEncode();
    rocksdb::Slice key{reinterpret_cast<const char*>(block.value()),
                       block.value_size()};
    std::unique_ptr<rocksdb::Iterator> iter(db->NewIterator({}));
    iter->Seek(key);
    if (iter->Valid()) {
      auto value = iter->value();
      std::cout << "lower_bound: " << value.ToString() << std::endl;
    } else {
      std::cout << "lower_bound: not found" << std::endl;
    }
  };

  auto upper_bound = [&db](const ndn::Name& prefix) {
    auto pnext = prefix.getSuccessor();
    const auto& block = pnext.wireEncode();
    rocksdb::Slice key{reinterpret_cast<const char*>(block.value()),
                       block.value_size()};
    std::unique_ptr<rocksdb::Iterator> iter(db->NewIterator({}));
    iter->Seek(key);
    if (iter->Valid() && (iter->Prev(), iter->Valid())) {
      auto value = iter->value();
      std::cout << "upper_bound: " << value.ToString() << std::endl;
    } else {
      std::cout << "upper_bound: not found" << std::endl;
    }
  };

  ndn::Name n4("/a/b1");
  lower_bound(n4);
  upper_bound(n4);

  ndn::Name n5("/a/b2");
  lower_bound(n5);
  upper_bound(n5);

  ndn::Name n6("/a/b");
  lower_bound(n6);
  upper_bound(n6);

  return 0;
}
