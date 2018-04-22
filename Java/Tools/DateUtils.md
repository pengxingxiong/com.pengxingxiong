## 获取日期之间的间隔列表
```
/* 通过开始和结束时间获取中间的时间列表 */
private List<String> getDateInterval( String timeStart, String timeEnd )
{
    LocalDate    start    = LocalDate.parse( timeStart );
    LocalDate    end    = LocalDate.parse( timeEnd );
    /* 用起始时间作为源头，按照每天加一天的方式创建一个无限流 */
    return(Stream.iterate( start, localDate - > localDate.plusDays( 1 ) )
    /* 截断无限流 */
    .limit( ChronoUnit.DAYS.between( start, end ) + 1 )
    /* 转化为字符串 */
    .map( LocalDate: : toString )
    /* 把流收集为List */
    .collect( Collectors.toList() ) );
}
```