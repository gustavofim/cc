float fsum(float a, float b)
{
    return a + b;
}

int main()
{
	int i = 12;
    float a = fsum(1, fsum(i, 2.5));
	put(a);
    return 0;
}
