��    5      �  G   l      �     �  &   �  I   �  j   	  j   t  Q   �  �   1  M   �  ?         P  !   q  �   �     Q  9   g  h   �  J   
	  9   U	  '   �	  4   �	  ,   �	  I   
  (   c
  5   �
  d   �
  x   '     �  (   �  9   �  1   
  $   <  M   a     �     �  <   �       /   %  �   U  H   �  W   $  P   |  5   �       8     Z   S     �  F   �  0   �     .  C   4  )   x  0   �  0   �           (   "  N   K  n   �  n   	  d   x  s   �  L   Q  �   �  $   -  &   R  �   y     .  9   F  p   �  S   �  5   E  )   {  4   �  ,   �  E     +   M  T   y  R   �  {   !     �     �  ?   �  ,     *   .  j   Y     �     �  �   �     �     �  \   �  A     E   M  P   �  4   �       ?   4  V   t     �  Z   �     3     R  7   Y  -   �  *   �  0   �     %   /                                        1   *          #   .                     4          +   &   2      -   $   3       0                                "   5   	          (      '           !      ,                               )         
    ' and exiting. ' is a directory. Not yet implemented. 'between' arguments are all POSIXct but have mismatched tzone attributes: 'between' function the 'x' argument is a POSIX class while 'lower' was not, coercion to POSIX failed with: 'between' function the 'x' argument is a POSIX class while 'upper' was not, coercion to POSIX failed with: 'between' lower= and upper= are both POSIXct but have different tzone attributes: 'data.table' relies on the package 'yaml' to write the file header; please add this to your library with install.packages('yaml') and try again. 'sets' contains a duplicate (i.e., equivalent up to sorting) element at index . Only integer, double or character columns may be roll joined. . Please align their time zones. . The UTC times will be compared. ; as such, there will be duplicate rows in the output -- note that grouping by A,B and B,A will produce the same aggregations. Use `sets=unique(lapply(sets, sort))` to eliminate duplicates. ; expecting length 2. A column may not be called .SD. That has special meaning. All columns used in 'sets' argument must be in 'by' too. Columns used in 'sets' but not present in 'by': Argument 'by' must be a character vector of column names used in grouping. Argument 'by' must have unique column names for grouping. Argument 'id' must be a logical scalar. Argument 'sets' must be a list of character vectors. Argument 'sort' should be logical TRUE/FALSE Argument 'x' is a 0-column data.table; no measure to apply grouping over. Argument 'x' must be a data.table object Attempting roll join on factor column when joining x. Character vectors in 'sets' list must not have duplicated column names within a single grouping set. Expression passed to grouping sets function must not update by reference. Use ':=' on results of your grouping function. File ' If you intended to overwrite the file at Input data.table must not contain duplicate column names. Input has no columns; creating an empty file at ' Input has no columns; doing nothing. Not yet implemented NAbounds=TRUE for this non-numeric and non-character type Perhaps you meant %s? RHS has length() Some lower>upper for this non-numeric and non-character type TRANSLATION CHECK The first element should be the lower bound(s); There exists duplicated column names in the results, ensure the column passed/evaluated in `j` and those in `by` are not overlapping. Using integer64 class columns require to have 'bit64' package installed. When using `id=TRUE` the 'j' expression must not evaluate to a column named 'grouping'. When using `id=TRUE` the 'x' data.table must not have a column named 'grouping'. between has been passed an argument x of type logical class must be length 1 dateTimeAs must be 'ISO','squash','epoch' or 'write.csv' internal error, package:xts is on search path but could not be loaded via requireNamespace is type logicalAsInt has been renamed logical01. Use logical01 only, not both. the second element should be the upper bound(s). to i. trying to use integer64 class when 'bit64' package is not installed which is not supported by data.table join with an empty one, please use file.remove first. x being coerced from class: matrix to data.table Project-Id-Version: data.table 1.12.5
PO-Revision-Date: 2019-10-04 17:06+08
Last-Translator: Michael Chirico <MichaelChirico4@gmail.com>
Language-Team: Mandarin
Language: Mandarin
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
 ' 并退出。 '是个目录。还没有编程实现。 'between' 的参数均为 POSIXct 类型但时区属性（tzone）不匹配： 'between' 中的 'x' 参数为 POSIX 类，而 'lower' 并不是，将 'lower' 自动转换成 POSIX 失败： 'between' 中的 'x' 参数为 POSIX 类，而 'upper' 并不是，将 'upper' 自动转换成 POSIX 失败： 'between' 中的 lower= 和 upper= 均为 POSIXct 类型但却有不同的时区属性（tzone）： 'data.table' 依赖于 'yaml' 包来写文件头；请运行 install.packages('yaml') 安装 'yaml' 包后再试。 'sets' 的索引含有重复的元素，在做排序时的作用是对等的 联接时。但只有整数（integer）、双精度（double）或字符（character）类型的列可以使用滚动联接（roll join）。 。请确保二者的时区一致。 。将采用 UTC 时间进行比较。 ；同样的，输出中也会包含重复的行（注意按照A、B分组与按照B、A分组的结果是一样的。）使用 `sets=unique(lapply(sets, sort))` 来消除重复。 ；其长度应为 2。 无法将列命名为 .SD，因为 .SD 为特殊符号。 在 'sets' 参数中应用的所有列也必须在 'by' 中。当前 'sets' 包含而 'by' 中不含的列有： 'by' 参数必须是一个字符向量，向量的元素是列名，用于分组。 'by' 参数用于分组，不可包含重复列名。 'id' 参数必须是一个逻辑标量。 'sets' 参数必须是一个字符向量的列表。 参数 'sort' 应为逻辑值 TRUE 或 FALSE 'x' 参数是一个 0 列的 data.table；无法对其应用分组。 'x' 参数必须是一个 data.table 对象 试图滚动联接（roll join）因子类型（factor）的列，这发生于将 x. 在单个分组中，'sets' 列表中的字符串向量不能有重复的列名。 传递给分组相关函数的表达式不能通过引用更新。请在你的分组函数返回的结果中使用 ':=' 。 文件' 如果你打算覆盖文件 作为输入的 data.table 对象不能含有重复的列名。 输入没有列，将创建一个空文件 ' 输入没有列，不执行任何操作。 对这种非数值（numeric）和非字符（character）的类型，尚未实现 NAbounds=TRUE 的功能 或许你想用的是 %s？ 右手侧（RHS）的长度为 对于该非数值（numeric）和非字符（character）类型的输入，存在一部分下界（lower）> 上界（upper）的情况 中文 第一个元素应为下界； 结果中存在重复的列名，请确保 `j` 和 `by` 传递的列中没有发生重叠。 要在列中使用 integer64 类，需要先安装 'bit64' 包。 当 `id=TRUE` 时，'j' 表达式不能针对 'grouping' 列求值。 当使用 `id=TRUE` 时，data.table 'x' 不能包含名为 'grouping' 的列。 传入 between 的参数 x 为逻辑（logical）型 class 的长度必须为 1 dateTimeAs 必须是 'ISO'，'squash'，'epoch' 或 'write.csv' 内部错误，在搜索时找到了 xts 包，但无法使用 requireNamespace 加载 的类型为 logicalAsInt 已重命名为 logical01。不要同时使用它们，仅使用 logical01。 第二个元素应为上界。 与 i. 试图使用 intger64 类型但 'bit64' 包尚未安装 ，该类型无法用于 data.table 的联接 为空文件，请先使用 file.remove。 x 的类将强制从 matrix 转变为 data.table 