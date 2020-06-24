# Dijkstra_MPI
# parallel 文件里包含的是并行的shortest.c程序和shortest.exe程序以及输入的数据；包括输入30节点、40节点和100节点
# serial 文件里包含的是串行的程序以及生成数据的randoms程序
# 环境配置：需安装MPI和可执行c文档的工具VC6.0（本程序使用）或VS或终端运行等
# 执行步骤： 
#         1. 安装MPI https://blog.csdn.net/baidu_33258926/article/details/51372904?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522159298185419724811828800%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=159298185419724811828800&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~baidu_landing_v2~default-6-51372904.pc_search_back_js&utm_term=MPI%E5%AE%89%E8%A3%85
#         2. 安装VC6.0；
#         3. 打开parallel文件夹运行shortest.c直接创建项目，并覆盖原有的Debug文件夹；
#         4. 在serial中，修改randoms.c中的i<1600,如果需要100个节点做测试，则使i<10000,生成100个节点对应的10000个权重数据；
#         5. 打开终端cmd，并cd到serial文件中的randoms下的Debug文件下，输入指令 randoms > 100node.txt，Debug文件下会生成100node.txt文件；
#         6. 打开100node.txt文件，在首行增加数字100表示100个节点，并按Enter换行，使得100独立一行，保存退出；
#         7. 将100node.txt文件copy一份放入到parallel文件夹下的Debug文件夹中；
#         8. 打开终端（用管理员命令执行），并cd到parallel\Debug文件中，终端输入 mpiexec -np 3 shortest
#         9. 终端提示输入节点文件：100node.txt
#         10. 终端提示输入开始节点：1
#         11. 如需保存结果，可使用：mpiexec -np 3 shortest > result.txt 注：回车后不会提示输入节点文件和开始节点，需自行输入节点文件和开始节点
#         12. 运行完成，查看保存结果(result.txt)
