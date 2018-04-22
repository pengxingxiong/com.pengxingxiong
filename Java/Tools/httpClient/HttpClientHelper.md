```java
package utils;

import com.alibaba.fastjson.JSONObject;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.util.EntityUtils;
import org.apache.log4j.Logger;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
/**
 * @author 彭行雄0016004591
 * @date 创建时间：2017/8/22
 */
public class HttpClientHelper {
    private static Logger log = Logger.getLogger(HttpClientHelper.class);
    /**
     * 使用HttpClient请求数据
     */
    public static String doGet(String url, String charset) {
        Map<String,String> header = new HashMap<>();
        return doGet(url, charset,header);
    }

    /** 发送post请求,获取json数据 */
    public static JSONObject doPost(String url, Map<String, String> map, String charset) {
        Map<String,String> header = new HashMap<>();
        return doPost(url,map,charset,header);
    }
    /**
     * 使用HttpClient请求数据，补充请求头
     */
    public static String doGet(String url, String charset,Map<String,String> header) {

        HttpClient httpclient = new DefaultHttpClient();
        // 准备请求
        HttpGet httpget = new HttpGet(url);
        if (!header.isEmpty()){
            for (Map.Entry<String,String> entry : header.entrySet()){
                httpget.setHeader(entry.getKey(),entry.getValue());
            }
        }
        // 执行请求
        HttpResponse response;
        String result = null;
        try {
            response = httpclient.execute(httpget);
            // 检查响应状态
            if (response != null) {
                HttpEntity resEntity = response.getEntity();
                if (resEntity != null) {
                    result = EntityUtils.toString(resEntity, charset);
                }
            }
        } catch (IOException e) {
            log.error("GET请求url:"+url+"发生错误,请求头为："+header);
            e.printStackTrace();
        }
        return result;
    }
    /** 发送post请求,获取json数据，补充请求头 */
    public static JSONObject doPost(String url, Map<String, String> map, String charset,Map<String,String> header) {
        HttpClient httpClient;
        HttpPost httpPost;

        String result = null;
        try {
            httpClient = new DefaultHttpClient();
            httpPost = new HttpPost(url);
            //设置头
            if (!header.isEmpty()){
                for (Map.Entry<String,String> entry : header.entrySet()){
                    httpPost.setHeader(entry.getKey(),entry.getValue());
                }
            }
            // 设置参数
            List<NameValuePair> list = new ArrayList<>();
            for (Map.Entry<String, String> elem : map.entrySet()) {
                list.add(new BasicNameValuePair(elem.getKey(), elem.getValue()));
            }
            if (list.size() > 0) {
                UrlEncodedFormEntity entity = new UrlEncodedFormEntity(list, charset);
                httpPost.setEntity(entity);
            }
            HttpResponse response = httpClient.execute(httpPost);
            if (response != null) {
                HttpEntity resEntity = response.getEntity();
                if (resEntity != null) {
                    result = EntityUtils.toString(resEntity, charset);
                }
            }
        } catch (Exception ex) {
            log.error("POST请求url:"+url+"发生错误");
            ex.printStackTrace();
        }
        return JSONObject.parseObject(result);
    }

}

```