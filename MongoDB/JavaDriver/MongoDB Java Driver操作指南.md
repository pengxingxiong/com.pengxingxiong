#基本操作

### 安装MongoDB Java Driver

使用maven的用户在pom.xml中使用以下的dependency。

```xml
<dependency>
   <groupId>org.mongodb</groupId>
   <artifactId>mongodb-driver</artifactId>
   <version>3.1.0-SNAPSHOT</version>
</dependency>
<dependency>
    <groupId>org.mongodb</groupId>
    <artifactId>mongo-java-driver</artifactId>
    <version>3.1.0-SNAPSHOT</version>
</dependency>
```

### 建立连接

程序可以通过MongoClient这个类和MongoDB数据库建立连接。

```java
MongoClient mongoClient = new MongoClient();
// or
MongoClient mongoClient = new MongoClient( "localhost" );
// or
MongoClient mongoClient = new MongoClient( "localhost" , 27017 );

// or, 连接副本集，MongoClient会自动识别出
MongoClient mongoClient = new MongoClient(
  Arrays.asList(new ServerAddress("localhost", 27017),
                new ServerAddress("localhost", 27018),
                new ServerAddress("localhost", 27019)));
MongoDatabase database = mongoClient.getDatabase("mydb");
```

如果mydb不存在的话，那么Mongo也会给我们新创建一个数据库。

MongoDatabase的本质就是一个数据库的连接，而MongoClient是一个Client，所以我们在应用中可能需要多个连接，却只需要创建一个MongoClient就可以了，MongoClient本身封装了一个连接池。关于MongoClient，后面再补一篇文章。

### 获得集合

在得到连接之后，通过getCollection()方法来获取，相同地，如果获取的集合不存在，那么Mongo会为我们创建这个集合。

```java
MongoCollection<Document> collection = database.getCollection("users");
```

### 插入一个文档

在MongoDB中，数据都是以文档的形式存在的，一个集合可以理解成一个“文档链”。

在获得集合之后，程序就可以通过集合来插入一个新的数据了。

在Mongo Java API中，文档对应的类是Document , 文档中间还可以内嵌文档。

比如插入这样的数据

```java
{
    "username" : "whthomas",
    "age" : "22",
    "location":{
        "city" : "hangzhou",
        "x" : 100,
        "y" : 200
    }
}
Document doc = new Document("username","whthomas").append("age", "22").append("location", new Document("city", "hangzhou").append("x", 100).append("y","200"));
collection.insertOne(doc);
```

如果要插入多个文档。使用insertMany()函数效率会更高一些，这个函数接受一个List< Document >类型。

```java
List<Document> documents = new ArrayList<Document>();
for (int i = 0; i < 100; i++) {
    documents.add(new Document("i", i));
}
collection.insertMany(documents);
```

### 查询操作

查询操作数据库操作中相对比较复杂的操作，在MongoDB中通过集合的find()方法来查询数据。

find()函数会返回FindIterable，它提供了一个接口给程序操作和控制这个集合。

#### 得到第一条数据

使用first()函数可以得到结果集中的第一条数据，如果没有找到数据，则返回一个null值。

```java
Document myDoc = collection.find().first();
```

#### 得到所有数据

通过iterator方法将FindIterable对象转换成一个MongoCursor对象。

```java
MongoCursor<Document> cursor = collection.find().iterator();
try {
    while (cursor.hasNext()) {
    System.out.println(cursor.next().toJson());
    }
} finally {
    cursor.close();
}
```

#### 条件查询

MongoDB的Java条件查询操作，在我看来有些不那么面向对象，写起来有点函数式编程的味道。

通过Filters、Sorts和Projections三个类，我们可以完成复杂的查询操作。

比如程序需要得到一个指定条件的数据

```java
import static com.mongodb.client.model.Filters.*;

Document myDoc = collection.find(eq("i", 71)).first();
```

往find函数中“传递”一个eq函数，得到i为71的数据。

过滤条件函数
|条件|函数|例子 |
|----|----|---|
|等于 |eq()| eq("i",50)|
|大于 |gt()| gt("i",50)|
|小于 |lt()| lt("i",50)|
|大于等于(>=)|gte()| gte("i",50)|
|小于等于(<=) |lte()| lte("i",50)|
|存在 |exists()| exists("i")|

#### 排序操作

对FindIterable对象使用sort函数进行排序。ascending函数表示升序，descending函数表示降序。

```java
collection.find().sort(orderBy(ascending("x", "y"), descending("z")))
```

#### 过滤字段

有时候，我们并不需要一条数据中所有的内容，只是需要一部分而已，mongoDB 提供了一个projection方法，解决了这个问题。

```java
collection.find().projection(fields(include("x", "y"), excludeId()))
```

#### 使用forEach

有时候对不同的集合会有相同的操作，做通用方法是最佳实践，Mongo对于函数式的编程范式真是充满了热情，为我们提供了大量非常“函数式”的方法（在Java这种完全面向对象的语言里，做到这样真是不容易）。

我们可以通过forEach函数和Block类完成对集合中每个节点数据的操作。

```java
Block<Document> printBlock = new Block<Document>() {
     @Override
     public void apply(final Document document) {
         System.out.println(document.toJson());
     }
};
collection.find(gt("i", 50)).forEach(printBlock);
```

### 更新数据

使用updateOne()函数，更新一条数据，第一个参数选取需要被更新的记录，第二个参数设置需要被更新的**具体数据**。

```java
collection.updateOne(eq("i", 10), new Document("$set", new Document("i", 110)));
```

如果需要更新多条数据，可以使用updateMany函数，这个函数会返回一个UpdateResult类的对象，这个对象里面保存了数据更新的结果。

```java
UpdateResult updateResult = collection.updateMany(lt("i", 100),
          new Document("$inc", new Document("i", 100)));
```

### 删除数据

通过集合使用deleteOne()方法来删除指定的数据，如果想要删除多条数据，使用deleteMany方法来完成操作.

```java
collection.deleteOne(eq("i", 110));
DeleteResult deleteResult = collection.deleteMany(gte("i", 100));
```

### Bulk操作

MongoDB提供了一种称为Bulk的操作方式，数据不会被立即被持久化到数据库中，而是等待程序调度，确定合适的时间持久化到数据库中。

Bulk操作支持有序操作，和无序操作两种模式。有序操作中数据的的操作，会按照顺序操作，一旦发生错误，操作就会终止；而无序操作，则不安顺序执行，只会报告哪些操作发生了错误。

Bulk操作支持增删改三种操作，对应的model分别是InsertOneModel、UpdateOneModel、DeleteOneModel、ReplaceOneModel。它们都继承于WriteModel

```java
// 有序操作
collection.bulkWrite(
  Arrays.asList(new InsertOneModel<>(new Document("_id", 4)),
                new InsertOneModel<>(new Document("_id", 5)),
                new InsertOneModel<>(new Document("_id", 6)),
                new UpdateOneModel<>(new Document("_id", 1),
                                     new Document("$set", new Document("x", 2))),
                new DeleteOneModel<>(new Document("_id", 2)),
                new ReplaceOneModel<>(new Document("_id", 3),
                                      new Document("_id", 3).append("x", 4))));
 // 无序操作
collection.bulkWrite(
  Arrays.asList(new InsertOneModel<>(new Document("_id", 4)),
                new InsertOneModel<>(new Document("_id", 5)),
                new InsertOneModel<>(new Document("_id", 6)),
                new UpdateOneModel<>(new Document("_id", 1),
                                     new Document("$set", new Document("x", 2))),
                new DeleteOneModel<>(new Document("_id", 2)),
                new ReplaceOneModel<>(new Document("_id", 3),
                                      new Document("_id", 3).append("x", 4))),
  new BulkWriteOptions().ordered(false));
```