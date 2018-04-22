# 1 配置文件
```
public static final ResourceBundle mailBundle = ResourceBundle.getBundle("mail");
public static final String mailHost = mailBundle.getString("mail.host");
public static final String mailFrom = mailBundle.getString("mail.from");
public static final String mailFromNAME = "nubia数据分析平台";
public static final String mailUsername = mailBundle.getString("mail.username");
public static final String mailPassword = mailBundle.getString("mail.password");
public static final String mailContentType = mailBundle
            .getString("mail.content_type");
public static final String mailList = mailBundle.getString("mail.list");
protected static final List<String> mlist = Arrays.asList(mailList.split(","));
```
这些配置主要在Constant类中配置，然后读取配置文件中的信息。

下面是工具类
```java
import java.sql.SQLException;
import java.util.Arrays;

import net.sourceforge.pinyin4j.PinyinHelper;
import net.sourceforge.pinyin4j.format.HanyuPinyinCaseType;
import net.sourceforge.pinyin4j.format.HanyuPinyinOutputFormat;
import net.sourceforge.pinyin4j.format.HanyuPinyinToneType;
import net.sourceforge.pinyin4j.format.HanyuPinyinVCharType;
import net.sourceforge.pinyin4j.format.exception.BadHanyuPinyinOutputFormatCombination;

/**
 * 拼音工具类
 * 
 */
public class PinYinUtil {
	/**
	 * 将字符串中的中文转化为拼音,其他字符不变
	 * 
	 * @param inputString
	 * @return
	 */
	public static String getPingYin(String inputString) {
		HanyuPinyinOutputFormat format = new HanyuPinyinOutputFormat();
		format.setCaseType(HanyuPinyinCaseType.LOWERCASE);
		format.setToneType(HanyuPinyinToneType.WITHOUT_TONE);
		format.setVCharType(HanyuPinyinVCharType.WITH_V);

		char[] input = inputString.trim().toCharArray();
		String output = "";

		try {
			for (int i = 0; i < input.length; i++) {
				if (java.lang.Character.toString(input[i]).matches(
						"[\\u4E00-\\u9FA5]+")) {
					String[] temp = PinyinHelper.toHanyuPinyinStringArray(
							input[i], format);
					output += temp[0];
				} else
					output += java.lang.Character.toString(input[i]);
			}
		} catch (BadHanyuPinyinOutputFormatCombination e) {
			e.printStackTrace();
		}
		return output;
	}

	/**
	 * 获取汉字串拼音首字母，英文字符不变
	 * 
	 * @param chinese
	 *            汉字串
	 * @return 汉语拼音首字母
	 */
	public static String getFirstSpell(String chinese) {
		StringBuilder pybf = new StringBuilder();
		char[] arr = chinese.toCharArray();
		HanyuPinyinOutputFormat defaultFormat = new HanyuPinyinOutputFormat();
		defaultFormat.setCaseType(HanyuPinyinCaseType.LOWERCASE);
		defaultFormat.setToneType(HanyuPinyinToneType.WITHOUT_TONE);
		for (int i = 0; i < arr.length; i++) {
			if (arr[i] > 128) {
				try {
					String[] temp = PinyinHelper.toHanyuPinyinStringArray(
							arr[i], defaultFormat);
					if (temp != null) {
						pybf.append(temp[0].charAt(0));
					}
				} catch (BadHanyuPinyinOutputFormatCombination e) {
					e.printStackTrace();
				}
			} else {
				pybf.append(arr[i]);
			}
		}
		return pybf.toString().replaceAll("\\W", "").trim();
	}

	/**
	 * 获取汉字串拼音，英文字符不变
	 * 
	 * @param chinese
	 *            汉字串
	 * @return 汉语拼音
	 */
	public static String getFullSpell(String chinese) {
		StringBuilder pybf = new StringBuilder();
		char[] arr = chinese.toCharArray();
		HanyuPinyinOutputFormat defaultFormat = new HanyuPinyinOutputFormat();
		defaultFormat.setCaseType(HanyuPinyinCaseType.LOWERCASE);
		defaultFormat.setToneType(HanyuPinyinToneType.WITHOUT_TONE);
		for (int i = 0; i < arr.length; i++) {
			if (arr[i] > 128) {
				try {
					pybf.append(PinyinHelper.toHanyuPinyinStringArray(arr[i],
							defaultFormat)[0]);
				} catch (BadHanyuPinyinOutputFormatCombination e) {
					e.printStackTrace();
				}
			} else {
				pybf.append(arr[i]);
			}
		}
		return pybf.toString();
	}

	/**
	 * 获取汉字串拼音首字母，英文字符不变
	 * 
	 * @param chinese
	 *            汉字串
	 * @return 汉语拼音首字母
	 */
	public static String getFirstCharSpell(String chinese) {
		StringBuilder pybf = new StringBuilder();
		char[] arr = chinese.toCharArray();
		if (arr.length > 0) {
			HanyuPinyinOutputFormat defaultFormat = new HanyuPinyinOutputFormat();
			defaultFormat.setCaseType(HanyuPinyinCaseType.LOWERCASE);
			defaultFormat.setToneType(HanyuPinyinToneType.WITHOUT_TONE);
			if (arr[0] > 128) {
				try {
					String[] temp = PinyinHelper.toHanyuPinyinStringArray(
							arr[0], defaultFormat);
					if (temp != null) {
						pybf.append(temp[0].charAt(0));
					}
				} catch (BadHanyuPinyinOutputFormatCombination e) {
					e.printStackTrace();
				}
			} else {
				pybf.append(arr[0]);
			}
			return pybf.toString().replaceAll("\\W", "").trim();
		} else {
			return null;
		}
	}

	public static void main(String[] args) throws SQLException {

		// DBUtil db = DBUtil.newInstance();
		// String sql = "select * from channel";
		// ResultSet rs = db.query(sql);
		// while (rs.next()) {
		// Long id = rs.getLong("id");
		// String name = rs.getString("name");
		// String firstchar = PinYinUtil.getFirstCharSpell(name);
		// String updatesql =
		// "update channel set firstchar='"+firstchar+"' where id=" + id;
		// // System.out.println(name+"----"+updatesql);
		// db.insert(updatesql);
		// }
		// String str =
		// "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADwAAAA8CAYAAAA6/NlyAAADU0lEQVRoQ+1b0Y6DMAyD//9oTqCBsuLYTkHTju6edlDaOmkcJ2PzNE3LVPhbFjx8nudjln1MvLbejNezMWgr7djKs+186y6XDEQcvG++HRuvZwBPiwbjtPeQUTJ/ZHtC4/exYwNuPbRbarV6a03mzWweZz527CunK661foYe/gF+manqYSfm1CloSS7+v8/P+OOjHn4kYId5s3TisH+MO8bku+dZON4Sw48AzPSHysMsxrJ4QnHK9sBYmj138rAaHO8z1aMUkZNyEBGpeSv734RH5YF/D9jSlYZFVs84Ohsd/SqJOVI4zRQ/wEGGISnICKi9hwTCTiCqokIecooFVcDMyMPoIVbqZXnyawErq7gx5igsgw62IUhIOGyt4nvz8LCAe8qv3jhXXBALluxU9HRQYAy3sYcUETLOlbTE6m2lyZ3K68DE0hLrUz0KsGNtljZUeqqeGEfHZ2u2+6RpCeVKxoKK/HpPzK2AmZZG4BSomFLcmEYeZ6dA9dfos8MBjnk4S0sxDpjXVNLviXtHYrLCBcbwHTFSWRQpN5d0UMhU1rbr4YrYVyLfyZvxtPTEbCZsxgNcjeHMcpXeNUtPSKqqoqG0NvoyTR3JjEhcCegcVwYCreOkyy3+hwTsSEHFos4xZZUVOjUZUWbVkzPHG2ldzcOq5PsKwKzF4+hmVJO25SWTqE66U+2lUu08HGD2ygPrKzlx5LA9Y1clUFjHI5XAQwJWDMy0r0NETl3t5tGopR1WPhGpm4czPesQUktiTq2LKrTKHk5V0uvNIVt4VBZzqi8lFz8C2CGZzHLMa+gZJUMdUkShpgz55uFhALfWrIhzZqTe7mcmXFSassIA9bQeDbj3+2EnjVwlusyjTmZI5ehwgO96x4OJEyY8VGHgiAunWXDwwpCAnX6yk7KcMYqBWXwyCaz4Yr8P8zBKU6pVimrSK+JiXa+n45E1MY65opZmrPgDDF7jV9ZlUpAVGEpmxtOA5nmTvXd5OE6axem/BpxZ0gHuxqdDpooET4VLr4cfAZjFiko5KI6Y9VVLuNIxRfvOCPjyz3giqLRx9iI4p6XLcq3DDezkrffsbw/VRqJFe3rNaH5VDkYDK+FxWVqeyKDwU7zN0smv0xCILNR6AP8BtmgX+/MHzlwAAAAASUVORK5CYII=";
		// System.out.println(str.substring(str.indexOf("base64,")+7));

		String mail = "zhangxiaoyan@reyun.com,liruijie@reyun.com";
		System.out.println(Arrays.asList(mail));
	}

}
```