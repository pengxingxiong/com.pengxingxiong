# 2017-9-1-HttpClient使用方法 #
```java
public class UrlUtils {
    /**
     * 使用HttpClient请求数据
     */
    static String doGet(String url, String charset) {

        HttpClient httpclient = new DefaultHttpClient();
        // 准备请求
        HttpGet httpget = new HttpGet(url);
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
            e.printStackTrace();
        }
        return result;
    }

    /** 发送post请求,获取json数据 */
    public static JSONObject doPost(String url, Map<String, String> map, String charset) {
        HttpClient httpClient;
        HttpPost httpPost;
        String result = null;
        try {
            httpClient = new DefaultHttpClient();
            httpPost = new HttpPost(url);
            // 设置参数
            List<NameValuePair> list = new ArrayList<>();
            for (Entry<String, String> elem : map.entrySet()) {
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
            ex.printStackTrace();
        }
        return JSONObject.parseObject(result);
    }
}
```