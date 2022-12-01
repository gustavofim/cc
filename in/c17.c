/* Messing with globals */
int i = 4;
int a[i];

void puts() {
	put(a[i-1]);
}

int main()
{
	a[3] = 101;
	puts();
    return 0;
}
