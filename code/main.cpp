#include <iostream>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"

using namespace std;
using namespace leveldb;

int main()
{
  DB* db;
  Options options;//db的一些选项
  options.create_if_missing = true;//如果指定数据库不存在，则创 建一个
  Status status = DB::Open(options, "./tmp", &db);//第二个参数是你要创建的db文件de位置
  if (status.ok()) {
    cout << "create leveldb succ" << endl;
  } else {
    cerr << status.ToString() << endl;
  }

 #if 0
  std::string key1 = "name";
  std::string value1 = "walter";
  
  //write
  status = db->Put(leveldb::WriteOptions(), key1, value1);
  if (status.ok()) {
    cout << "write key into leveldb succ;" << endl;
  } else {
    cout << "write key into leveldb failed:" << status.ToString() << endl;
  }
 
  //read
  std::string value;
  status = db->Get(leveldb::ReadOptions(), key1, &value);
  if (status.ok()) {
    cout << "read key from leveldb succ key:" << key1 << " value:" << value << endl;
    std::string key2 = "old_name";
    leveldb::WriteBatch batch;

    batch.Delete(key1);
    batch.Put(key2, value);
    status = db->Write(leveldb::WriteOptions(), &batch);
    if (status.ok()) {
      cout << "leveldb write batch succ" << endl;
    } else {
      cout << "leveldb write batch failed " << status.ToString() << endl;
    }
  } else {
    cout << "read key from leveldb failed:" << status.ToString() << endl;
  }
  #endif
  #if 0
  //delete
  status = db->Delete(leveldb::WriteOptions(), key1);//leveldb中没有直接的删除，写实写入一条记录，类型标记为delete，后续做处理
  if (status.ok()) {
    cout << "leveldb delete record succ" << endl;
  } else {
    cout << "delete key from leveldb failed:" << status.ToString() << endl;
  }

  //read after delete
  value = "";
  status = db->Get(leveldb::ReadOptions(), key1, &value);
  if (status.ok()) {
    cout << "read key from leveldb succ key:" << key1 << " value:" << value << endl;
  } else {
    cout << "read key from leveldb failed:" << status.ToString() << endl;
  }
  # endif

  leveldb::WriteBatch batch;
  batch.Put("1", "2");
  batch.Put("abc", "123");
  batch.Put("01", "test");
  batch.Put("abcd", "1122");
  batch.Put("name", "walter");
  status = db->Write(leveldb::WriteOptions(), &batch);
  if (status.ok()) {
    cout << "leveldb write batch succ" << endl;
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      cout << "key:" << it->key().ToString() << " value:" << it->value().ToString() << endl;
      if (!status.ok()) {
        cout << "leveldb iterator failed " << status.ToString() << endl;
        break;
      }
    }

    if (nullptr != it) {
      delete it;
      it = nullptr;
    }
  } else {
    cout << "leveldb write batch failed " << status.ToString() << endl;
  }
  
  delete db;//关闭db
  db = nullptr;

  return 0;
}
