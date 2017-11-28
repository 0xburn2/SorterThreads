/*
 * CS214 Assignment 1
 * Peter Lambe and Umar Rabbani
 * Fall 2017
 */

#ifndef SORTER_H
#define SORTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// holds row data
typedef struct {
	// raw row string value
    char* rowValue;
	// length of characters in row
    int rowLength;

    // value at each column
    char** fields;
    // index of column to be sorted by
    int sortedColumnNum;

} row;

// mergesort
void mergeSort(row* target, int col, int n);

// checks if input is string or number
int isString(char* string);

#endif
