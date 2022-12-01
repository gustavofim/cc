/* C -- factorial. */

int main()
{
    int i = 5;
    int fact = 1;
    put("The fact of");
    put(i);
    while (i > 0) {
        fact = fact * i;
        i = i - 1;
    }

    put("is");
    put(fact);

    return 0;
}
