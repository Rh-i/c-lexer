// 分支与循环，只用子集内的运算符
int main()
{
    int n = 10;
    int sum = 0;
    while (n > 0)
    {
        sum = sum + n;
        n = n - 1;
    }
    if (sum >= 55 && n == 0)
    {
        sum = 0;
    }
    return sum;
}
