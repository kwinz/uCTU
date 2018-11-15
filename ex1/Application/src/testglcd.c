#include "glcd.h"
#include <stdbool.h>
#include "rand.h"

void background(){
    
}

int main (void){
    glcdInit();
    xy_point p1 = {5,20}, p2 = {80,80};

    glcdDrawLine(p1 , p2, &glcdInvertPixel);

while (true){
    background();
}

    return 0;
}