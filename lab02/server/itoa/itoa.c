#include "itoa.h"

void swap(char *a, char *b)
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

void reverse(char str[], int length) 
{ 
    	int start = 0;
    	int end = length - 1;
    	
    	while (start < end)
    	{
        	swap(str + start, str + end);
        	++start;
		--end;
	}
}

char *itoa(int num, char *str, int base)
{
	int i = 0;
	bool is_neg = false;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	if (num < 0) {
		is_neg = true;
		num = -num;
	}

	while (num != 0) {
		int rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10 + 'a') : (rem + '0');
		num /= base;
	}

	if (is_neg) {
		str[i++] = '-';
	}

	str[i] = '\0';

	reverse(str, i);

	return str;
}
