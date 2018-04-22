```java
import com.beust.jcommander.DynamicParameter;
import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParameterException;
import com.beust.jcommander.internal.Lists;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 命令行参数校验工具
 * 参考文档 {@code http://jcommander.org/#_individual_parameter_validation}
 *
 * @author pengxingxiong(0016004591) 2018/2/22
 */
public class JCommanderExample {
    @Parameter
    private List<String> parameters = Lists.newArrayList();
    //用validateWith进行值校验
    @Parameter(names = {"--length", "-l"}, required = true, description = "param length is necessary and should be positive", validateWith = PositiveInteger.class)
    private int length;
    @Parameter(names = {"--pattern", "-p"}, required = true, description = "param pattern is necessary")
    private int pattern;
    @DynamicParameter(names = "-D", description = "Dynamic parameters go here")
    private Map<String, String> dynamicParams = new HashMap<>();
    @Parameter(names = "-debug", description = "Debug mode")
    private boolean debug = false;
    @Parameter(names = "-password", description = "Connection password", password = true)
    private String password;

    public static void main(String... args) {
        String[] argv = { "-l", "512", "-p", "2", "-debug", "-Doption=value", "a", "b", "c","-password","123"};
        JCommanderExample example = new JCommanderExample();
        example.run(argv);
    }

    private void run(String... args) {
        JCommander jcmd = new JCommander(this);
        try {
            jcmd.parse(args);
        } catch (ParameterException e) {
            System.out.println(e.getMessage());
            jcmd.usage();//显示参数的基本信息
            System.exit(1);
        }
        getResult();
    }

    private void getResult() {
        System.out.printf("%d %d\n", length, pattern);
        System.out.println("parameters=" + parameters);
        System.out.println("dynamicParams=" + dynamicParams);
        System.out.println("debug=" + debug);
        System.out.println("password=" + password);
    }
}
```
```java
import com.beust.jcommander.IParameterValidator;
import com.beust.jcommander.ParameterException;

/**
 * @author pengxingxiong(0016004591) 2018/2/22
 */
public class PositiveInteger implements IParameterValidator {
    public void validate(String name, String value)
            throws ParameterException {
        int n = Integer.parseInt(value);
        if (n < 0) {
            throw new ParameterException("Parameter " + name + " should be positive (found " + value + ")");
        }
    }
}
```
输出结果为：
```bash
"C:\Program Files\Java\jdk1.8.0_45\bin\java" "....example.jCommander.JCommanderExample -l 512 --pattern 2 a b c -Doption=value -debug true
512 2
parameters=[a, b, c]
dynamicParams={option=value}
debug=true
password=123

Process finished with exit code 0
```