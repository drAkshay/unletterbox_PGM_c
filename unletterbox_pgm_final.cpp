/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** <unletterbox_pgm>.cpp
** This is supposed to read a PGM file (P5 format), detect and crop the left 
** and right black bars in the image. 
**
** USAGE: (ideally all in the same folder)
** g++ -o unletterbox_pgm_final.o unletterbox_pgm_final.cpp
** ./unletterbox_pgm_final.o 17.pgm
**
** Author: <Dr. Akshay P 08/04/2015>
** -------------------------------------------------------------------------*/

/* 
** ASSUMPTIONS:
** 
** pillar bars are always black 
** 8-bit depth supported only. Checks included.
** Support only P5 version of PGM. Checks included.
** currently doesn't automatically handle comments in between header starting with #. 
**     basic check included for comments above, but needs to be manually turned on.
** -------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <iostream>
#include <map>  
using namespace std;

/* this function creates a frequency map for the various possible start points of the actual image
(after the first pillar bar), per row and returns the cumulative max freq start point as the final start point */

int frequencyStartCnt(int start){
static map<int , int> frequency_count; // to keep value between function calls
int num=0;
int max=0;
frequency_count[start]++; 
for(map<int,int>:: iterator it = frequency_count.begin(); it != frequency_count.end(); it++)
{
    if (it->second>num){
        max=it->first;
        num=it->second;
    }
}
return max;
}

/* this function creates a frequency map for the various possible end points of the actual image
(after the last pillar bar), per row and returns the cumulative max freq start point as the final end point */

int frequencyEndCnt(int end, int start, int cols){
static map<int , int> frequencyend_count; // to keep value between function calls
int num=0;
int max=0;
frequencyend_count[end]++;
for(map<int,int>:: iterator it = frequencyend_count.begin(); it != frequencyend_count.end(); it++)
{
    //cout<<it->first<<" "<<it->second<<endl;
    if (it->second>num && it->first>cols-start){ //also takes into account if the last bar is towards right side of the pic.
        max=it->first;
        num=it->second;
    }
}
return max;
}

int main(int argc, char* argv[]) //input the file name in the .o file, ideally in the same folder as .cpp
{
    char line[LINE_MAX];
    int cols, rows, maxgray;
    int cnt; //state var to help with edge detection
    int i, j;
    int tmp;
    int start,end;
    FILE *fd;
    int flg=0; //state var to help with corrupted header parsing

    fd = fopen(argv[1], "rb"); //use 'rb' mode if using to read binary files in windows specifically for newlines, if not r/rb is similar in unix.
    if (fd == NULL) {
        printf("Cannot open the PGM input file provided. Check \n");
        exit(EXIT_FAILURE);
    }

    // version of the pgm file input. we currently only support PGM with P5 version, we do not support P2 which have RGB values
    fgets(line, LINE_MAX, fd);
    printf("version header is %s", line);
    if (strcmp(line, "P5\n") != 0) { //to deal with corrupted headers and not P5 headers
        const char *p = strtok (line," ");
        while (p != NULL)
        {
            //printf ("p is .%s.\n",p);
            if ((strcmp(p, "P5") != 0) && flg==0) { //not P5
                printf("image is not in PGM format!\n");
                fclose(fd);
                exit(EXIT_FAILURE);    
            } else if ((strcmp(p, "P5") == 0) && flg==0) { // P5 corrupted header, where header is like P5 640 480 255
                printf ("Found P5 in corrupted header\n");
                flg=1;
                p = strtok (NULL, " ");
            } else if ((strcmp(p, "P5") != 0) && flg==1) { //headers separated by whitespaces and not newlines
                cols=atoi(p);
                //printf ("cols is %d\n",cols);
                flg=2;
                p = strtok (NULL, " ");
            } else if ((strcmp(p, "P5") != 0) && flg==2) {
                rows=atoi(p);
                //printf ("rows is %d\n",rows);
                flg=3;
                p = strtok (NULL, " ");
            } else if ((strcmp(p, "P5") != 0) && flg==3) {
                maxgray=atoi(p);
                //printf ("maxgray is %d\n",maxgray);
                flg=0;
                p = strtok (NULL, " ");
            } 
            else {
                printf("image header not supported, currently!\n");
                fclose(fd);
                exit(EXIT_FAILURE);
            }
        }
    }
    while (!rows || !cols || !maxgray) { // if not corrupted header 

    // to skip comments, if present in the input file after P5. 
    // Current provided input files do not have comments.
    // In future, a function/check can be added which checks for '#' here and handles it accordingly. 
    
    //fgets(line, sizeof(line), fd);
    

    // to read the cols, rows and max bit depth value for gray scale
    if (fscanf(fd, "%d %d %d\n", &cols, &rows, &maxgray) != 3) {
        printf("Corrupted header\n");
        fclose(fd);
        exit(EXIT_FAILURE);
    }
    if (maxgray > 255) {
        printf("Currently only 8-bit depth PGM are supported!\n");
        fclose(fd);
        exit(EXIT_FAILURE);
    }
    }
    printf("Columns: %d\nRows: %d\nMaxGrayVal: %d\n", cols, rows, maxgray);
    
    int array[rows][cols]; // let's create an array to read and append the values from the input PGM file. 
    
    //we populate array here with greyscale pixel values and look for changes in the value from 0 (black).
    //false positives likely, hence a freq. distribution is maintained and mapped. 
    if (maxgray < 256) {
        for (i = 0; i < rows; ++i){
            cnt=0; // markers to keep track of change in gray scale value from 0 to anything else
            for (j = 0; j < cols; ++j) {
                tmp = fgetc(fd);
                array[i][j] = tmp;
                //printf("Pixel value for [%d][%d] is: %d \n",i,j,array[i][i]);
                // some if-else conditions present only for improving readability
                if (cnt==0 && array[i][j]==0){
                    cnt=0;
                }else if (cnt==0 && array[i][j]!=0){ // first change from black to other
                    cnt=1;
                    start=frequencyStartCnt(j);
                    //printf("start is [%d][%d] \n",i,j);
                }else if (cnt==1 && array[i][j]!=0){
                    cnt=1;
                }else if (cnt==1 && array[i][j]==0){
                    cnt=2; //next change from other to black 
                    end=frequencyEndCnt(j,start,cols);
                    //printf("end is [%d][%d] \n",i,j);
                }else if (cnt==2 && array[i][j]==0){
                    cnt=2;
                }
            }
           } 
        }
    //printf("final start is [%d] \n",start);
    //printf("final end is [%d] \n",end);
    fclose(fd);
    
    //by the end of this part, start & end have start & end locations of original image (minus pillars)
    
    FILE *od;
    
    //writing to the output file
    od = fopen("out.pgm", "wb");
    if (od == NULL) {
        printf("Cannot open image file\n");
        exit(EXIT_FAILURE);
    }
    
    //to handle blackbars and no blackbars images. 
    int newcols; 
    if ((start==end) || (start>end) || (end>cols) || (start<0)){ //to counter when the image has no black letter bars 
    newcols=cols;
    start=0;
    end=cols;
    printf("No black letterboxes detected in the picture. Picture dimensions remain same!\n");
    } else {
    newcols = end-start;
    printf("Black letterboxes detected in the picture and removed!\n");
    printf("New image has following dimensions: \n");
    printf("Columns: %d\nRows: %d\nMaxGrayVal: %d\n", newcols, rows, maxgray);
    }
    
    fprintf(od, "P5 ");
    fprintf(od, "%d %d ", newcols, rows); //image size headers specified here
    fprintf(od, "%d ", maxgray);
 
    if (maxgray < 256) {
        for (i = 0; i < rows; ++i){ // height is same 
            for (j = start; j < end; ++j) { //cols is now end-start
                tmp = ((array[i][j]));
                fputc(tmp, od);
            }
        }    
    }
    fclose(od);
}


