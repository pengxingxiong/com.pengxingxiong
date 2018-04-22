# excel工具
```java
import com.nubia.stat.report.constants.Constant;
import com.nubia.stat.report.constants.StateCode;
import com.nubia.stat.report.exception.Throws;
import com.nubia.stat.report.exception.UnCaughtException;
import com.nubia.stat.report.model.ExportFileParams;
import lombok.extern.slf4j.Slf4j;
import org.apache.poi.hssf.usermodel.HSSFRow;
import org.apache.poi.hssf.usermodel.HSSFSheet;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.apache.poi.ss.usermodel.*;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;

import javax.servlet.ServletOutputStream;
import java.io.*;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * @author pengxingxiong(0016004591) 2017/9/29
 * Excel解析工具类
 */
@Slf4j
public class ExcelUtils {
    private final static String excel2003L =".xls";    //2003- 版本的excel
    private final static String excel2007U =".xlsx";   //2007+ 版本的excel
    /**
     * 描述：获取IO流中的excel数据，组装成List<List<Object>>对象
     */
    public  List<List<Object>> excelReader(InputStream in, String fileName) throws IOException {
        List<List<Object>> list;
        //System.out.println(inputStream2String(in));
        //创建Excel工作薄
        Workbook work = this.getWorkbook(in,fileName);
        if(null == work){
            throw Throws.logicException("创建Excel工作薄为空！");
        }
        Sheet sheet;
        Row row;
        Cell cell;

        list = new ArrayList<>();
        //遍历Excel中所有的sheet
        for (int i = 0; i < work.getNumberOfSheets(); i++) {
            sheet = work.getSheetAt(i);
            if(sheet==null){continue;}
            //遍历当前sheet中的所有行
            for (int j = sheet.getFirstRowNum(); j <= sheet.getLastRowNum(); j++) {
                row = sheet.getRow(j);
                if(row==null){continue;}
                //遍历所有的列
                List<Object> li = new ArrayList<>();
                for (int y = row.getFirstCellNum(); y < row.getLastCellNum(); y++) {
                    cell = row.getCell(y);
                    li.add(this.getCellValue(cell));
                }
                list.add(li);
            }
        }
        work.close();
        return list;
    }

    /**将数据封装为excel对象，并写入到响应对象response中
     * @param exportFileParams 要导出的文件所需要的参数对象
     * */
    public static HSSFWorkbook excelWriter(ExportFileParams exportFileParams) throws IOException {
        //1.创建一个workbook对应一个excel文件。为了防止客户端的excel版本过低，因此使用excel2003格式
        HSSFWorkbook  wb = new HSSFWorkbook();
        //2.在workbook中创建一个sheet对应excel中的sheet
        HSSFSheet sheet = wb.createSheet();
        //3.在sheet表中添加表头第0行，老版本的poi对sheet的行列有限制
        HSSFRow rowTop = sheet.createRow(0);
        //提取列名
        String[] tableTitle = exportFileParams.getCellArrangeStrategy().split(",");
        if (tableTitle.length == 0){
            throw new UnCaughtException(StateCode.EXCEL_ROWTOP_ERROR.getCode(), StateCode.EXCEL_ROWTOP_ERROR.getMessage());
        }
        if (exportFileParams.getSqlResult().size() == 0){
            throw new UnCaughtException(StateCode.GET_DATA_EMPTY.getCode(), StateCode.GET_DATA_EMPTY.getMessage());
        }
        //4.创建单元格，设置表头
        int cellSize;
        for (cellSize = 0;cellSize<tableTitle.length;cellSize++) {
            if (Constant.En2Ch.containsKey(tableTitle[cellSize])){//英文字段转为中文
                rowTop.createCell(cellSize).setCellValue(Constant.En2Ch.get(tableTitle[cellSize]));
            }else {
                rowTop.createCell(cellSize).setCellValue(tableTitle[cellSize]);
            }
        }
        //5.写入实体数据
        for (int rowNo = 0;rowNo < exportFileParams.getSqlResult().size();rowNo++){
            HSSFRow rowData = sheet.createRow(rowNo + 1);
            //创建单元格设值
            for (int cellNo = 0;cellNo <= cellSize-1;cellNo++){
                String cellName = tableTitle[cellNo];
                Map<String, Object> temp = exportFileParams.getSqlResult().get(rowNo);
                String cellData = String.valueOf(temp.get(cellName));
                rowData.createCell(cellNo).setCellValue(cellData);
            }
        }
        return wb;
    }
    /**将数据封装为excel对象，并写入到响应对象response中,特别针对需要进行列转行的sql结果
     * @param exportFileParams 要导出的文件所需要的参数对象
     * */
    public static HSSFWorkbook excelWriterForC2R(ExportFileParams exportFileParams) throws IOException {
        //1.创建一个workbook对应一个excel文件。为了防止客户端的excel版本过低，因此使用excel2003格式
        HSSFWorkbook  wb = new HSSFWorkbook();
        //2.在workbook中创建一个sheet对应excel中的sheet
        HSSFSheet sheet = wb.createSheet();
        //3.在sheet表中添加表头第0行，老版本的poi对sheet的行列有限制
        HSSFRow rowTop = sheet.createRow(0);
        //提取列名
        String[] tableTitle = exportFileParams.getCellArrangeStrategy().split(",");
        if (tableTitle.length == 0){
            throw new UnCaughtException(StateCode.EXCEL_ROWTOP_ERROR.getCode(), StateCode.EXCEL_ROWTOP_ERROR.getMessage());
        }
        //提取列转行数据
        Map<String,Map<Object,Object>> dsSortMap = exportFileParams.getDsSortMap();
        if (exportFileParams.getDsSortMap().size() == 0){
            throw new UnCaughtException(StateCode.GET_DATA_EMPTY.getCode(), StateCode.GET_DATA_EMPTY.getMessage());
        }
        //4.创建单元格，设置表头
        int cellSize;
        for (cellSize = 0;cellSize<tableTitle.length;cellSize++) {
            if (Constant.En2Ch.containsKey(tableTitle[cellSize])){//英文字段转为中文
                rowTop.createCell(cellSize).setCellValue(Constant.En2Ch.get(tableTitle[cellSize]));
            }else {
                rowTop.createCell(cellSize).setCellValue(tableTitle[cellSize]);
            }
        }
        int rowNo = 0;
        //5.写入实体数据
        for (String key : dsSortMap.keySet()){
            HSSFRow rowData = sheet.createRow(++rowNo);//创建新的行
            //第1列放入日期
            String cellData = key;
            rowData.createCell(0).setCellValue(cellData);
            if (dsSortMap.get(key).size() == 0){//当前日期无数据,则用0补全
                for (int cellNo = 1;cellNo <= cellSize-1;cellNo++){
                    rowData.createCell(cellNo).setCellValue("0");
                }
            }else {
                for (int cellNo = 1; cellNo <= cellSize - 1; cellNo++) {
                    String cellName = tableTitle[cellNo];
                    Map<Object, Object> temp = dsSortMap.get(key);
                    cellData = temp.get(cellName).toString();
                    rowData.createCell(cellNo).setCellValue(cellData);
                }
            }
        }
        return wb;
    }
    /**将excel对象写入到http响应对象的输出流*/
    static void excelObject2response(HSSFWorkbook wb, ServletOutputStream outputStream) throws IOException {
        wb.write(outputStream);
        outputStream.flush();
        outputStream.close();
    }
    /**导出为html表格*/
    static String toHtmlTable(HSSFWorkbook wb){
        StringBuilder context = new StringBuilder();
        Sheet sheet = wb.getSheetAt(0);//第一个sheet页
        context.append("  <style>")
                .append(" table,table tr th, table tr td { border:1px solid;text-align:center;line-height:21px;padding:8px; }\n")
                .append(" table {border-collapse: collapse; padding:2px;}  table tbody tr:first-child>td{font-weight:bold}table tbody tr:first-child>td{background-color:#FCE9DA;} table tbody tr:nth-of-type(odd) {background-color: #f9f9f9;}  </style>");
        context.append("<table>");
        context.append("<tr>");//表头
        Row rowTop = sheet.getRow(0);
        int cellSize = rowTop.getLastCellNum();
        for (int cellNo = 0;cellNo < cellSize;cellNo++){
            context.append("<td>");
            context.append(rowTop.getCell(cellNo).toString());
            context.append("</td>");
        }
        context.append("</tr>");
        for (int rowNo = 1;rowNo<=sheet.getLastRowNum();rowNo++){
            Row row = sheet.getRow(rowNo);
            context.append("<tr>");
            for (int cellNo = 0;cellNo < cellSize;cellNo++){
                context.append("<td>");
                context.append(row.getCell(cellNo).toString());
                context.append("</td>");
            }
            context.append("</tr>");
        }
        context.append("</table>");
        return context.toString();
    }
    /**描述：根据文件后缀，自适应上传文件的版本*/
    private Workbook getWorkbook(InputStream inStr, String fileName) throws IOException {
        Workbook wb;
        String fileType = fileName.substring(fileName.lastIndexOf("."));
        if(excel2003L.equals(fileType)){
            wb = new HSSFWorkbook(inStr);  //2003-
        }else if(excel2007U.equals(fileType)){
            wb = new XSSFWorkbook(inStr);  //2007+
        }else{
            throw Throws.logicException("解析的文件格式有误！");
        }
        return wb;
    }
    /**将excel对象写入文件*/
    static String writeExceltoFile(HSSFWorkbook wb, String saveName){
        String filePath = ExcelUtils.class.getProtectionDomain().getCodeSource().getLocation().getPath();
        if (filePath.endsWith(".jar")) {// 可执行jar包运行的结果里包含".jar"
            // 截取路径中的jar包名
            filePath = filePath.substring(0, filePath.lastIndexOf("/") + 1);
            if (System.getProperty("os.name").contains("Windows")){
                saveName = filePath + "\\temp\\"+saveName;
            }else {//linux环境
                File file = new File(filePath);
                saveName = file.getAbsoluteFile() + "/temp/"+saveName;
            }
        }else {//不包含jar，说明没有打包，就直接用项目就可以了

            saveName = System.getProperty("user.dir") +"\\temp\\"+saveName;
        }
        System.out.println(saveName);
        File file=new File(saveName);
        if(!file.exists())
        {
            try {
                file.createNewFile();
            } catch (IOException e) {
                log.error("excel文件创建失败",e);
                return "";
            }
        }
        //创建文件流
        OutputStream stream;
        try {
            stream = new FileOutputStream(saveName);
        } catch (FileNotFoundException e) {
            log.error("excel本地路径不存在",e);
            return "";
        }
        //写入数据
        try {
            wb.write(stream);
            //关闭文件流
            stream.close();
        } catch (IOException e) {
            log.error("将excel写入文件失败",e);
            return "";
        }
        return saveName;
    }
    /**
     * 描述：对表格中数值进行格式化
     */
    private Object getCellValue(Cell cell){
        Object value = null;
        DecimalFormat df = new DecimalFormat("0");  //格式化number String字符
        SimpleDateFormat sdf = new SimpleDateFormat("yyy-MM-dd");  //日期格式化
        DecimalFormat df2 = new DecimalFormat("0.00");  //格式化数字

        if (cell.getCellTypeEnum() == CellType.STRING){
            value = cell.getRichStringCellValue().getString();
        }else if (cell.getCellTypeEnum() == CellType.NUMERIC){
            if("General".equals(cell.getCellStyle().getDataFormatString())){
                value = df.format(cell.getNumericCellValue());
            }else if("m/d/yy".equals(cell.getCellStyle().getDataFormatString())){
                value = sdf.format(cell.getDateCellValue());
            }else{
                value = df2.format(cell.getNumericCellValue());
            }
        }else if (cell.getCellTypeEnum() == CellType.BOOLEAN){
            value = cell.getBooleanCellValue();
        }else if (cell.getCellTypeEnum() == CellType.BLANK){
            value = "";
        }
        return value;
    }
    /**获取文件类型*/
    public static String GET_FILE_TYPE(String filename) {
        if (filename == null)
            return null;
        int index = filename.lastIndexOf(46);
        if (index == -1)
            return null;
        return filename.substring(index);
    }
}

```