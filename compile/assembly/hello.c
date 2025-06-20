#include <stdio.h>

int sum(int a, int b)
{
	return a + b;
}

int main(int argc, char *argv[])
{
	int n1 = 10;
	int n2 = 5;
	int s = sum(n1, n2);
	printf("n1:%d, n2:%d, sum:%d\n", n1, n2, s);
	return 0;
}
