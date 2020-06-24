#include<stdio.h>
#include<stdlib.h>

int main()

{

	int i;
	for(i=0; i<2500; i++) //随机产生10个数。
	{
		printf("%d ", rand()%100);
		if((i+1)%100==0)
			printf("\n");
	}
	return 0;
}