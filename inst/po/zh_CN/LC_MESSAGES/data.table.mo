��    R      �  m   <      �      �  &        9  +   S  4        �  6   �  Y   �     X     n     �  )   �  g   �  8   4	  I   m	  6   �	  �   �	     
  D   �  D   �  >   *  >   i  K   �  R   �  *   G  8   r  :   �  {   �  5   b  l   �  E     /   K  4   {  E   �  �   �  m   �  l   �  k   \  #   �  '   �  �        �  !   �  �   �  �   �  �   &  &   �  ?   �  ;   "  <   ^  ?   �  C   �          <     R  (   i     �  A   �  P   �  @   8     y  %   �     �  +   �  .   �  �   "       #   '  "   K  #   n  #   �  $   �     �     �  "         5  "   V  "   y     �  *   �     �  N  �     M  (   g     �     �  &   �     �  4   �  E   -     s     �     �  +   �  N   �  6   1  5   h  -   �  �   �    b  R      T   �   L   '!  N   t!  ]   �!  M   !"  )   o"  9   �"  P   �"  u   $#  9   �#  j   �#  G   ?$  *   �$  6   �$  :   �$  �   $%  m   �%  l   &  S   �&  )   �&  )   	'  �   3'     �'     �'  �   (  �   �(  �   @)  -   �)  K   �)  =   ?*  H   }*  8   �*  6   �*     6+     Q+     d+  +   x+     �+  =   �+  B   �+  A   ;,     },  '   �,     �,  *   �,  ,   �,  �   (-     �-     .  !   %.  $   G.  $   l.  '   �.     �.  #   �.     �.     /      2/     S/     o/      �/     �/            9      0                B           2   %       .   K      '   M   -      4                     L   H       (      <                 A   *   ,               ?   G   )   E       #   $          >         O   R       5      =       !               C   ;      I   8   /           +   
              Q   N       &   @       7   1      J      :          6      	       3          D   "   F          P      0/1 column will be read as %s
   File copy in RAM took %.3f seconds.
   No NAstrings provided.
   None of the NAstrings look like numbers.
   One or more of the NAstrings looks like a number.
   Opening file %s
   Using %d threads (omp_get_max_threads()=%d, nth=%d)
   `input` argument is provided rather than a file name, interpreting as raw text to read
   show progress = %d
   skip num lines = %llu
   skip to string = <<%s>>
 Avoidable %.3f seconds. %s time to copy.
 Column %d of input list x is length %d, inconsistent with first column of that item which is length %d. Could not allocate (very tiny) group size thread buffers Failed to allocate TMP or UGRP or they weren't cache line aligned: nth=%d Failed to allocate parallel counts. my_n=%d, nBatch=%d GForce mean can only be applied to columns, not .SD or similar. Likely you're looking for 'DT[,lapply(.SD,mean),by=,.SDcols=]'. See ?data.table. GForce min can only be applied to columns, not .SD or similar. To find min of all items in a list such as .SD, either add the prefix base::min(.SD) or turn off GForce optimization using options(datatable.optimize=1). More likely, you may be looking for 'DT[,lapply(.SD,min),by=,.SDcols=]' Internal error. Argument 'cols' to CanyNA is type '%s' not 'integer' Internal error. Argument 'cols' to Cdt_na is type '%s' not 'integer' Internal error. Argument 'x' to CanyNA is type '%s' not 'list' Internal error. Argument 'x' to Cdt_na is type '%s' not 'list' Internal error: Failed to allocate counts or TMP when assigning g in gforce Internal error: NAstrings is itself NULL. When empty it should be pointer to NULL. Internal error: NUMTYPE(%d) > nLetters(%d) Internal error: column not supported, not caught earlier Internal error: gsum returned type '%s'. typeof(x) is '%s' Internal error: invalid ties.method for frankv(), should have been caught before. please report to data.table issue tracker Internal error: last byte of character input isn't \0 Internal error: nrow=%d  ngrp=%d  nbit=%d  shift=%d  highSize=%d  nBatch=%d  batchSize=%d  lastBatchSize=%d
 Internal error: o's maxgrpn attribute mismatches recalculated maxgrpn Internal error: unknown ties value in frank: %d Internal error: unsupported type at the end of gmean Item %d of 'cols' is %d which is outside 1-based range [1,ncol(x)=%d] No non-missing values found in at least one group. Coercing to numeric type and returning 'Inf' for such groups to be consistent with base No non-missing values found in at least one group. Returning 'Inf' for such groups to be consistent with base No non-missing values found in at least one group. Returning 'NA' for such groups to be consistent with base Previous fread() session was not cleaned up properly. Cleaned up ok at the beginning of this fread() call.
 System errno %d unmapping file: %s
 System error %d unmapping view of file
 The sum of an integer column for a group was more than type 'integer' can hold so the result has been coerced to 'numeric' automatically for convenience. This gsum took (narm=%s) ...  Timing block %2d%s = %8.3f   %8d
 Type '%s' not supported by GForce mean (gmean) na.rm=TRUE. Either add the prefix base::mean(.) or turn off GForce optimization using options(datatable.optimize=1) Type '%s' not supported by GForce min (gmin). Either add the prefix base::min(.) or turn off GForce optimization using options(datatable.optimize=1) Type '%s' not supported by GForce sum (gsum). Either add the prefix base::sum(.) or turn off GForce optimization using options(datatable.optimize=1) Type 'complex' has no well-defined min Unable to allocate %d * %d bytes for counts in gmean na.rm=TRUE Unable to allocate %d * %d bytes for si in gmean na.rm=TRUE Unable to allocate %d * %d bytes for sum in gmean na.rm=TRUE Unable to allocate %s of contiguous virtual RAM. %s allocation. Unable to allocate TMP for my_n=%d items in parallel batch counting Unsupported column type '%s' [01] Check arguments
 [02] Opening the file
 dec='' not allowed. Should be '.' or ',' file not found: %s freadMain: NAstring <<%s>> has whitespace at the beginning or end freadMain: NAstring <<%s>> is recognized as type boolean, this is not permitted. gather implemented for INTSXP, REALSXP, and CPLXSXP but not '%s' gather took ...  gforce assign high and low took %.3f
 gforce eval took %.3f
 gforce initial population of grp took %.3f
 irowsArg is neither an integer vector nor NULL is.sorted (R level) and fsorted (C level) only to be used on vectors. If needed on a list/data.table, you'll need the order anyway if not sorted, so use if (length(o<-forder(...))) for efficiency in one step, or equivalent at C level l is not an integer vector mean is not meaningful for factors. min is not meaningful for factors. nrow [%d] != length(x) [%d] in gmin nrow [%d] != length(x) [%d] in gsum nrow must be integer vector length 1 nrow==%d but must be >=0 o has length %d but sum(l)=%d quote == dec ('%c') is not allowed sep == dec ('%c') is not allowed sep == quote ('%c') is not allowed sum is not meaningful for factors. type '%s' is not yet supported x must be either NULL or an integer vector x must be type 'double' Project-Id-Version: data.table 1.12.5
Report-Msgid-Bugs-To: 
POT-Creation-Date: 2019-10-27 09:58+0800
PO-Revision-Date: 2019-10-04 17:06+08
Last-Translator: Michael Chirico <MichaelChirico4@gmail.com>
Language-Team: Mandarin
Language: Mandarin
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
  0/1 列被读取为  %s
 内存上的文件复制耗时 %.3f 秒
 未提供 NAstrings  没有 NAstrings 为数值 一个或多个 NAstrings 类似数值 打开文件 %s
 使用 %d 线程 (omp_get_max_threads()=%d, nth=%d)
 提供 `input` 参数而非文件名，理解为原始的文本读取 显示进程  %d
 跳过行数为 %llu
 跳转至 string = <<%s>>
 可避免的 %.3f ，秒。 %s 复制用时 输入列表x的列 %d 长度为  %d，不同于第一列的该项长度为 %d 无法分配（极小）块组大小的线程缓冲区 分配TMP或UGRP失败或缓存行不一致： nth=%d 分配并行计算失败，my_n=%d, nBatch=%d GForce 求平均值仅适用于列，而不能用于 .SD 或其它。或许你在查找 'DT[,lapply(.SD,mean),by=,.SDcols=]'。参见 ?data.table 。 GForce 求最小值（min）仅适用于列，而不能用于 .SD 或其它。要找到列表中（如 .SD）所有元素的最小值，须使用 base::min(.SD) 或者设置 options(datatable.optimize=1) 关闭 GForce 优化。你更像是在查找 'DT[,lapply(.SD,min),by=,.SDcols=]' 内部错误：参数 'cols' 关于 CanyNA 是 '%s' 类型而不是'integer'类型 内部错误：参数 'cols' 关于 Cdt_na 是 '%s' 类型而不是 'integer' 类型 内部错误：参数 'x' 关于 CanyNA 是 '%s' 类型而不是'list'类型 内部错误：参数 'x' 关于 Cdt_na 是 '%s' 类型而不是 'list' 类型 内部错误：在 gforce 中为 g 赋值时，未能成功为 counts 或者 TMP 分配空间 内部错误：NAstrings 自身为空值。当清空该项会指向NULL空值 内部错误：NUMTYPE(%d) > nLetters(%d) 内部错误：列有不支持类型，未被前置识别 内部错误：gsum 返回的类型是 '%s'，而 typeof(x) 的结果则是 '%s' 内部错误：对于 frankv()的无效值ties.method，应在之前被捕获。请报告给 data.table issue tracker 内部错误：字符输入的最后一个字节不是 \0 内部错误：nrow=%d ngrp=%d  nbit=%d  shift=%d  highSize=%d  nBatch=%d  batchSize=%d  lastBatchSize=%d
 内部错误：o 的 maxgrpn 属性与重新计算的 maxgrpn 不匹配 内部错误：frank中有未知的ties值 内部错误：gmean 结尾处存在不支持的类型 'cols' 的 %d 项为 %d ，超出1的范围 [1,ncol(x)=%d] 在至少一个分组中没有发现缺失值。为了与 base 保持一致，将这些组强制转换为数值类型并返回 ‘Inf’。 在至少一个分组中没有发现缺失值。为了与 base 保持一致，这些组将返回 ‘Inf’。 在至少一个分组中没有发现缺失值。为了与 base 保持一致，这些组将返回 ‘NA’。 之前的会话fread()未正确清理。在当前 fread() 会话开始前清理好
 系统错误 %d 取消映射文件： %s
 系统错误 %d 取消映射文件视图
 某整数列分组求和的结果中，出现了超过了整型（interger）数值所允许最大值的情况，故结果被自动转换为数值类型（numeric） gsum 占用了 (narm=%s) ... 定时块 %2d%s = %8.3f   %8d
 GForce 求均值（gmean）不支持类型 '%s'。请使用 base::mean(.) 或者设置 options(datatable.optimize=1) 关闭 GForce 优化 类型'%s'不支持应用 GForce 最小值（gmin） 优化。你可以添加前缀 base::min(.) 或者使用 options(datatable.optimize=1) 关闭 GForce 优化 GForce 求和（gsum）不支持类型 '%s'。请使用 base::sum(.) 或者设置 options(datatable.optimize=1) 关闭 GForce 优化 复数不能比较大小，故没有最小值 无法为 gmean na.rm=TRUE 的计数（counts）分配 %d * %d 字节空间 无法为 gmean na.rm=TRUE 的 si 分配 %d * %d 字节空间 无法为 gmean na.rm=TRUE 的总和（sum）分配 %d * %d 字节空间 无法分配 %s 的连续虚拟内存。 %s 已分配。 无法分配TMP给并行批处理计算的 my_n=%d 项 不支持的列类型 '%s' [01] 参数检查
 [02]  打开文件
 dec='' 不允许，应该为 '.' 或者 ',' 文件未找到： %s freadMain: NAstring <<%s>>  在开始或者结束处有空白 freadMain: NAstring <<%s>> 被识别为布尔型，这是不允许 gather 已支持 INTSXP，REALSXP 和 CPLXSXP，但不支持 '%s' gather 用了 ... gforce 分配 high 和 low 用了 %.3f
 gforce eval 用了 %.3f
 grp 的 gforce 初始群体数占了 %.3f
 irowsArg 既不是整数向量也不是 NULL is.sorted (R层面)和 fsorted (C 层面)使用对象仅为向量。如果需要用于list或data.table，需要对其进行排序如果(length(o<-forder(...)))，使用提高效率，或相当于在  l 不是整数向量 因子的平均值没有意义 因子的最小值没有意义。 gmin 中 nrow [%d] != length(x) [%d] gsum 中 nrow [%d] != length(x) [%d] nrow 必须为长度为1的整型向量 nrow==%d 但是必须  >=0 o 的长度为 %d，但 sum(l) = %d quote == dec ('%c') 不允许 sep == dec ('%c') 不允许 sep == quote ('%c') 不被允许 因子的和没有意义。 类型 '%s' 目前不支持 x 必须为空值或整型向量 x 必须为浮点数类型 