# 对字符串进行MD5加密工具
```java
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import org.apache.commons.lang.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ShortUrl {
	protected static Logger logger = LoggerFactory.getLogger(ShortUrl.class);
	
	public static String[] generateCode(String url) {
		String key = "Binjoo"; // 网址的混合KEY
		String[] chars = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
				"k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v",
				"w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7",
				"8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
				"K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
				"W", "X", "Y", "Z" };
		String hex = toMd5(key + url);// 对传入网址和混合KEY进行MD5加密
		String[] resUrl = new String[4];

		for (int i = 0; i < 4; i++) {
			// 把加密字符按照8位一组16进制与0x3FFFFFFF进行位与运算
			String sTempSubString = hex.substring(i * 8, i * 8 + 8);
			long lHexLong = 0x3FFFFFFF & Long.parseLong(sTempSubString, 16);
			String outChars = "";
			for (int j = 0; j < 6; j++) {
				// 把得到的值与 0x0000003D 进行位与运算，取得字符数组 chars 索引
				long index = 0x0000003D & lHexLong;
				// 把取得的字符相加
				outChars += chars[(int) index];
				// 每次循环按位右移 5 位
				lHexLong = lHexLong >> 5;
			}
			resUrl[i] = outChars;
		}

		return resUrl;
	}

	public static String[] generateCodeN(String url, int length) {
		String key = "REYUN!@#"; // 网址的混合KEY
		String[] chars = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
				"k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v",
				"w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7",
				"8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
				"K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
				"W", "X", "Y", "Z" };
		String hex = toMd5(key + url);// 对传入网址和混合KEY进行MD5加密
		System.out.println(hex);
		hex = hex + CharacterUtils.getRandomString(length * 8);

		String[] resUrl = new String[length];

		for (int i = 0; i < length; i++) {
			// 把加密字符按照8位一组16进制与0x3FFFFFFF进行位与运算
			String sTempSubString = hex.substring(i * 8, i * 8 + 8);
			long lHexLong = 0x3FFFFFFF & Long.parseLong(sTempSubString, 16);
			String outChars = "";
			for (int j = 0; j < 6; j++) {
				// 把得到的值与 0x0000003D 进行位与运算，取得字符数组 chars 索引
				long index = 0x0000003D & lHexLong;
				// 把取得的字符相加
				outChars += chars[(int) index];
				// 每次循环按位右移 5 位
				lHexLong = lHexLong >> 5;
			}
			resUrl[i] = outChars;
		}

		return resUrl;
	}

	private static String toMd5(String arg) {
		try {
			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(arg.getBytes());
			byte b[] = md.digest();
			int i;
			StringBuilder buf = new StringBuilder("");
			for (int offset = 0; offset < b.length; offset++) {
				i = b[offset];
				if (i < 0)
					i += 256;
				if (i < 16)
					buf.append("0");
				buf.append(Integer.toHexString(i));
			}
			return buf.toString();
		} catch (NoSuchAlgorithmException e) {
			logger.error(e.getMessage());
		}
		return "";
	}

	public static void main(String[] args) {
		String[] result = generateCodeN(
				"zhangxiaoyan@reyun.comAASJJHDMFHBbjdBDFHJDNFHDFF", 200);
		System.out.println(result.length);
		System.out.println(StringUtils.join(result, ","));
	}

}
```