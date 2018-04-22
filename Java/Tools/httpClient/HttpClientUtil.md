# http请求代理类
```java
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.commons.httpclient.Cookie;
import org.apache.commons.httpclient.Header;
import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpConnectionManager;
import org.apache.commons.httpclient.HttpMethod;
import org.apache.commons.httpclient.HttpStatus;
import org.apache.commons.httpclient.MultiThreadedHttpConnectionManager;
import org.apache.commons.httpclient.NameValuePair;
import org.apache.commons.httpclient.cookie.CookiePolicy;
import org.apache.commons.httpclient.cookie.CookieSpec;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.httpclient.methods.PostMethod;
import org.apache.commons.httpclient.methods.RequestEntity;
import org.apache.commons.httpclient.methods.StringRequestEntity;
import org.apache.commons.httpclient.params.HttpConnectionManagerParams;
import org.apache.commons.httpclient.params.HttpMethodParams;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.http.client.methods.HttpPost;

import net.sf.json.JSONObject;

public class HttpClientUtil {
    /**
     * 日志处理类
     */
    private static final Log log = LogFactory.getLog(HttpClientUtil.class);

    // 读取超时
    private final static int SOCKET_TIMEOUT = 10000;
    // 连接超时
    private final static int CONNECTION_TIMEOUT = 10000;
    // 每个HOST的最大连接数量
    private final static int MAX_CONN_PRE_HOST = 20;
    // 连接池的最大连接数
    private final static int MAX_CONN = 100;
    // 连接池
    private final static HttpConnectionManager httpConnectionManager;

    static {
        httpConnectionManager = new MultiThreadedHttpConnectionManager();
        HttpConnectionManagerParams params = httpConnectionManager.getParams();
        params.setConnectionTimeout(CONNECTION_TIMEOUT);
        params.setSoTimeout(SOCKET_TIMEOUT);
        params.setDefaultMaxConnectionsPerHost(MAX_CONN_PRE_HOST);
        params.setMaxTotalConnections(MAX_CONN);
    }

    /**
     * Http get请求，获取结果.
     *
     * @param url
     * @param ip
     * @return
     */
    public static String doHttpGetRequest(String url) {
        HttpClient httpClient = new HttpClient(httpConnectionManager);
        resetRequestHeader(httpClient);
        HttpMethod method = new GetMethod(url);
        String response = null;
        try {
            httpClient.executeMethod(method);
            if (method.getStatusCode() == HttpStatus.SC_OK) {
                response = method.getResponseBodyAsString();
            }
        } catch (IOException e) {
            log.error("执行HTTP Get请求" + url + "时，发生异常！", e);
        } finally {
            method.releaseConnection();
        }
        return response;
    }

    /**
     * Http post请求，获取结果.
     *
     * @param url
     * @param ip
     * @return
     */
    public static String doHttpPostRequest(String url, JSONObject json) {
        HttpClient httpClient = new HttpClient();
        resetRequestHeader(httpClient);
        PostMethod method = new PostMethod(url);
        if (json != null) {
            String param = json.toString();
            RequestEntity se = null;
            try {
                se = new StringRequestEntity(param, "application/json",
                        Constant.encoding);
            } catch (UnsupportedEncodingException e1) {
                log.error("fail to encode param");
            }
            method.setRequestEntity(se);
        }
        String response = null;
        try {
            int status = httpClient.executeMethod(method);
            response = method.getResponseBodyAsString();
        } catch (IOException e) {
            log.error("执行HTTP Post请求" + url + "时，发生异常！", e);
        } finally {
            method.releaseConnection();
        }
        return response;
    }

    public static String doHttpPostRequestMap(String url,
                                              Map<String, String> params) {
        HttpClient httpClient = new HttpClient();
        resetRequestHeader(httpClient);
        PostMethod method = new PostMethod(url);

        List<NameValuePair> param = new ArrayList<>();
        if (params != null) {
            for (Map.Entry<String, String> entry : params.entrySet()) {
                param.add(new NameValuePair(entry.getKey(), entry.getValue()));
            }
            method.setRequestBody(param.toArray(new NameValuePair[param.size()]));
        }
        String response = null;
        try {
            int status = httpClient.executeMethod(method);
            response = method.getResponseBodyAsString();
        } catch (IOException e) {
            log.error("执行HTTP Post请求" + url + "时，发生异常！", e);
        } finally {
            method.releaseConnection();
        }
        return response;
    }

    public static Cookie[] loginCookie(String url, JSONObject json) {
        HttpClient httpClient = new HttpClient();
        resetRequestHeader(httpClient);
        PostMethod method = new PostMethod(url);

        String param = json.toString();
        RequestEntity se = null;
        try {
            se = new StringRequestEntity(param, "application/json",
                    Constant.encoding);
        } catch (UnsupportedEncodingException e1) {
            log.error("fail to encode param");
        }
        method.setRequestEntity(se);
        try {
            httpClient.executeMethod(method);
        } catch (IOException e) {
            log.error("执行HTTP Post请求" + url + "时，发生异常！", e);
        } finally {
            method.releaseConnection();
        }
        CookieSpec cookiespec = CookiePolicy.getDefaultSpec();
        Cookie[] cookies = cookiespec.match("localhost", 8083, "/", false,
                httpClient.getState().getCookies());
        if (cookies.length == 0) {
            System.out.println("None");
        } else {
            for (int i = 0; i < cookies.length; i++) {
                System.out.println(cookies[i].toString());
            }
        }
        return cookies;
    }

    /**
     * 设置一下返回错误的通用提示,可以自定义格式.
     *
     * @param reason
     * @return
     */
    public static String returnError(String reason) {
        StringBuilder buffer = new StringBuilder();
        buffer.append("<?xml version=\"1.0\" encoding=\"GBK\"?>");
        buffer.append("<Response>");
        buffer.append("<Success>false</Success>");
        buffer.append("<reason>");
        buffer.append(reason);
        buffer.append("</reason>");
        buffer.append("</Response>");
        return buffer.toString();
    }

    public final static String REQUEST_HEADER = "x-forwarded-for";

    /**
     * 将客户IP写入请求头 这个设置可以伪装IP请求,注意使用
     *
     * @param client
     * @param ip
     * @return
     */
    public static void resetRequestHeader(HttpClient client) {
        List<Header> headers = new ArrayList<Header>();
        headers.add(new Header("User-Agent", getUA()));
        headers.add(new Header("Content-Type", "application/json"));
        client.getHostConfiguration()
                .getParams()
                .setParameter(HttpMethodParams.HTTP_CONTENT_CHARSET,
                        Constant.encoding);
    }

    /**
     * 随机获取user_agent
     *
     * @return
     */
    public static String getUA() {
        String[] _user_agent_pc = {
                "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)",
                "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)",
                "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 6.0)",
                "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0)",
                "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0)",
                "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.1)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.0)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1)",

                "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Maxthon 2.0)",
                "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; TencentTraveler 4.0)",
                "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; The World)",
                "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; 360SE)",
                "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Avant Browser)",
                "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0)",

                "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; Maxthon 2.0)",
                "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; TencentTraveler 4.0)",
                "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; The World)",
                "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; 360SE)",
                "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; Avant Browser)",
                "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; Trident/4.0)",

                "Mozilla/4.0 (compatible; MSIE 9.0; Windows NT 6.1; Maxthon 2.0)",
                "Mozilla/4.0 (compatible; MSIE 9.0; Windows NT 6.1; TencentTraveler 4.0)",
                "Mozilla/4.0 (compatible; MSIE 9.0; Windows NT 6.1; The World)",
                "Mozilla/4.0 (compatible; MSIE 9.0; Windows NT 6.1; 360SE)",
                "Mozilla/4.0 (compatible; MSIE 9.0; Windows NT 6.1; Avant Browser)",
                "Mozilla/4.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/4.0)",

                "Mozilla/5.0 (compatible; MSIE 7.0; Windows NT 5.1; Maxthon 2.0)",
                "Mozilla/5.0 (compatible; MSIE 7.0; Windows NT 5.1; TencentTraveler 4.0)",
                "Mozilla/5.0 (compatible; MSIE 7.0; Windows NT 5.1; The World)",
                "Mozilla/5.0 (compatible; MSIE 7.0; Windows NT 5.1; 360SE)",
                "Mozilla/5.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0)",

                "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0; Maxthon 2.0)",
                "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0; TencentTraveler 4.0)",
                "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0; The World)",
                "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0; 360SE)",
                "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0; Avant Browser)",
                "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0; Trident/4.0)",

                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Maxthon 2.0)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; TencentTraveler 4.0)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; The World)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; 360SE)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Avant Browser)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/4.0)",

                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.6; rv:2.0.1) Gecko/20100101 Firefox/4.0.1",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.10; rv:39.0) Gecko/20100101 Firefox/39.0",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_6) AppleWebKit/534.50 (KHTML, like Gecko) Version/5.1 Safari/534.50",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0 Safari/535.11",

                "Mozilla/5.0 (Windows NT 6.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1",
                "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-us) AppleWebKit/534.50 (KHTML, like Gecko) Version/5.1 Safari/534.50",
                "Mozilla/5.0 (Windows NT 6.0; rv:2.0) Gecko/20100101 Firefox/4.0 Opera 12.14",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.0) Opera 12.14",
                "Mozilla/5.0 (Windows NT 5.1) Gecko/20100101 Firefox/14.0 Opera/12.0",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; de) Opera 11.51",
                "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0 Safari/537.36",
                "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2227.0 Safari/537.36",
                "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2227.0 Safari/537.36",
                "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2226.0 Safari/537.36",
                "Mozilla/5.0 (Windows NT 6.4; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2225.0 Safari/537.36",
                "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2225.0 Safari/537.36",
                "Mozilla/5.0 (MSIE 9.0; Windows NT 6.1; Trident/7.0; rv:11.0) like Gecko QQBrowser/8.1.3886.400",
                "Mozilla/5.0 (MSIE 9.0; Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko QQBrowser/8.2.3638.400",
                "Mozilla/5.0 (MSIE 9.0; Windows NT 6.0; Trident/7.0; rv:11.0) like Gecko QQBrowser/8.3.4765.400",
                "Mozilla/5.0 (MSIE 9.0; Windows NT 6.1; Trident/7.0; rv:11.0) like Gecko QQBrowser/9.1.3471.400",
                "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0; 2345Explorer 3.4.0.12519)",
                "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0; 2345Explorer 3.5.0.12758)",
                "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0; 2345Explorer 4.0.0.13120)",
                "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0; 2345Explorer 4.2.0.13550)",
                "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0; 2345Explorer 5.0.0.14004)",
                "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0; 2345Explorer/6.1.0.8158)",
                "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0; 2345Explorer/6.2.0.9202)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0; UBrowser/5.0.1369.26)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0; UBrowser/5.2.2603.1)",
                "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0; UBrowser/5.4.4237.43)",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:34.0) Gecko/20100101 Firefox/34.0",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:36.0) Gecko/20100101 Firefox/36.0",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:37.0) Gecko/20100101 Firefox/37.0",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:38.0) Gecko/20100101 Firefox/38.0",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:39.0) Gecko/20100101 Firefox/39.0",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:40.0) Gecko/20100101 Firefox/40.0",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:41.0) Gecko/20100101 Firefox/41.0",

                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/2.1.7.6 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/34.0.1847.11 YYE/3.6 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/38.0.2125.111 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.115 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.101 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.152 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.124 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.107 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.46 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2478.0 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2498.0 Safari/537.36",

                "Opera/9.80 (Macintosh; Intel Mac OS X 10.6.8; U; en) Presto/2.8.131 Version/11.11",
                "Opera/9.80 (Windows NT 6.1; U; en) Presto/2.8.131 Version/11.11",
                "Opera/9.80 (X11; Linux i686; Ubuntu/14.10) Presto/2.12.388 Version/12.16",
                "Opera/9.80 (Windows NT 6.0) Presto/2.12.388 Version/12.14",
                "Opera/9.80 (Windows NT 6.1; WOW64; U; pt) Presto/2.10.229 Version/11.62",
                "Opera/9.80 (Windows NT 6.0; U; pl) Presto/2.10.229 Version/11.62",
                "Opera/9.80 (Macintosh; Intel Mac OS X 10.6.8; U; fr) Presto/2.9.168 Version/11.52",
                "Opera/9.80 (Macintosh; Intel Mac OS X 10.6.8; U; de) Presto/2.9.168 Version/11.52",
                "Opera/9.80 (Windows NT 5.1; U; en) Presto/2.9.168 Version/11.51",
                "Opera/9.80 (Windows NT 6.1; U; es-ES) Presto/2.9.181 Version/12.00",
                "Opera/9.80 (Windows NT 5.1; U; zh-sg) Presto/2.9.181 Version/12.00",
                "Opera/12.0 (Windows NT 5.2;U;en)Presto/22.9.168 Version/12.00",
                "Opera/12.0 (Windows NT 5.1;U;en)Presto/22.9.168 Version/12.00",
                "Opera/12.80 (Windows NT 5.1; U; en) Presto/2.10.289 Version/12.02"};

        int index = (int) (Math.random() * _user_agent_pc.length);
        return _user_agent_pc[index];
    }

    /**
     * Http post请求，获取结果
     * @param url
     * @param params
     * @param headerParams
     * @return
     */
    public static String doHttpPostRequestMap(String url, Map<String, String> params, Map<String, String> headerParams) {
        HttpClient httpClient = new HttpClient();
        resetRequestHeader(httpClient);
        PostMethod method = new PostMethod(url);
        List<NameValuePair> param = new ArrayList<>();
        if (params != null) {
            for (Map.Entry<String, String> entry : params.entrySet()) {
                param.add(new NameValuePair(entry.getKey(), entry.getValue()));
            }

            method.setRequestBody(param.toArray(new NameValuePair[param.size()]));
        }
        if (headerParams != null) {
            Iterator<Map.Entry<String, String>> iterator = headerParams.entrySet().iterator();
            while (iterator.hasNext()) {
                Map.Entry<String, String> entry = iterator.next();
                method.addRequestHeader(entry.getKey(), entry.getValue());
            }
        }
        String response = null;
        try {
            httpClient.executeMethod(method);
            response = method.getResponseBodyAsString();
        } catch (IOException e) {
            log.error("执行HTTP Post请求" + url + "时，发生异常！", e);
        } finally {
            method.releaseConnection();
        }
        return response;
    }

    /**
     * Http get请求，获取结果
     * @param url
     * @param headerParams
     * @return
     */
    public static String doHttpGetRequest(String url, Map<String, String> headerParams) {
        HttpClient httpClient = new HttpClient(httpConnectionManager);
        resetRequestHeader(httpClient);
        HttpMethod method = new GetMethod(url);
        if (headerParams != null) {
            Iterator<Map.Entry<String, String>> iterator = headerParams.entrySet().iterator();
            while (iterator.hasNext()) {
                Map.Entry<String, String> entry = iterator.next();
                method.addRequestHeader(entry.getKey(), entry.getValue());
            }
        }
        String response = null;
        try {
            httpClient.executeMethod(method);
            if (method.getStatusCode() == HttpStatus.SC_OK) {
                response = method.getResponseBodyAsString();
            }
        } catch (IOException e) {
            log.error("执行HTTP Get请求" + url + "时，发生异常！", e);
        } finally {
            method.releaseConnection();
        }
        return response;
    }

    public static void main(String[] args) {
        String response = HttpClientUtil
                .doHttpGetRequest("http://10.206.63.19:9999/api/app/getAppByPackageName.do?time=1471401299&sign=dd912c078ec857ddcfc7890937deac42&packageName=com.lemon.android.govermentbook6");
        System.out.println(response);
    }

}
```