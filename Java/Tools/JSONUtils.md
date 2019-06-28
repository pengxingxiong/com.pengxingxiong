现在我们就来研究一下fastJson。

Fastjson是国内著名的电子商务互联网公司阿里巴巴内部开发的用于java后台处理json格式数据的一个工具包，包括“序列化”和“反序列化”两部分，它具备如下特征：

1）.速度最快，[测试](http://lib.csdn.net/base/softwaretest)表明，fastjson具有极快的性能，超越任其他的java json parser。包括自称最快的jackson。

2）.功能强大，完全支持java bean、集合、Map、日期、Enum，支持范型，支持自省。

3）.无依赖，能够直接运行在Java SE 5.0以上版本

4）.支持[Android](http://lib.csdn.net/base/android)。

5）.开源 (Apache 2.0)

下面给出fastJson处理json数据格式的代码样例：

```java
package fastJson.test;  
  
import com.alibaba.fastjson.JSON;  
import com.alibaba.fastjson.JSONObject;  
import com.alibaba.fastjson.serializer.SerializerFeature;  
  
public class FastJsonTest  
{  
  
    /** 
     * 序列化 
     */  
    public void toJsonString()  
    {  
        User user = new User("testFastJson001", "maks", 105);  
        String text = JSON.toJSONString(user);  
        System.out.println("toJsonString()方法：text=" + text);  
        // 输出结果：text={"age":105,"id":"testFastJson001","name":"maks"}  
    }  
  
    /** 
     * 反序列化为json对象 
     */  
    public void parseJsonObject()  
    {  
        String text = "{\"age\":105,\"id\":\"testFastJson001\",\"name\":\"maks\"}";  
        JSONObject json = JSON.parseObject(text);  
        System.out.println("parseJsonObject()方法：json==" + json);  
        // 输出结果：json=={"age":105,"id":"testFastJson001","name":"maks"}  
    }  
  
    /** 
     * 反序列化为javaBean对象 
     */  
    public void parseBeanObject()  
    {  
        String text = "{\"age\":105,\"id\":\"testFastJson001\",\"name\":\"maks\"}";  
        User user = (User) JSON.parseObject(text, User.class);  
        System.out.println("parseBeanObject()方法：user==" + user.getId() + "," + user.getName() + "," + user.getAge());  
        // 输出结果：user==testFastJson001,maks,105  
    }  
  
    /** 
     * 将javaBean转化为json对象 
     */  
    public void bean2Json()  
    {  
        User user = new User("testFastJson001", "maks", 105);  
        JSONObject jsonObj = (JSONObject) JSON.toJSON(user);  
        System.out.println("bean2Json()方法：jsonObj==" + jsonObj);  
        // 输出结果：jsonObj=={"age":105,"id":"testFastJson001","name":"maks"}  
    }  
  
    /** 
     * 全序列化 直接把java bean序列化为json文本之后，能够按照原来的类型反序列化回来。支持全序列化，需要打开SerializerFeature.WriteClassName特性 
     */  
    public void parseJSONAndBeanEachother()  
    {  
        User user = new User("testFastJson001", "maks", 105);  
        SerializerFeature[] featureArr = { SerializerFeature.WriteClassName };  
        String text = JSON.toJSONString(user, featureArr);  
        System.out.println("parseJSONAndBeanEachother()方法：text==" + text);  
        // 输出结果：text=={"@type":"fastJson.test.User","age":105,"id":"testFastJson001","name":"maks"}  
        User userObj = (User) JSON.parse(text);  
        System.out.println("parseJSONAndBeanEachother()方法：userObj==" + userObj.getId() + "," + userObj.getName() + "," + userObj.getAge());  
        // 输出结果：userObj==testFastJson001,maks,105  
    }  
  
    public static void main(String[] args)  
    {  
        FastJsonTest test = new FastJsonTest();  
        // 序列化  
        test.toJsonString();  
        // 反序列化为json对象  
        test.parseJsonObject();  
        // 反序列化为javaBean对象  
        test.parseBeanObject();  
        // 将javaBean转化为json对象  
        test.bean2Json();  
        // 全序列化  
        test.parseJSONAndBeanEachother();  
    }  
}  
```

控制台输出结果：

```shell
toJsonString()方法：text={"age":105,"id":"testFastJson001","name":"maks"}  
parseJsonObject()方法：json=={"age":105,"id":"testFastJson001","name":"maks"}  
parseBeanObject()方法：user==testFastJson001,maks,105  
bean2Json()方法：jsonObj=={"age":105,"id":"testFastJson001","name":"maks"}  
parseJSONAndBeanEachother()方法：text=={"@type":"fastJson.test.User","age":105,"id":"testFastJson001","name":"maks"}  
parseJSONAndBeanEachother()方法：userObj==testFastJson001,maks,105  
```

附：javaBean类User.java

```java
package fastJson.test;  
  
import java.io.Serializable;  
  
public class User implements Serializable {  
      
    private static final long serialVersionUID = 1L;  
      
    private String id;  
    private String name;  
    private int age;  
      
    public User() {  
        super();  
    }  
  
    public User(String id, String name, int age) {  
        super();  
        this.id = id;  
        this.name = name;  
        this.age = age;  
    }  
  
    public int getAge() {  
        return age;  
    }  
  
    public void setAge(int age) {  
        this.age = age;  
    }  
  
    public String getId() {  
        return id;  
    }  
  
    public void setId(String id) {  
        this.id = id;  
    }  
  
    public String getName() {  
        return name;  
    }  
  
    public void setName(String name) {  
        this.name = name;  
    }  
      
}  
```

