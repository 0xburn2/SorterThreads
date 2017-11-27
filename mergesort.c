/*
 * CS214 Assignment 1
 * Peter Lambe and Umar Rabbani
 * Fall 2017
 */

#include "Sorter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// l is left, r is right, col is indoex of column selected to sort by
void merge(row* L, row* R, row* result, int l, int r, int col){

    int index = 0;
    int i = 0;
    int j = 0;

    // begins merging
    while(i < l && j < r){

    	// 0 if no swap is needed, 1 if swap is needed
    	 int boolVal;

    	 // if it doesn't exist
    	  if(!L[i].fields[col]){
    	    boolVal = 0;
    	  }

    	  if(!R[j].fields[col] && L[i].fields[col]){
    	      boolVal = 1;
    	  }

    	  if(isString(L[i].fields[col]) != 1 && isString(R[j].fields[col]) != 1){

    		  float c1 = atof(L[i].fields[col]);
    		  float c2 = atof(R[j].fields[col]);

    		  if(c1 <= c2){
    			  boolVal = 0;
    	      }
   		      else{
   		          boolVal = 1;
   		      }
    	  } else{
    		  if(strcasecmp(L[i].fields[col],R[j].fields[col]) <= 0){
    			  boolVal = 0;
    		  } else{
    	        boolVal = 1;
    		  }
    	  }

    	  // picks the lesser value to put into result
    	  if (boolVal == 0){
    		  result[index] = L[i];
    		  index++;
    		  i++;
    	  } else{
    		  result[index] = R[j];
    		  index++;
    		  j++;
        }
    }

    // merges the remaining values
    while(i < l){
        result[index] = L[i];
        i++;
        index++;
    }
    while(j < r){
        result[index] = R[j];
        j++;
        index++;
    }
    i = 0;
    while(i < index){
        i++;
    }

}

void mergeSort(row* target, int col, int n){
    int i, mid;


    if(n < 2){
    	return;
    }



    mid = n/2;

    //split array into two parts


    row* L = (row*)malloc(mid * sizeof(row));
    row* R = (row*)malloc((n-mid) * sizeof(row));

    // populate L and R
    for(i = 0; i < mid; i++){
        L[i] = target[i];
    }
    for(i = mid; i < n; i++){
        R[i - mid] = target[i];
    }

    // continue splitting arrays further
    mergeSort(L, col, mid);
    mergeSort(R, col, n - mid);

    // merge all of it back together
    merge(L, R, target, mid, n - mid, col);

    free(L);
    free(R);

}


