# 验证码生成器
```java
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsEnvironment;
import java.awt.RenderingHints;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.Random;

import javax.imageio.ImageIO;

/**
 * 验证码生成器
 */
public class ValidateCodeUtil {

	// 图片的宽度。
	private int width = 160;
	// 图片的高度。
	private int height = 40;
	// 验证码字符个数
	private int codeCount = 5;
	// 验证码干扰线数
	private int lineCount = 150;
	// 验证码
	private static String code = null;
	// 验证码图片Buffer
	private BufferedImage buffImg = null;

	//去掉1、0、i、o容易混淆的四个
	private static final String VERIFY_CODES = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";  
	private static Random random = new Random();  
	 
	public ValidateCodeUtil() {
		this.createCode();
	}

	/**
	 * 
	 * @param width
	 *            图片宽
	 * @param height
	 *            图片高
	 */
	public ValidateCodeUtil(int width, int height) {
		this.width = width;
		this.height = height;
		this.createCode();
	}

	/**
	 * 
	 * @param width
	 *            图片宽
	 * @param height
	 *            图片高
	 * @param codeCount
	 *            字符个数
	 * @param lineCount
	 *            干扰线条数
	 */
	public ValidateCodeUtil(int width, 
			int height, 
			int codeCount,
			int lineCount) {
		this.width = width;
		this.height = height;
		this.codeCount = codeCount;
		this.lineCount = lineCount;
		setCode(this.generateVerifyCode(codeCount));
	}
	
	
	private String generateVerifyCode(int verifySize){
		return generateVerifyCode(verifySize,VERIFY_CODES);
	}
	
	private String generateVerifyCode(int verifySize,String sources){
		if(sources == null || sources.length() == 0){  
           sources = VERIFY_CODES;  
		}  
		int codesLen = sources.length();  
		Random rand = new Random(System.currentTimeMillis());  
		StringBuilder verifyCode = new StringBuilder(verifySize);  
        for(int i = 0; i < verifySize; i++){  
            verifyCode.append(sources.charAt(rand.nextInt(codesLen-1)));  
        }  
        return verifyCode.toString();   
	}

	public void createCode() {
		int x = 0, fontHeight = 0, codeY = 0;
		int red = 0, green = 0, blue = 0;

		x = width / (codeCount + 5);// 每个字符的宽度
		fontHeight = height - 2;// 字体的高度
		codeY = height - 4;
		// 图像buffer
		buffImg = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
		Graphics2D g = buffImg.createGraphics();
		// 将图像填充为白色
		g.setColor(Color.WHITE);
		g.fillRect(0, 0, width, height);
		Font font = new Font("宋体", Font.BOLD, fontHeight);
		g.setFont(font);
		g.drawRect(0, 0, width-1, height-1);
		g.setColor(getRandColor(160,200));
		for (int i = 0; i < lineCount; i++) {
			int xs = random.nextInt(width);
			int ys = random.nextInt(height);
			int xe = xs + random.nextInt(width / 8);
			int ye = ys + random.nextInt(height / 8);
			g.drawLine(xs, ys, xe, ye);
		}
		// randomCode记录随机产生的验证码
		StringBuilder randomCode = new StringBuilder();
		// 随机产生codeCount个字符的验证码。
		for (int i = 0; i < codeCount; i++) {
			String strRand = String.valueOf(code.charAt(i));
			// 产生随机的颜色值，让输出的每个字符的颜色值都将不同。
			red = random.nextInt(255);
			green = random.nextInt(255);
			blue = random.nextInt(255);
			g.setColor(new Color(red, green, blue));
			g.drawString(strRand, (i * 2) * x, codeY);
			// 将产生的四个随机数组合在一起。
			randomCode.append(strRand);
		}
		// 将四位数字的验证码保存到Session中。
		code = randomCode.toString();
	}

	public void write(String path) throws IOException {
		OutputStream sos = new FileOutputStream(path);
		this.write(sos);
	}

	public void write(OutputStream sos) throws IOException {
		ImageIO.write(buffImg, "png", sos);
		sos.close();
	}
	
	
	public void write(OutputStream out,String code) throws IOException{
		write(width, height, lineCount,out, code);
	}
	public void write(int w, int h, int lineCount,OutputStream out,String code) throws IOException{
		int verifySize = code.length();  
        BufferedImage image = new BufferedImage(w, h, BufferedImage.TYPE_INT_RGB);  
        Random rand = new Random();  
        Graphics2D g2 = image.createGraphics();  
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);  
        Color[] colors = new Color[5];  
        Color[] colorSpaces = new Color[] { Color.WHITE, Color.CYAN,  
                Color.GRAY, Color.LIGHT_GRAY, Color.MAGENTA, Color.ORANGE,  
                Color.PINK, Color.YELLOW };  
        float[] fractions = new float[colors.length];  
        for(int i = 0; i < colors.length; i++){  
            colors[i] = colorSpaces[rand.nextInt(colorSpaces.length)];  
            fractions[i] = rand.nextFloat();  
        }  
        Arrays.sort(fractions);  
          
        g2.setColor(Color.WHITE);// 设置边框色  
        g2.fillRect(0, 0, w, h);  
          
        Color c = getRandColor(200, 250);  
        g2.setColor(c);// 设置背景色  
        g2.fillRect(0, 2, w, h-4); 
		   //绘制干扰线  
        g2.setColor(getRandColor(160, 200));// 设置线条的颜色  
        for (int i = 0; i < lineCount; i++) {  
            int x = random.nextInt(w - 1);  
            int y = random.nextInt(h - 1);  
            int xl = random.nextInt(6) + 1;  
            int yl = random.nextInt(12) + 1;  
            g2.drawLine(x, y, x + xl + 40, y + yl + 20);  
        }  
          
        // 添加噪点  
        float yawpRate = 0.05f;// 噪声率  
        int area = (int) (yawpRate * w * h);  
        for (int i = 0; i < area; i++) {  
            int x = random.nextInt(w);  
            int y = random.nextInt(h);  
            int rgb = getRandomIntColor();  
            image.setRGB(x, y, rgb);  
        }  
		shear(g2, w, h, c);// 使图片扭曲  
		  
        g2.setColor(getRandColor(100, 160));  
        int fontSize = h-4;  
        Font font = new Font("Arial", Font.ITALIC, fontSize);  
        g2.setFont(font);  
        char[] chars = code.toCharArray();  
        for(int i = 0; i < verifySize; i++){  
            AffineTransform affine = new AffineTransform();  
            affine.setToRotation(Math.PI / 5 * rand.nextDouble() * (rand.nextBoolean() ? 1 : -1), (w / verifySize) * i + fontSize/2, h/2);  
            g2.setTransform(affine);  
            g2.drawChars(chars, i, 1, ((w-10) / verifySize) * i + 5, h/2 + fontSize/2 - 5);  
        }  
        g2.dispose();  
        ImageIO.write(image, "jpg", out);  
        out.close();
	}

	
	
	private static Color getRandColor(int fc, int bc) {  
        if (fc > 255)  
            fc = 255;  
        if (bc > 255)  
            bc = 255;  
        int r = fc + random.nextInt(bc - fc);  
        int g = fc + random.nextInt(bc - fc);  
        int b = fc + random.nextInt(bc - fc);  
        return new Color(r, g, b);  
    }  
	
	
	private static int getRandomIntColor() {  
        int[] rgb = getRandomRgb();  
        int color = 0;  
        for (int c : rgb) {  
            color = color << 8;  
            color = color | c;  
        }  
        return color;  
    }  
      
    private static int[] getRandomRgb() {  
        int[] rgb = new int[3];  
        for (int i = 0; i < 3; i++) {  
            rgb[i] = random.nextInt(255);  
        }  
        return rgb;  
    }  
  
    private static void shear(Graphics g, int w1, int h1, Color color) {  
        shearX(g, w1, h1, color);  
        shearY(g, w1, h1, color);  
    }  
	
	private static void shearX(Graphics g, int w1, int h1, Color color) {  
		  
        int period = random.nextInt(2);  
  
        boolean borderGap = true;  
        int frames = 1;  
        int phase = random.nextInt(2);  
  
        for (int i = 0; i < h1; i++) {  
            double d = (double) (period >> 1)  
                    * Math.sin((double) i / (double) period  
                            + (6.2831853071795862D * (double) phase)  
                            / (double) frames);  
            g.copyArea(0, i, w1, 1, (int) d, 0);  
            if (borderGap) {  
                g.setColor(color);  
                g.drawLine((int) d, i, 0, i);  
                g.drawLine((int) d + w1, i, w1, i);  
            }  
        }  
  
    } 
	 private static void shearY(Graphics g, int w1, int h1, Color color) {  
        int period = random.nextInt(40) + 10; // 50;  
        boolean borderGap = true;  
        int frames = 20;  
        int phase = 7;  
        for (int i = 0; i < w1; i++) {  
            double d = (double) (period >> 1)  
                    * Math.sin((double) i / (double) period  
                            + (6.2831853071795862D * (double) phase)  
                            / (double) frames);  
            g.copyArea(i, 0, 1, h1, 0, (int) d);  
            if (borderGap) {  
                g.setColor(color);  
                g.drawLine(i, (int) d, i, 0);  
                g.drawLine(i, (int) d + h1, i, h1);  
            }  
  
        }  
  
    } 
	 
	 private void getFontnames(){
		 	String[] fontnames = GraphicsEnvironment.getLocalGraphicsEnvironment()  
			.getAvailableFontFamilyNames();//获得当前系统字体  
	 }
	 
	  public static void main(String[] args) throws IOException {
	    	ValidateCodeUtil vCode = new ValidateCodeUtil(100, 34, 5, 10);
	    	OutputStream out = new FileOutputStream(new File("D:/tmp/code.jpg"));
	    	vCode.write(out,vCode.getCode());
	    	out.close();
		}
	 
	public BufferedImage getBuffImg() {
		return buffImg;
	}

	public static void setCode(String code) {
		ValidateCodeUtil.code = code;
	}

	public String getCode() {
		return code;
	}
	

}
```