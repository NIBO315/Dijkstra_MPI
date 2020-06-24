
/**输入邻接矩阵和源顶点，求最短路径。输入邻接矩阵中，位置(x,y)为顶点y到x的边权**/
//#include "stdafx.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**定义M为无穷大**/
#define M 1.7e308

#define BOOL int
#define FALSE 0
#define TRUE  1
#define W(x,y) W[x*nodenum+y]

FILE* fp;
char ch;
char id[20];
int point;
double* W;
double* dist;
BOOL* bdist;
int nodenum = 0;
int S = 0;// 开始节点

/* show err and exit*/
void fatal(char* err)
{
	printf("%s/n", err);
	exit(0);
}

/**从文件中读取字符**/
void GetChar()
{
	fread(&ch,sizeof(char),1,fp);
}

/**读取一个小数，如果为字符M或m，令其值为无穷大**/
double GetNextNum()
{
	double num;

	while(!isdigit(ch) && ch!='M' && ch!='m')
		GetChar();

	if(isdigit(ch))
	{
		point = 0;
		while(isdigit(ch))
		{
			id[point] = ch;
			point ++;
			GetChar();
		}
		id[point] = 0;
		num = atof(id);
	}else{
		num = M;
		GetChar();
	}
	return num;
}


/**读入邻接矩阵**/
//处理器0读入边权邻接矩阵W
double ReadMatrix(int my_rank)
{
	char file[40];
	int i,j;
	double num;
	
	// 计时函数定义
	clock_t begain1,end1;
	clock_t begain2,end2;
	double time1,time2,times;

	printf("Begin to read the matrix!\n");
	printf("The first integer of the file should be the size of the matrix!\n");
	printf("Input the file name of the matrix:");
	fflush(stdout); //刷新输入缓存
	
	begain1 = clock();
	scanf("%s",file);//输入数据文件data.txt
	end1 =clock();

	if((fp = fopen(file,"r")) == NULL)
	{
		fatal("File name input error!");
	}
	num = GetNextNum();
	if(num < 0 || num > 10000)
	{
		fclose(fp);
		fatal("The matrix row input error!");
	}
	nodenum = (int)num;
	printf("Input the start node:");
	fflush(stdout);
	
	begain2 = clock();
	scanf("%d",&S);
	end2 = clock();
	if(S >= nodenum) fatal("The start node input too big!\n");

	W = (double*)malloc(sizeof(double)*num*num);
	if( W == NULL)
	{
		fclose(fp);
		fatal("Dynamic allocate space for matrix fail!");
	}
	for(i=0;i<nodenum;i++)
		for(j=0;j<nodenum;j++)
		{
			W(i,j) = GetNextNum();
		}
	fclose(fp);
	printf("Process %d:Finish reading the matrix,the nodenum is: %d;\n",my_rank,nodenum);

	time1 = (double)(end1-begain1)/(double)CLOCKS_PER_SEC;
	time2 = (double)(end2-begain2)/(double)CLOCKS_PER_SEC;
	times =time1 + time2;
	return times;
}

/**各处理器数据初始化**/
// 初始化dist和bdist数组
void Init(int my_rank,int group_size,int ep)
{
	int i;
	MPI_Status status;
	if(my_rank == 0)
	{
		for(i=1;i<group_size;i++)
		{
			MPI_Send(&W(ep*(i-1),0),ep*nodenum,MPI_DOUBLE,i,i,MPI_COMM_WORLD); //发送消息
		}
	}
	else{
		dist = (double*)malloc(sizeof(double)*ep);
		bdist = (int*) malloc(sizeof(BOOL)*ep);
		W = (double*)malloc(sizeof(double)*ep*nodenum);
		if(W == NULL || dist == NULL || bdist == NULL)
			fatal("Dynamic allocate space for matrix fail!");
		MPI_Recv(W,ep*nodenum,MPI_DOUBLE,0,my_rank,MPI_COMM_WORLD,&status); //接收消息
		for(i=0;i<ep; i++)
		{
			if(i+(my_rank-1)*ep == S)
			{
				dist[i] = 0;
				bdist[i] = TRUE;
			}
			else{
				dist[i] = W(i,S);
				bdist[i] = FALSE;
			}
		}
	}
}


/**输出邻接矩阵**/
void OutPutMatrix(int my_rank,int group_size,int ep,int mynum)
{
	int i,j;

	if(my_rank != 0)
	{
		for(i=0;i<mynum;i++)
		{
			printf("Processor %d:\t",my_rank);
			for(j=0;j<nodenum;j++)
			{
				if(W(i,j) > 1000000) printf("M\t");
				else printf("%d\t",(int)W(i,j));
			}
			printf("\n");
		}
	}
}


/**输出结果**/
void OutPutResult(int my_rank,int group_size,int ep,int mynum)
{
	int i,j;
	if(my_rank != 0)
	{
		for(i=0;i<mynum;i++)
		{
			printf("node  %d:\t%d\n",(my_rank-1)*ep+i,(int)dist[i]);
		}
	}
}

/**算法主循环**/
void FindMinWay(int my_rank,int group_size,int ep,int mynum)
{
	int i,j;
	int index,index2;
	double num,num2;
	int calnum;
	MPI_Status status;
	int p = group_size;

	for(i=0; i<nodenum;i++)
	{
		index = 0;
		num = M;

		/**步骤(3.1)**/
		// 各处理器找出自己负责范围内未搜索节点中最小的dist
		for(j=0;j<mynum;j++)
		{
			if(dist[j] < num && bdist[j]==FALSE)
			{
				num = dist[j];
				index = ep*(my_rank-1)+j;
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);//同步点，构建一个屏障，任何进程都没法跨越屏障，直到所有的进程都到达屏障
	
		/**步骤(3.2)**/
		// 各处理器协作对p-1各index求最小
		calnum = group_size-1;
		while(calnum > 1)
		{
			/**节点数目为偶数时**/
			if(calnum % 2 == 0)
			{
				calnum = calnum/2;
				if(my_rank > calnum)//如果进程号靠后就向前面的进程发送结果数据
				{
					MPI_Send(&index,1,MPI_INT,my_rank-calnum,
						my_rank-calnum,MPI_COMM_WORLD);
					MPI_Send(&num,1,MPI_DOUBLE,my_rank-calnum,
						my_rank-calnum,MPI_COMM_WORLD);
				}else if(my_rank!=0){//进程号不为0且是前半部分的进程就接收结果数据
					MPI_Recv(&index2,1,MPI_INT,my_rank+calnum,my_rank,
						MPI_COMM_WORLD,&status);
					MPI_Recv(&num2,1,MPI_DOUBLE,my_rank+calnum,my_rank,
						MPI_COMM_WORLD,&status);
					if(num2 < num)
					{
						num = num2;
						index = index2;
					}
				}
			}else{
			/**节点数目为奇数时**/
				calnum = (calnum+1)/2;
				if(my_rank > calnum)
				{
					MPI_Send(&index,1,MPI_INT,my_rank-calnum,
						my_rank-calnum,MPI_COMM_WORLD);
					MPI_Send(&num,1,MPI_DOUBLE,my_rank-calnum,
						my_rank-calnum,MPI_COMM_WORLD);
				}else if(my_rank!=0 && my_rank < calnum){
					MPI_Recv(&index2,1,MPI_INT,my_rank+calnum,my_rank,
						MPI_COMM_WORLD,&status);
					MPI_Recv(&num2,1,MPI_DOUBLE,my_rank+calnum,my_rank,
						MPI_COMM_WORLD,&status);
					if(num2 < num)
					{
						num = num2;
						index = index2;
					}
				}
			}
			MPI_Barrier(MPI_COMM_WORLD);//同步点，构建一个屏障，任何进程都没法跨越屏障，直到所有的进程都到达屏障
		}

		/**步骤(3.3)**/
		// 处理器1广播通知其他处理器自己的num和index
		MPI_Bcast(&index,1,MPI_INT,1,MPI_COMM_WORLD);
		MPI_Bcast(&num,  1,MPI_DOUBLE,1,MPI_COMM_WORLD);
		/**步骤(3.4)**/
		//更新dist数组
		for(j=0;j<mynum;j++)
		{
			if((bdist[j]==FALSE)&&(num + W(j,index) < dist[j]))
				dist[j] =num  + W(j,index);
		}

		/**步骤(3.5)**/
		// 更新bdist数组
		if(my_rank == index/ep+1)
		{
			bdist[index%ep] = TRUE;
		}
		MPI_Barrier(MPI_COMM_WORLD);//同步点，构建一个屏障，任何进程都没法跨越屏障，直到所有的进程都到达屏障
	}
}

/**主函数**/
int main(int argc,char** argv)
{
	int group_size,my_rank;
	MPI_Status status;
	int i,j;
	int ep;
	int mynum;
	
	double *p = NULL;//获取ReadMatrix()函数返回的时间数据值
	double a;

	clock_t start,finish;
	double time;

	start = clock();

	MPI_Init(&argc,&argv);/*并行开始 初始化*/
	MPI_Comm_size(MPI_COMM_WORLD,&group_size);// 确定一个通信域中的进程总数
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);//获取调用该函数的进程的进程号 
	/*
	if(group_size <= 1)
	{
		printf("Not enough processor!\n");
		exit(0);
	}
	*/
	/**步骤(1)**/
	if(my_rank == 0)//如果进程号是0，执行的操作
	{
		double times;
		times = ReadMatrix(my_rank);//读数据
		p = &times;

		for(i=1;i<group_size;i++)
		{
			MPI_Send(&nodenum,1,MPI_INT,i,i,MPI_COMM_WORLD);//将节点数发送给各进程
			MPI_Send(&S,1,MPI_INT,i,i,MPI_COMM_WORLD);//开始节点发送给各进程
		}
	}else{//如果进程号不是0执行的操作
		MPI_Recv(&nodenum,1,MPI_INT,0,my_rank,MPI_COMM_WORLD,&status);
		MPI_Recv(&S,1,MPI_INT,0,my_rank,MPI_COMM_WORLD,&status);
	}
 
	ep = nodenum/(group_size-1);//除去进程0外为每个进程分配n=N/p各节点
	if(ep*group_size-ep < nodenum) ep++;

	if(ep*my_rank <= nodenum)
	{
		mynum = ep;
	}else if(ep*my_rank < nodenum+ep)
	{
		mynum = nodenum - ep*(my_rank-1);
	}
	else mynum = 0;
	if (my_rank == 0) mynum = 0;
    /**步骤(2)**/
	Init(my_rank,group_size,ep);//各处理器进行初始化

	OutPutMatrix(my_rank, group_size, ep, mynum);//输出每个进程负责的节点数据

	/**步骤(3)**/
	// 进入算法主循环
	FindMinWay(my_rank,group_size,ep,mynum);

	OutPutResult(my_rank,group_size,ep,mynum);

	MPI_Finalize();//并行结束

	free(W);
	free(dist);
	free(bdist);

	a= *p;	
	finish = clock();

	time = (double)(finish - start)/CLOCKS_PER_SEC;
	
	printf("运行时间为：%lf毫秒",(time-a)*1000);

	return 0;
}

