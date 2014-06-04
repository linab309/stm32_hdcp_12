
//延时纳秒或者最小时间
void DelayNs(int n)
{
	for(;n>0;n--)
		;
}
//延时微秒
void DelayUs(int n)
{
	int i,k;
	for(i=0;i<21;i++)
	{
		k = n;
		for(;k>0;k--)
			;
	}
}
//延时毫秒
void DelayMs(int n)
{
	int i,k;
	for(i=0;i<21600;i++)
	{
		k = n;
		for(;k>0;k--)
			;
	}
}

