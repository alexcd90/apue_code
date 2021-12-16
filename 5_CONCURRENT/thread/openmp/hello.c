#include <stdlib.h>
#include <stdio.h>

int main(){

#pragma omp parallel
{
    puts("hello");
    puts("world");
}

}