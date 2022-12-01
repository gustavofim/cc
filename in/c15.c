/* Messing with chars */

char getchar()
{
	char a;
	a = get();
	return a;
}

void main()
{
    char a;
	char b[5];
	int i = 0;
	while (i < 4) {
		put("Enter a char:");
		a = getchar();
		b[i] = a;
		i = i + 1;
	}
	put("Resulting 4 digit string:");
	put(b);
}
