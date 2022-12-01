/*  C -- basic arithmetic operations. */

int main()
{
    int x = 4;
    int y;
    y = 2;

    x = x + y * 2;
    y = x - y / 2;

    put("x:");
    put(x);
    put("y:");
    put(y);
    
    return 0;
}
