/* Messing with arrays */

int main()
{
	int a[10];
	int b[20];
	a[0] = 1;
	b[0] = 1;
	a[b[0]] = 4;
	b[a[b[0]]] = 5;
	
	put(a[1]);
	put(b[4]);

    return 0;
}
