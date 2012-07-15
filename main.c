#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>


// SWITCHES
unsigned int MEMORY_PERCENTAGE_USED = 10;
//


struct matrix_item
{
  unsigned int row;
  unsigned int column;
  double value;
};

struct matrix_item * matrix_items=0;
unsigned int current_matrix_item=0;
unsigned int max_matrix_item=0;

long timeval_diff ( struct timeval *difference, struct timeval *end_time, struct timeval *start_time )
{

   struct timeval temp_diff;

   if(difference==0) { difference=&temp_diff; }

  difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
  difference->tv_usec=end_time->tv_usec-start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while(difference->tv_usec<0)
  {
    difference->tv_usec+=1000000;
    difference->tv_sec -=1;
  }

  return 1000000LL*difference->tv_sec+ difference->tv_usec;

}

void clear_line()
{
            fputs("\033[A\033[2K\033[A\033[2K",stdout);
            rewind(stdout);
            ftruncate(1,0);
}


void print_matrix(FILE * output_file,double * matrix_items,unsigned int rows_dimension,unsigned int columns_dimension)
{
    unsigned int row=0, column=0;

     for ( column = 0; column < columns_dimension; column++)
     {
       for ( row = 0; row < rows_dimension; row++)
       {
         fprintf(output_file,"%f ",matrix_items[column*columns_dimension + row]);
        }
        fprintf(output_file,"\n");
      }
}

inline void print_sparse_coordinate(FILE * output_file_rows,FILE * output_file_columns,FILE * output_file_values,
                                    unsigned int row,          unsigned int column,           char * value)
{
   fprintf(output_file_rows,"%u\n",row);
   fprintf(output_file_columns,"%u\n",column);

   //%s has a newline included
   fprintf(output_file_values,"%s",value);
}


inline void add_sparse_coordinate(unsigned int row,unsigned int column,double value)
{
   if (current_matrix_item<max_matrix_item)
    {
        matrix_items[current_matrix_item].row=row;
        matrix_items[current_matrix_item].column=column;
        matrix_items[current_matrix_item].value=value;
        ++current_matrix_item;
    } else
    {
        fprintf(stderr,"Out Of Memory!\n");
    }
}


inline void swap(struct matrix_item *a, struct matrix_item *b)
{
  struct matrix_item t=*a; *a=*b; *b=t;
}

void sort(struct matrix_item *arr,unsigned int beg,unsigned int end)
{
  if (end > beg + 1)
  {
    int piv = arr[beg].row , piv2 = arr[beg].column, l = beg + 1, r = end;
    while (l < r)
    {
      if (arr[l].row <= piv)
       {
         if ((arr[l].row == piv)&&(arr[l].column>piv2))
          {
             swap(&arr[l], &arr[--r]);
          } else
          {
              l++;
          }
       }
      else
        swap(&arr[l], &arr[--r]);
    }
    swap(&arr[--l], &arr[beg]);
    sort(arr, beg, l);
    sort(arr, r, end);
  }
}



void sort_sparse_coordinates()
{
   sort(matrix_items,0,current_matrix_item);
}



void write_sparse_coordinates(FILE * output_file_rows,FILE * output_file_columns,FILE * output_file_values)
{
   unsigned int i=0;
   fprintf(stderr,"Writing Rows \n");
   for ( i=0; i<current_matrix_item; i++ )
    {
      fprintf(output_file_rows,"%u\n",matrix_items[i].row);
    }

   fprintf(stderr,"Writing Columns \n");
   for ( i=0; i<current_matrix_item; i++ )
    {
      fprintf(output_file_columns,"%u\n",matrix_items[i].column);
    }

   fprintf(stderr,"Writing Values \n");
   for ( i=0; i<current_matrix_item; i++ )
    {
      fprintf(output_file_values,"%f\n",matrix_items[i].value);
    }
}

int main(int argc, char** argv)
{
    printf("Matrix Type Conversion !\n");

    //elevate io ops
    pid_t pid;
    if ((pid = getpid()) < 0) { /*Could not get PID */ } else
                              { char command[129]={0};
                                sprintf(command,"sudo ionice -c 1 -n 0 -p %u ",pid);
                                int i=system((char* )command);
                                 if (i==0) { fprintf(stderr,"IO elevation ok \n"); }
                                //printf("%s\n",command);
                              }



    struct timeval starttime,endtime,difference;
    gettimeofday(&starttime,0x0);


    if (argc!=4) {
                    fprintf(stderr,"Wrong Number of arguments provided ( %u ) : \n",argc);
                    fprintf(stderr,"Syntax : MatrixTypeConversion InputIndexes InputData Output");
                    return 1;
                 }

    FILE *indexes_file=0; //KIx30.txt SkylineIndexes.txt
    indexes_file = fopen(argv[1],"r");
    FILE *data_file=0; // KData30b.txt SkylineData.txt
    data_file = fopen(argv[2],"r");
    FILE *output_file_rows=0;
    FILE *output_file_columns=0;
    FILE *output_file_values=0;



    char output_file_string[100]={0};
    strcpy(output_file_string,argv[3]);
    strcat(output_file_string,"Rows");
    output_file_rows = fopen(output_file_string,"w");


    strcpy(output_file_string,argv[3]);
    strcat(output_file_string,"Column");
    output_file_columns = fopen(output_file_string,"w");


    strcpy(output_file_string,argv[3]);
    strcat(output_file_string,"Values");
    output_file_values = fopen(output_file_string,"w");


    if (indexes_file==0)
    { fprintf(stderr,"Error Opening Index file \n"); return 1; }
    if (data_file==0)
    { fprintf(stderr,"Error Opening Data file \n"); return 1; }
    if ( (output_file_rows==0)||(output_file_columns==0)||(output_file_values==0) )
    { fprintf(stderr,"Error Opening Output file \n"); return 1; }


     unsigned int matrix_dimension=0;
     char number_str[100]={0};
     unsigned int number_val,last_number_val,rows_to_fill,column_number,row_number;
     double matrix_val;

     //GET MATRIX DIMENSIONS
     while (fgets(number_str, 100 , indexes_file) != 0)
       {
           number_val=atoi(number_str);
           ++matrix_dimension;
       }
     --matrix_dimension;
     fprintf(stderr,"Matrix has a size of %ux%u \n",matrix_dimension,matrix_dimension);
     fprintf(stderr,"We will need a maximum of %u elements \n",2*(number_val-matrix_dimension)+matrix_dimension);
     rewind (indexes_file);


     // ALLOCATE MATRIX
     unsigned int memory_limit = matrix_dimension * matrix_dimension * sizeof(struct matrix_item);
     fprintf(stderr,"According to heuristic , the worst case is needing %u MB of memory \n",memory_limit/1048576);

     memory_limit =(unsigned int) ( memory_limit *((float) MEMORY_PERCENTAGE_USED/100) );
     max_matrix_item = matrix_dimension * matrix_dimension*((float)MEMORY_PERCENTAGE_USED/100);

     if ( matrix_dimension < 1000 )
      {
            max_matrix_item = matrix_dimension*matrix_dimension;
      }

     fprintf(stderr,"Using a %u %% estimation for %u filled elements we need %u MB of memory \n",MEMORY_PERCENTAGE_USED,max_matrix_item,memory_limit/1048576);

     matrix_items = calloc(max_matrix_item,sizeof(struct matrix_item) );
     current_matrix_item=0;
     if ( matrix_items == 0 )
       {
            fprintf(stderr,"Error allocating %u MB of memory \n",memory_limit/1048576);
            return 1;
       }  else
       {
            fprintf(stderr,"Successfully allocated %u MB of memory \n",memory_limit/1048576);
       }


     fprintf(stderr,"\n\n\n");
     column_number=0;
     row_number=0;

     unsigned int i=0;
     if ( fgets(number_str, 100 , indexes_file) == 0 ) { fprintf(stderr,"Error reading index file \n"); }
     last_number_val=atoi(number_str);
     while (fgets(number_str, 100 , indexes_file) != 0)
       {
        number_val=atoi(number_str);
        rows_to_fill=number_val-last_number_val;
        last_number_val=number_val;

        //fprintf(stderr,"line %u = %u = %s \n",i,number_val,number_str);

            row_number=column_number;
            i=0;
            while (i<rows_to_fill)
            {
              if ( fgets(number_str, 100 , data_file) == 0 )
                {
                    fprintf(stderr,"Error out of data input..! \n");
                    break;
                }

              matrix_val=atof(number_str);

              if ( (matrix_dimension<=column_number) || (matrix_dimension<=row_number) )
               {
                   fprintf(stderr,"Error @ file pair , accessing[%u,%u] \n",column_number,row_number);
               } else
               {
                // fprintf(stderr,"Element %u,%u = %f \n",row_number,column_number,matrix_val);
                // matrix_items[column_number*matrix_dimension + row_number] = matrix_val;
                if ( matrix_val!=0 )
                 {
                   add_sparse_coordinate(row_number,column_number,matrix_val);
                 }

              if ( row_number!=column_number )
               {
                 // fprintf(stderr,"Element %u,%u = %f \n",column_number,row_number,matrix_val);
                 // matrix_items[row_number*matrix_dimension + column_number] = matrix_val;
                  if ( matrix_val!=0  )
                  {
                    add_sparse_coordinate(column_number,row_number,matrix_val);
                  }
               }

               }


              --row_number;
              ++i;
            }

         if ((column_number>3000) && (column_number%10000==0))
          {
            clear_line();
            fprintf(stdout,"Matrix Type Conversion running...!\n");
            fprintf(stdout,"%u %% \n",column_number*100/matrix_dimension);
          }

         ++column_number;
       }


      fprintf(stderr,"Sorting Output.. \n");
      sort_sparse_coordinates();
      write_sparse_coordinates(output_file_rows,output_file_columns,output_file_values);

      fprintf(stderr,"100% Done.. \n");
      gettimeofday(&endtime,0x0);
      fprintf(stderr,"Matrix Type Conversion  time : %u microseconds \n",timeval_diff(&difference,&endtime,&starttime));



      // CHILLOUT
      free(matrix_items);

      fclose(indexes_file);
      fclose(data_file);

      fflush(output_file_rows);
      fflush(output_file_columns);
      fflush(output_file_values);

      fclose(output_file_rows);
      fclose(output_file_columns);
      fclose(output_file_values);



    return 0;
}
