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
package com.reyun.util;

import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import javax.mail.BodyPart;
import javax.mail.MessagingException;
import javax.mail.Multipart;
import javax.mail.internet.MimeBodyPart;
import javax.mail.internet.MimeMultipart;
import javax.mail.internet.MimeUtility;

import com.reyun.util.excel.ExcelBean;
import com.reyun.util.subscription.appstore.BizReport;
import org.apache.commons.mail.EmailAttachment;
import org.apache.commons.mail.EmailException;
import org.apache.commons.mail.HtmlEmail;
import org.apache.commons.mail.MultiPartEmail;
import org.apache.commons.mail.SimpleEmail;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.reyun.model.Email;

public class MailUtils {
	public static final String CONTENT_TYPE = Constant.mailContentType;
	public static final String FROM_EMAIL_ADDRESS = Constant.mailFrom;
	public static final String FROM_EMAIL_NAME = Constant.mailFromNAME;

	protected static Logger logger = LoggerFactory.getLogger(MailUtils.class);
	/**
	 * 简单的发邮件方式 邮件内容只有标题和邮件内容 支持多个用户批量发送
	 * 
	 * @param subject
	 *            邮件标题
	 * @param contents
	 *            邮件内容
	 * @param userEmailAddress
	 *            收入人的邮件地址 为List形式
	 * @throws EmailException 
	 * @throws Exception
	 */
	public static void sendSimpleEmail(String subject, String contents,
			List<String> userEmailAddress) throws EmailException  {

		SimpleEmail email = new SimpleEmail();
		email.setHostName(Constant.mailHost);
		email.setAuthentication(Constant.mailUsername, Constant.mailPassword);
		// 发送给多个人
		for (int i = 0; i < userEmailAddress.size(); i++) {
			email.addTo(userEmailAddress.get(i), userEmailAddress.get(i));
		}
		email.setFrom(FROM_EMAIL_ADDRESS, Constant.mailUsername);
		email.setSubject(subject);
		email.setContent(contents, CONTENT_TYPE);
		email.send();
	}

	/**
	 * 发送带附件的邮件方式 邮件内容有标题和邮件内容和附件，附件可以是本地机器上的文本，也可以是web上的一个URL 文件，
	 * 当为web上的一个URL文件时，此方法可以将WEB中的URL文件先下载到本地，再发送给收入用户
	 * 
	 * @param subject
	 *            邮件标题
	 * @param contents
	 *            邮件内容
	 * @param userEmailAddress
	 *            收入人的邮件地址 为List形式
	 * @param multiPaths
	 *            附件地址 为数组形式
	 * @throws MalformedURLException 
	 * @throws EmailException 
	 * @throws Exception
	 * @throws Exception
	 */

	public static void sendMultiPartEmail(String subject, String contents,
			List<String> userEmailAddress, String[] multiPaths,List<String> fileList) throws MalformedURLException, EmailException
			 {
		List list = new ArrayList();
		for (int j = 0; j < multiPaths.length; j++) {
			EmailAttachment attachment = new EmailAttachment();
			if (multiPaths[j].indexOf("http") == -1) // 判断当前这个文件路径是否在本地

			{
				attachment.setPath(multiPaths[j]);
			} else {
				attachment.setURL(new URL(multiPaths[j]));
			}

			try {
				attachment.setName(
						// 解决附件名乱码
						MimeUtility.encodeText(fileList.get(j)));
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
			}
			attachment.setDisposition(EmailAttachment.ATTACHMENT);
			attachment.setDescription("");
			list.add(attachment);
		}
		// 发送邮件信息
		MultiPartEmail email = new MultiPartEmail();
		email.setHostName(Constant.mailHost);
		email.setAuthentication(Constant.mailUsername, Constant.mailPassword);
		// 发送给多个人
		for (int i = 0; i < userEmailAddress.size(); i++) {
			email.addTo(userEmailAddress.get(i), userEmailAddress.get(i));
		}
		email.setFrom(FROM_EMAIL_ADDRESS, FROM_EMAIL_NAME,"utf-8");
		email.setSubject(subject);
		email.setMsg(contents); // 注意这个不要使用setContent这个方法 setMsg不会出现乱码
		for (int a = 0; a < list.size(); a++) // 添加多个附件
		{
			email.attach((EmailAttachment) list.get(a));
		}
		// email.attach(attachment);
		email.send();
	}

	/**
	 * 此方法只发送一个邮箱，只能添加一个附件 发送带附件的邮件方式
	 * 邮件内容有标题和邮件内容和附件，附件可以是本地机器上的文本，也可以是web上的一个URL 文件，
	 * 当为web上的一个URL文件时，此方法可以将WEB中的URL文件先下载到本地，再发送给收入用户
	 * 
	 * @param subject
	 *            邮件标题
	 * @param contents
	 *            邮件内容
	 * @param userEmailAddress
	 *            收入人的邮件地址
	 * @param multiPath
	 *            附件地址
	 * @throws MalformedURLException 
	 * @throws UnsupportedEncodingException 
	 * @throws EmailException 
	 * @throws Exception
	 * @throws Exception
	 */

	public static void sendPartEmail(String subject, String contents,
			String userEmailAddress, String multiPath) throws MalformedURLException, UnsupportedEncodingException, EmailException  {
		EmailAttachment attachment = new EmailAttachment();
		if (multiPath.indexOf("http") == -1) // 判断当前这个文件路径是否在本地 // setURL;
		{
			attachment.setPath(multiPath);
		} else {
			attachment.setURL(new URL(multiPath));
		}
		attachment.setDisposition(EmailAttachment.ATTACHMENT);
		attachment.setDescription("");
		attachment.setName(MimeUtility.encodeText(multiPath.substring(multiPath.indexOf("export"),
				multiPath.length())));
		// 发送邮件信息
		MultiPartEmail email = new MultiPartEmail();
		email.setHostName(Constant.mailHost);
		email.setAuthentication(Constant.mailUsername, Constant.mailPassword);
		// 发送给一个人
		email.addTo(userEmailAddress, userEmailAddress);
		email.setFrom(FROM_EMAIL_ADDRESS, FROM_EMAIL_NAME,"utf-8");
		email.setSubject(subject);
		email.setMsg(contents); // 注意这个不要使用setContent这个方法 setMsg不会出现乱码
		// 添加多个附件
		email.attach(attachment);
		// email.attach(attachment);
		email.send();
	}

	/**
	 * 发送Html格式的邮件
	 * 
	 * @param subject
	 *            邮件标题
	 * @param contents
	 *            邮件内容
	 * @param userEmailAddress
	 *            接收用户的邮箱地址
	 * @throws EmailException 
	 * 
	 * @throws Exception
	 */
	public static void sendHtmlEmail(String subject, String contents,
			List<String> userEmailAddress) throws EmailException  {

		HtmlEmail email = new HtmlEmail();
		email.setHostName(Constant.mailHost);
		email.setAuthentication(Constant.mailUsername, Constant.mailPassword);
		// 发送给多个人
		for (int i = 0; i < userEmailAddress.size(); i++) {
			email.addTo(userEmailAddress.get(i), userEmailAddress.get(i));
		}
		email.setFrom(FROM_EMAIL_ADDRESS, FROM_EMAIL_NAME,"utf-8");
		email.setSubject(subject);
		email.setHtmlMsg(contents);
		email.setTextMsg(contents);
		email.setCharset("utf-8");
		email.send();
	}


	/**
	 * 发送Html格式的邮件 (可加附件)
	 *
	 * @param subject
	 *            邮件标题
	 * @param contents
	 *            邮件内容
	 * @param userEmailAddress
	 *            接收用户的邮箱地址
	 *            发送人的邮箱地址
	 * @throws EmailException
	 *
	 * @throws Exception
	 */
	public static void sendHtmlEmail(String subject, String contents,
									 List<String> userEmailAddress,String[] multiPaths,List<String> fileNameList) throws EmailException, MalformedURLException {

		HtmlEmail email = new HtmlEmail();
		email.setHostName(Constant.mailHost);
		email.setAuthentication(Constant.mailUsername, Constant.mailPassword);
		// 发送给多个人
		for (int i = 0; i < userEmailAddress.size(); i++) {
			email.addTo(userEmailAddress.get(i), userEmailAddress.get(i));
		}
		email.setFrom(FROM_EMAIL_ADDRESS, FROM_EMAIL_NAME,"utf-8");
		email.setSubject(subject);
		email.setHtmlMsg(contents);
		email.setTextMsg(contents);

		List list = new ArrayList();
		// EmailAttachment [] attachmentArray = new
		// EmailAttachment[multiPaths.length];
		for (int j = 0; j < multiPaths.length; j++) {
			EmailAttachment attachment = new EmailAttachment();
			if (multiPaths[j].indexOf("http") == -1) // 判断当前这个文件路径是否在本地
			// 如果是：setPath 否则
			// setURL;
			{
				attachment.setPath(multiPaths[j]);
			} else {
				attachment.setURL(new URL(multiPaths[j]));
			}
			try {
				attachment.setName(
						// 解决附件名乱码
						MimeUtility.encodeText(fileNameList.get(j)));
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
			}
			attachment.setDisposition(EmailAttachment.ATTACHMENT);
			attachment.setDescription("");
			list.add(attachment);
		}
		for (int a = 0; a < list.size(); a++) // 添加多个附件
		{
			email.attach((EmailAttachment) list.get(a));
		}
		email.setCharset("utf-8");
		email.send();
	}

	/**
	 * 统一的发送邮件的方法 调用时一定要实例化EmailBean对象
	 * @throws EmailException 
	 * @throws MalformedURLException 
	 * 
	 * @throws Exception
	 * 
	 */
//	public static void sendEmail(Email email) throws MalformedURLException, EmailException  {
//		if (email.isHaveMultPaths()) {
//			sendMultiPartEmail(email.getSubject(), email.getContents(),
//					email.getUserEmailAddress(), email.getMultiPaths());
//		} else {
//			sendSimpleEmail(email.getSubject(), email.getContents(),
//					email.getUserEmailAddress());
//		}
//	}

	public static void main(String[] args) {
		List<String> mailList = new ArrayList<>();
		mailList.add("carrie_yz@163.com");
		try {
			MailUtils.sendSimpleEmail("test", "test", mailList);
		} catch (EmailException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}

```