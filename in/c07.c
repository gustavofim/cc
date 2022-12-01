/* C -- array. */

int main()
{
    int arr1[5];
    int arr2[5];
	
	int i = 0;
	while (i < 5) {
		arr2[i] = i + 1;
		i = i + 1;
	}

    i = 0;
	while (i < 5) {
		arr1[i] = arr2[4 - i];
        put(arr1[i]);
		i = i + 1;
	}

    return 0;
}
