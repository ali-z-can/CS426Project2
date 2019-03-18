#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main(int argc,char **argv) {

    int docSize,numberOfLines;
    docSize = atoi(argv[2]);

    //array of arrays to hold input file line by line
    int **array;
    //information given in util.c
    array = (int**)calloc(0, sizeof(int*));
    /*
     * Debug
     * fprintf(stderr,"array addres = %d\n",array);
     */

    numberOfLines = readDoc(argv[1],array,docSize);
    int q,j;
    /*
     * Debug
     * fprintf(stderr,"here numberOfLines = %d\n",numberOfLines);
     */

    /*
     * Debug check if array initialized correctly
    for(j = 0; j<numberOfLines;j++){
        for (q = 0; q < docSize+1; q++) {
            fprintf(stderr,"arr[%d][%d] = %d \n",numberOfLines,q,array[j][q]);
        }
    }
     */
}