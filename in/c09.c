/* C -- nested while. */

int main()  
{  
   int n = 3;
   int i = 1;
   int j = 1;
   while (i <= n)
   {  
       while (j <= n)
       {  
           put(i*j);
		   j = j + 1;
       }
	   i = i + 1;
	   j = 1;
   }  
   
   return 0;
}
