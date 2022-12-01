/* Jump statements and mod */

void main()
{
	float i = 1;
	
    put("Printing odd numbers from 1 to 9:");
    while (i) {
		if (i == 10) {
			break;
		} else if (i % 2 == 0) {
			i = i + 1;
			continue;
		}
		put(i);
		i = i + 1;
	}
}
