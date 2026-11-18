int main()
{
    // 例程：整数、浮点与字符常量的基本运算
    int a = 12;
    int b = 0x1F;
    float rate = 1.5f;
    char mark = 'x';
    if (a < b && b >= 30)
    {
        a = a + b;
    } 
    while (a > 0) 
    {
        a = a - 1;
    }

    // 行注释：把 a 归零
    /* 块注释 */
    return a;
}

