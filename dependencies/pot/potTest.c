#include <stdio.h>
#include "pot.h"
#include "../utils/util.h"

int main(void){

    for(int i = 0; i < 100; i++){
        printf("%d\n", POT_readValue());
        sleepMs(100);
    }
    return 0;
}