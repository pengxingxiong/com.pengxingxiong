```java
package utils;

import org.apache.http.Consts;
import org.apache.http.HttpEntity;
import org.apache.http.NameValuePair;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.config.Registry;
import org.apache.http.config.RegistryBuilder;
import org.apache.http.conn.socket.ConnectionSocketFactory;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.impl.client.HttpClientBuilder;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.impl.conn.PoolingHttpClientConnectionManager;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.util.EntityUtils;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author pengxingxiong(0016004591) 2017/11/23
 */
public class HttpsClientHelper {
    private static HttpClientBuilder BUILDER = null;
    private static RequestConfig REQUEST_CONFIG = null;
    // 读取超时
    private final static int SOCKET_TIMEOUT = 20000;
    // 连接超时
    private final static int CONNECTION_TIMEOUT = 10000;
    // 连接池的最大连接数
    private final static int MAX_CONNECTION = 10;
    // 每个HOST的最大连接数量
    private final static int DEFAULT_MAX_CONNECTION = 5;

    static {
        SSLConnectionSocketFactory socketFactory = enableSSL();
        Registry<ConnectionSocketFactory> registry = RegistryBuilder.<ConnectionSocketFactory> create().register("https", socketFactory).build();
        REQUEST_CONFIG = RequestConfig.custom()
                .setSocketTimeout(SOCKET_TIMEOUT)
                .setConnectTimeout(CONNECTION_TIMEOUT)
                .setConnectionRequestTimeout(CONNECTION_TIMEOUT)
                .build();
        //连接池
        PoolingHttpClientConnectionManager connectionManager = new PoolingHttpClientConnectionManager(registry);
        connectionManager.setMaxTotal(MAX_CONNECTION);
        connectionManager.setDefaultMaxPerRoute(DEFAULT_MAX_CONNECTION);
        BUILDER = HttpClients.custom().setConnectionManager(connectionManager);
    }
    /**证书处理，这里是忽略证书，所以方法体都几乎为空*/
    private static SSLConnectionSocketFactory enableSSL() {
        try {
            SSLContext context = SSLContext.getInstance("TLS");
            TrustManager manager = new X509TrustManager() {

                @Override
                public X509Certificate[] getAcceptedIssuers() {
                    return null;
                }

                @Override
                public void checkServerTrusted(X509Certificate[] arg0, String arg1) throws CertificateException {

                }

                @Override
                public void checkClientTrusted(X509Certificate[] arg0, String arg1) throws CertificateException {

                }
            };
            context.init(null, new TrustManager[] { manager }, null);
            return new SSLConnectionSocketFactory(context, NoopHostnameVerifier.INSTANCE);
        } catch (KeyManagementException | NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return null;
    }
    /**
     * @author pengxingxiong
     */
    public static String getHttp(String url, Map<String, String> params, Map<String, String> header) throws IOException, URISyntaxException {
        HttpGet httpGet = new HttpGet(url);
        httpGet.setConfig(REQUEST_CONFIG);
        if (!header.isEmpty()) {
            for (Map.Entry<String, String> entry : header.entrySet()) {
                httpGet.setHeader(entry.getKey(), entry.getValue());
            }
        }
        // 设置参数
        List<NameValuePair> list = new ArrayList<>();
        for (Map.Entry<String, String> elem : params.entrySet()) {
            list.add(new BasicNameValuePair(elem.getKey(), elem.getValue()));
        }
        if (list.size() > 0) {
            UrlEncodedFormEntity entity = new UrlEncodedFormEntity(list);
            httpGet.setURI(new URI(httpGet.getURI().toString() + "?" + EntityUtils.toString(entity)));
        }
        CloseableHttpResponse response = BUILDER.build().execute(httpGet);
        return EntityUtils.toString(response.getEntity(), Consts.UTF_8);
    }
    public static String getHttp(String url, Map<String, String> params) throws IOException, URISyntaxException {
        Map<String, String> header = new HashMap<>();
        return getHttp(url, params, header);
    }
    public static String postHttp(String url, Map<String, String> params, Map<String, String> header) throws IOException {
        HttpPost httpPost = new HttpPost(url);
        httpPost.setConfig(REQUEST_CONFIG);
        if (!header.isEmpty()) {
            for (Map.Entry<String, String> entry : header.entrySet()) {
                httpPost.setHeader(entry.getKey(), entry.getValue());
            }
        }
        // 设置参数
        List<NameValuePair> list = new ArrayList<>();
        for (Map.Entry<String, String> elem : params.entrySet()) {
            list.add(new BasicNameValuePair(elem.getKey(), elem.getValue()));
        }
        if (list.size() > 0) {
            UrlEncodedFormEntity entity = new UrlEncodedFormEntity(list);
            httpPost.setEntity(entity);
        }
        CloseableHttpResponse response = BUILDER.build().execute(httpPost);

        return EntityUtils.toString(response.getEntity(), Consts.UTF_8);
    }
    public static String postHttp(String url, Map<String, String> params) throws IOException, URISyntaxException {
        Map<String, String> header = new HashMap<>();
        return postHttp(url, params, header);
    }
}

```