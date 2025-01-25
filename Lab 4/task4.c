#include <string.h>
#include <stdio.h>

int digit_cnt(char* input){
    int counter = 0;
    for (int i = 0; input[i] != 0; i++)
    {
        if (input[i] >= '0' && input[i] <= '9')
        {
            counter++;
        }
    }
    return counter;
}



int main(int argc, char **argv){
    if (argc > 1)
    {
        int count = digit_cnt(argv[1]);
        printf("There are %d digits\n", count);
    }
    return 0;
}


