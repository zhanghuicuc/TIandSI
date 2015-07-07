TIandSI

雷霄骅 Lei Xiaohua 张晖 Zhang Hui
leixiaohua1020@126.com
zhanghuicuc@gmail.com
中国传媒大学/数字电视技术
Communication University of China / Digital TV Technology

本工程可以计算一个压缩视频或原始码流的时间信息TI（Temporal perceptual Information，
也可以称时间复杂度）和空间信息SI（Spatial perceptual Information，也可以
称空间复杂度）。计算方法出自标准：ITU-R BT.1788。使用了SSE技术提高计算效率。

Professional: 支持压缩码流和像素数据作为输入
Graphical: 支持图形化显示TI（时间信息）和SI（空间信息）

Help

----------------------------------------
Options:
-i      Input file URL
-o      Output.csv file URL [Default is {Input Name}.csv]
-l      Limit calculate frame number
-g      Show result Graphically [Option is 'ti' or 'si']
-n      Interval to show result to the screen [Default is 5]
-h      Show this text.
----------------------------------------
Examples:
TISIprog -i test.avi
TISIprog -i test.avi -o test_cal.csv
TISIprog -i test.avi -g ti