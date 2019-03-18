//
// Created by alican on 18.03.2019.
//
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int readDoc(char *docName,int **arr2,int docSize){

    FILE *fp;
    fp = fopen(docName,"r");
    int i,j;
    int *arr;
    i = 0;
    fprintf(stderr,"arr2 addres = %d\n",arr2);

    //moved to main to have it as a return array
    //int **arr2;
    //to use realloc consistently, allocate a memory of size 0 to increment later
    //moved to main to have an actual address to pass
    //arr2 = (int**)calloc(0, sizeof(int*));
    int x;
    char y;

    while(fscanf(fp,"%d",&x) != EOF){

        //initialize arr to read one line of data (id and weight vector)
        arr = (int*)calloc(docSize+1,(sizeof(int)));

        //necessary to be sure that EOF is handled properly
        arr[0] = x;

        // ger rid of the : character between id and weight vector
        fscanf(fp,"%c",&y);
        /*
         * Debug check if integers are getting red properly
         * fprintf(stderr,"y = %c\n",y);
        */
        for(j = 2; j<docSize+2;j++ ){
            fscanf(fp,"%d",&arr[j-1]);

        }

        /*
         * -Debug- check if arr is initialized properly
         * for(j = 0; j<docSize+1;j++ ){
            fprintf(stderr,"read %d",arr[j]);

        }*/

        fprintf(stderr,"\ni = %d\n",i);

        //collect lines (id weight vector pairs) together in a 2D array
        arr2 = (int**) realloc(arr2,(i+1)*(sizeof(int*)));
        arr2[i] = arr;
        //debug
        //fprintf(stderr,"arr = %d arr2[i] = %d \n",arr,arr2[i][0]);
        i++;
    }
    /*
    int q;
    for(j = 0; j<i;j++){
        for (q = 0; q < docSize+1; q++) {
            //fprintf(stderr,"j: %d q: %d\n",j,q);
            fprintf(stderr,"arr[%d][%d] = %d \n",i,q,arr2[j][q]);
        }
    }
     */
    return i;
}