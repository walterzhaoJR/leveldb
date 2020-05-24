# LevelDB源码学习、分析01：初识

* 写在最前面：在开始前，先简单说两句。我叫walter，是一名程序员。一直想找寻一个记录自己平时学习、思考的地方，希望自己可以坚持记录、更新。也很期待和大家的讨论，交流。

## 1 简介

### 1.1 什么是leveldb

* LevelDB is a fast key-value storage library written at Google that provides an ordered mapping from string keys to string values. 这个是[官方](https://github.com/google/leveldb)给出的解释：LevelDB是Google编写的快速键值存储库，提供了从字符串键到字符串值的有序映射。

### 1.2 features

- Keys and values are arbitrary byte arrays.键值都是任意的键值数组。
- Data is stored sorted by key.数据是按照key的顺序来存储。
- Callers can provide a custom comparison function to override the sort order.调用者可以提供自定义的比较函数来覆盖默认的排序顺序。
- The basic operations are `Put(key,value)`, `Get(key)`, `Delete(key)`.提供了put、get、delete三种基本操作。
- Multiple changes can be made in one atomic batch.在一个原子的批处理中进行多次修改。
- Users can create a transient snapshot to get a consistent view of data.用户可以创建一个短暂的快照以获取一致的数据视图。
- Forward and backward iteration is supported over the data.支持正向和反向迭代器。
- Data is automatically compressed using the Snappy compression library.使用Snappy压缩数据。
- External activity (file system operations etc.) is relayed through a virtual interface so users can customize the operating system interactions.外部活动通过虚拟接口进行中继，用户可以自定义操作系统交互。

### 1.3 其他的说明

* 直接使用leveldb 的项目比较少见，最常见的是使用 rocksdb，rocksdb是Facebook基于leveldb的项目，做了 一些优化，性能有些提升，同时提供了更多功能。

## 2.一些基本的操作

* 从github上下载到源码后，进行编译。在编写代码的时候链接leveldb和pthread库，这里就不细说了。

### 2.1 创建、关闭db

```c++
#include <iostream>
#include "leveldb/db.h"

using namespace std;
using namespace leveldb;

int main()
{
  DB* db;
  Options options;//db的一些选项
  options.create_if_missing = true;//如果指定数据库不存在，则创会建一个
  Status status = DB::Open(options, "./tmp", &db);//第二个参数是你要创建的db文件的位置
  if (status.ok()) {
    cout << "create leveldb succ" << endl;
  } else {
    cerr << status.ToString() << endl;
  }

  delete db;//关闭db
  db = nullptr;

  return 0;
}
```

* Status是leveldb大多数函数的返回类型，用来检查函数的执行情况，和错误信息输出。
* 在./tmp目录下我们会看到一些leveldb锁创建的文件，这些文件具体的作用，我会在后边的章节解释。

```linux
-rw-r--r-- 1 root root    0 May 24 03:32 000003.log
-rw-r--r-- 1 root root   16 May 24 03:32 CURRENT
-rw-r--r-- 1 root root    0 May 24 03:32 LOCK
-rw-r--r-- 1 root root   60 May 24 03:32 LOG
-rw-r--r-- 1 root root   50 May 24 03:32 MANIFEST-000002
```

### 2.2基本的读写操作

```c++
#include <iostream>
#include "leveldb/db.h"

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
  } else {
    cout << "read key from leveldb failed:" << status.ToString() << endl;
  }

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

  delete db;//关闭db
  db = nullptr;

  return 0;
}
```

* 执行结果，我们可以看到再次读取被删除的记录会报错，这里使用的也是Status这个结构

```linux
[root@c18b3a8b5f04 code]# ./main
create leveldb succ
write key into leveldb succ;
read key from leveldb succ key:name value:walter
leveldb delete record succread key from leveldb failed:NotFound:
```

* 这个时候我们再看下我们DB的目录：

```linux
-rw-r--r-- 1 root root  122 May 24 04:55 000007.ldb
-rw-r--r-- 1 root root  122 May 24 04:58 000010.ldb
-rw-r--r-- 1 root root   57 May 24 04:58 000011.log
-rw-r--r-- 1 root root   16 May 24 04:58 CURRENT
-rw-r--r-- 1 root root    0 May 24 03:32 LOCK
-rw-r--r-- 1 root root  326 May 24 04:58 LOG
-rw-r--r-- 1 root root  324 May 24 04:55 LOG.old
-rw-r--r-- 1 root root  110 May 24 04:58 MANIFEST-000009
```

* 这里我们看到比刚才2.1节中多了一些ldb文件，这个就是leveldb的sst（Sorted Strings Table）文件。ldb文件在老版本之前就直接是*.sst这类文件。xxx.ldb文件中的KV记录是按key的大小排好序的。随着数据量增加，ldb文件有很多。但是，它们文件名前缀数字、文件大小都与所属的level无联系（文件名即文件大小都不包含level语义），所以我们无法从文件名和文件大小判断出某个文件中数据所处的level。不过，一个文件只可能属于一个level。

### 2.3 原子更新

* 我们这里先看一个操作：

```c++
std::string value;
leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value);
if (s.ok()) s = db->Put(leveldb::WriteOptions(), key2, value);
if (s.ok()) s = db->Delete(leveldb::WriteOptions(), key1);
```

* 假设在put key2和delete key1之间进程退出，那么可能会存在多个key有相同的value。leveldb提供了WrietBatch这个api来完成一系列的更新。如果失败，一次WriteBatch的 数据都丢失，如果成功，一次WriteBatch的数据都更新成功。此外，WriteBatch还可以提升写入效率，多个更新操作合并到WriteBatch会 快一些。

```c++
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

  delete db;//关闭db
  db = nullptr;

  return 0;
}
```



### 2.4 Synchronous Writes 同步写

* 在默认的情况下，leveldb 的写入操作是异步的：把写操作从进程推送到操作系统后就返回。从操作系统内存到底层持久化存储的传输是异步的。同步标志可以为一个特定的写操作打开，使这个写操作直到数据被写入到持久化存储器中才返回。（在 Posix 系统上，这是通过在写操作返回前调用 `fsync(...)` 或 `fdatasync(...)` 或 `msync(..., MS_SYNC)` 来实现的。）

```c++
leveldb::WriteOptions write_options;
write_options.sync = true;
db->Put(write_options, ...);
```

* 异步写的速度通常是同步写的1000倍。异步写入的缺点是，系统的崩溃可能会导致最后的一些更新丢失。请注意，只是写过程(例如，不是重启)的崩溃不会导致任何的丢失，因为即使关闭了同步写，一个更新操作会在它被认为完成之前，从进程的内存推送到操作系统。
* 不过异步写入通常可以安全地使用。例如，当导入大量的数据到数据库时，你可以在崩溃后重启批处理导入程序来弥补丢失的更新。混合方案也是可能的，可以每隔N个异步写操作加入一个同步写操作，同步写操作写入一个标志，表示这之前的数据都完整写入了，在崩溃发生时，批处理导入程序只会在最后一个同步写在上一次的运行中完成之后被重启。（同步写入可以更新标记，该标记描述崩溃时从何处重新启动。）
* 此外`WriteBatch` 提供了一个异步写的替代品。多个更新也许会被放在相同的 WriteBatch 上，并用同步写入(把`write_options.sync`设置为`true`)一起执行。同步写入的额外开销将会平摊在批处理的所有写操作上。

### 2.5 Concurrency 并发操作

* 一个DB在同一时刻只能被一个进程打开。在 leveldb implementation中，通过从操作系统获取锁来防止误用。在进程程中，相同的 `leveldb:DB` 对象可以被多个并发的线程安全地共享。即，不同的线程可以在不需要外部同步信号的情况下，在相同的数据库上写入或者读取迭代器或者调用Get（leveldb 的实现会自动地进行所需的同步）。然而，其他的对象(像迭代器和WriteBatch)也许需要外部同步。如果两个线程共享这样的一个对象，他们必须使用自己的锁协议保护地访问。

### 2.6 Iteration 迭代器

* leveldb存储是有序的，可以使用迭代器依次访问数据。下面的例子演示了如何打印一个数 据库中的所有key value对。

```c++
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

  leveldb::WriteBatch batch;
  /*注意这里我插入的顺序，再观察下一会的输出顺序*/
  batch.Put("abcd", "1122");
  batch.Put("1", "2");
  batch.Put("abc", "123");
  batch.Put("01", "test");
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
```

* 我们可以看到通过迭代器打印db中的所有key，value，并且存储的顺序是按照key的大小排序顺序。

```c++
[root@c18b3a8b5f04 code]# ./main
create leveldb succ
leveldb write batch succ
key:01 value:test
key:1 value:2
key:abc value:123
key:abcd value:1122
key:name value:walter
key:old_name value:walter
```

* 还可以选取一个范围进行迭代，这个范围是：[start,limit)

```c++
for (it->Seek(start);
   it->Valid() && it->key().ToString() < limit;
   it->Next()) {
  ...
}
```

* 也只支持逆序迭代的，不过就是性能会差一些：

```c++
for (it->SeekToLast(); it->Valid(); it->Prev()) {
  ...
}
```



* 本篇简单罗列了leveldb的一些基本的操作，后续我会继续介绍leveldb的使用、数据结构等细节。