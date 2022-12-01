/* Array as parameter */

void hello(char a[10])
{
	a[0] = 'H';
	a[1] = 'e';
	a[2] = 'l';
	a[3] = 'l';
	a[4] = 'o';
}

void main()
{
    char a[5];
    hello(a);
	put(a);
}
