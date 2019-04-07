#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <mpi.h>
#include <math.h>
#include <sys/time.h>

typedef struct mytype_s
{
    int id;
    int val;
} MyType;

void CreateMyTypeMPI(){

    int count;
    count = 2;
    int array_of_blocklengths[] = { 1, 1 };
    MPI_Aint array_of_displacements[] = { 0, sizeof(int)};
    MPI_Datatype array_of_types[] = { MPI_INT, MPI_INT };
    MPI_Datatype my_mpi_type;
    MPI_Type_create_struct(count, array_of_blocklengths, array_of_displacements, array_of_types, &my_mpi_type);
    MPI_Type_commit( &my_mpi_type );

}



void swap(struct mytype_s* a, struct mytype_s* b)
{
    MyType t = *a;
    a->id = b->id;
    a->val = b->val;

    b->val = t.val;
    b->id = t.id;

}

int partition (struct mytype_s arr[], int low, int high)
{
    int pivot = arr[high].val;    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j].val <= pivot)
        {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quickSort(struct mytype_s arr[], int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void compareDocs(MyType *in,MyType *out,int *len,MPI_Datatype *typeptr){
    int forCounter,indexCounter;
    int countera,counterb;
    countera = 0;
    counterb =0;
    indexCounter = 0;
    MyType dummy[*len];

    for (forCounter = 0;  forCounter<*len ; forCounter++) {
        if(in[forCounter].val < out[indexCounter].val){
            dummy[forCounter].val = in[countera].val;
            dummy[forCounter].id = in[countera].id;
            countera++;
        }
        else{
            dummy[forCounter].val = out[counterb].val;
            dummy[forCounter].id = out[counterb].id;
            counterb++;
        }
    }
    for (forCounter = 0;  forCounter<*len ; forCounter++) {
        out[forCounter].val = dummy[forCounter].val;
        out[forCounter].id = dummy[forCounter].id;
    }

}

void CreateMyMpiFunc(){
    MPI_Op MyOp;
    MPI_Op_create((MPI_User_function *) compareDocs,1,&MyOp);
}
void kreduce(int * leastk, int * myids, int * myvals, int k, int world_size, int my_rank);
int main(int argc,char **argv) {

    struct timeval serialbegin,parallelbegin,serialend,parallelend;
    int size,rank;
    MPI_Status s;
    int docSize,numberOfLines;
    int k;
    k = atoi(argv[2]);
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);




    if(rank==0){ //master
        //array of arrays to hold input file line by line
        int **array;
        int *qarr;
        docSize = atoi(argv[1]);

        qarr = (int*)calloc(docSize, sizeof(int));
        //information given in util.c
        //todo i2
        //array = (int**)calloc(0, sizeof(int*));
        /*
         * Debug
         * fprintf(stderr,"array addres = %d\n",array);
         */
        gettimeofday(&serialbegin,NULL);
        int dumy;
        array = readDoc2(argv[3],&numberOfLines,docSize);

        int q,j;
        readQ(argv[4],qarr,docSize);
        /*
         * Debug
         * fprintf(stderr,"here numberOfLines = %d\n",numberOfLines);
         * Debug check if arrays initialized correctly
        for(j = 0; j<numberOfLines;j++){
            for (q = 0; q < docSize+1; q++) {
                fprintf(stderr,"arr[%d][%d] = %d \n",j,q,array[j][q]);
            }
        }

        for (q = 0; q < docSize; q++) {
            fprintf(stderr,"qarr[%d] = %d \n",q,qarr[q]);
        }
          */
        int arraytohopefullysend[numberOfLines*docSize];//myvals
        int arraytohopefullysend2[numberOfLines];//myids
        int scounts[size+1+docSize];

        int rem;
        rem  = numberOfLines % size;
        int sts;
        sts = (numberOfLines-rem) / size;


        for(j=0; j< size; j++){
            scounts[j] = sts;
            if(rem > 0){
                scounts[j]++;
                rem--;
            }


        }
        scounts[size] = docSize;

        for(j=0;j<docSize;j++){
            scounts[size+1+j] = qarr[j];
        }

        /*fprintf(stderr,"scounts size: %d\n%d for \"actual\" purpose\n1 for docSize\n%d for q\n",size+1+docSize,docSize);

        for(j=0;j<docSize;j++){
            fprintf(stderr,"qarr[%d] = %d scounts[%d] = %d\n",j,qarr[j],size+1+j,scounts[size+1+j]);
        }
        fprintf(stderr,"rank: %d  docsize: %d\n",rank,scounts[size]);
        */


        int unnecesarryCounter;
        unnecesarryCounter = 0;
        for(int i = 0; i < numberOfLines; i++){
            for (j = 1;  j<docSize+1 ; j++) {
                arraytohopefullysend[unnecesarryCounter] = array[i][j];
                //fprintf(stderr,"%d tried to added to myvals at %d reult: %d\n",array[i][j],unnecesarryCounter,arraytohopefullysend[unnecesarryCounter]);
                unnecesarryCounter++;
            }
            arraytohopefullysend2[i] = i+1;
        }

        gettimeofday(&serialend,NULL);
        gettimeofday(&parallelbegin,NULL);
        kreduce(scounts,arraytohopefullysend2,arraytohopefullysend,k,size,rank);
        gettimeofday(&parallelend,NULL);

        int sert,part;
        sert = ((serialend.tv_sec - serialbegin.tv_sec)*1000 + (serialend.tv_usec - serialbegin.tv_usec));

        part = ((parallelend.tv_sec - parallelbegin.tv_sec)*1000 + (parallelend.tv_usec - parallelbegin.tv_usec));
        fprintf(stderr,"Sequential part: %ld ms\nParallel part: %ld ms \nTotal time: %ld\n",((serialend.tv_sec - serialbegin.tv_sec)*1000000L + (serialend.tv_usec - serialbegin.tv_usec)),((parallelend.tv_sec - parallelbegin.tv_sec)*1000000L + (parallelend.tv_usec - parallelbegin.tv_usec)),((parallelend.tv_sec - parallelbegin.tv_sec)*1000000L + (parallelend.tv_usec - parallelbegin.tv_usec))+((serialend.tv_sec - serialbegin.tv_sec)*1000000L + (serialend.tv_usec - serialbegin.tv_usec)));
        fprintf(stderr,"Least k = %d ids:\n",k);
        for(int i = 0; i < k;i++){
            fprintf(stderr,"%d\n",scounts[i]);

        }
    }

    else{

        int *scounts;
        int *arraytohopefullysend2,*arraytohopefullysend;
        kreduce(scounts,arraytohopefullysend2,arraytohopefullysend,k,size,rank);

    }


    MPI_Finalize();


}

void kreduce(int * leastk, int * myids, int * myvals, int k, int world_size, int my_rank){


    //-----create a custom mpi datatype to hold both id and val-----------------------------------------------------
    int count;
    count = 2;
    int array_of_blocklengths[] = { 1, 1 };
    MPI_Aint array_of_displacements[] = { 0, sizeof(int)};
    MPI_Datatype array_of_types[] = { MPI_INT, MPI_INT };
    MPI_Datatype my_mpi_type;
    MPI_Type_create_struct(count, array_of_blocklengths, array_of_displacements, array_of_types, &my_mpi_type);
    MPI_Type_commit( &my_mpi_type );
    //---------------------------------------------------------------------------------------------------------------



    //-----create a custom mpi op to compare values while protecting ids---------------------------------------------
    MPI_Op MyOp;
    MPI_Op_create((MPI_User_function *) compareDocs,1,&MyOp);
    //---------------------------------------------------------------------------------------------------------------
;

    int docSize;

    int displs[world_size],j;
    //k is the least number of elements to return
    //leastk is the array contains least k ids (only for root)
    //myids is the id array
    //myvals is the


    if(my_rank == 0){
        docSize = leastk[world_size];
        displs[0] = 0;
        for(j = 1; j<world_size;j++){
            displs[j] = displs[j-1] + leastk[j-1];

        }

        /*
         * debug
         *
         * for(j = 0; j<world_size;j++){
            //fprintf(stderr,"rank: %d  %d leastk: %d \n",my_rank,j, leastk[j]);
            fprintf(stderr,"rank: %d  %d displs: %d \n",my_rank,j, displs[j]);

        }
        for(j = 0; j<world_size+1;j++){
            fprintf(stderr,"rank: %d  %d leastk: %d \n",my_rank,j, leastk[j]);
            //fprintf(stderr,"rank: %d  %d displs: %d \n",my_rank,j, displs[j]);

        }*/
    }


    int toRecieve;

    MPI_Scatter((void*)&leastk[0], 1,MPI_INT, &toRecieve,1,MPI_INT,0,MPI_COMM_WORLD);
    //fprintf(stderr,"%d here torec = %d \n",my_rank,toRecieve);

    int recArr[toRecieve];
    MPI_Scatterv((void*)&myids[0], &leastk[0], &displs[0],MPI_INT,&recArr[0],toRecieve,MPI_INT,0,MPI_COMM_WORLD);

    /*for(j = 0; j<toRecieve;j++){
        fprintf(stderr,"rank: %d  val: %d\n",my_rank,recArr[j]);
    }*/

    //send and recieve D (dictionary size)
    MPI_Bcast((void*)&docSize,1,MPI_INT,0,MPI_COMM_WORLD);

    int que[docSize];
    if(my_rank == 0){

        displs[0] = 0;

        for(j = 0; j<world_size;j++){
            displs[j] = displs[j]*docSize;
            leastk[j] = leastk[j]*docSize;

        }

        for(j=0;j<docSize;j++){
            que[j] =leastk[world_size+1+j];
        }
        /*
        for(j=0;j<docSize;j++) {
            fprintf(stderr,"que[%d] = %d\n",j,que[j]);
        }
         */
    }

    toRecieve = toRecieve*docSize;
    int recArr2[toRecieve];
    //fprintf(stderr,"%d here torec = %d \n",my_rank,toRecieve);
    MPI_Scatterv((void*)&myvals[0], &leastk[0], &displs[0],MPI_INT,&recArr2[0],toRecieve,MPI_INT,0,MPI_COMM_WORLD);

    /*
    for(j = 0; j<toRecieve;j++){
        fprintf(stderr,"rank: %d  val: %d\n",my_rank,recArr2[j]);
    }*/

    MPI_Bcast((void*)&que,docSize,MPI_INT,0,MPI_COMM_WORLD);

    //int calculator[toRecieve/docSize][2];
    int forCounter;

    struct mytype_s docs[toRecieve/docSize];

    int sum;
    sum = 0;
    for(j = 0; j<toRecieve/docSize;j++){
        for(forCounter=0;forCounter<docSize;forCounter++){
            sum += pow(recArr2[(j*docSize)+forCounter],que[forCounter]);
            //fprintf(stderr,"%d ^ %d = %d at rank %d\n",recArr2[(j*docSize)+forCounter],que[forCounter],sum,my_rank);

        }
        docs[j].id = recArr[j];
        docs[j].val = sum;
        //fprintf(stderr,"rank: %d added %d with id %d to %d\n",my_rank,docs[j].val,docs[j].id,j);
        sum = 0;
    }
    /*
    swap(&docs[0],&docs[1]);
    for(j = 0; j<toRecieve/docSize;j++) {
        fprintf(stderr,"rank: %d added %d with id %d to %d\n",my_rank,docs[j].val,docs[j].id,j);

    }
     */

    //-----sort the array with quick sort----
    int n;
    n = sizeof(docs)/ sizeof(docs[0]);
    quickSort(docs, 0, n-1);
    /*for(j = 0; j<toRecieve/docSize;j++) {
        fprintf(stderr,"rank: %d added %d with id %d to %d\n",my_rank,docs[j].val,docs[j].id,j);

    }

    -------------------------------------------------*/

    struct mytype_s getReduced[k];

    for(forCounter = 0; forCounter<k;forCounter++){
        getReduced[forCounter] = docs[forCounter];
    }
    struct mytype_s globalGetReduced[k];
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&getReduced,&globalGetReduced,k,my_mpi_type,MyOp,0,MPI_COMM_WORLD);


    if(my_rank==0)
    {
        for(forCounter=0;forCounter<k;forCounter++){
            //fprintf(stderr,"id: %d val: %d\n",globalGetReduced[forCounter].id,globalGetReduced[forCounter].val);
            //myvals[forCounter] = globalGetReduced[forCounter].val;
            leastk[forCounter] = globalGetReduced[forCounter].id;
        }
    }

}