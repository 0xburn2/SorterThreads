/*
 * CS214 Assignment 2
 * Peter Lambe and Umar Rabbani
 * Fall 2017
 */

#include "Sorter.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

int* shared;
int titleCompiled;
int sortedColumnNum;
int numberOfRows;
int* threadCount;
int firstTraverseCall;

int ret;
int success;
pthread_mutexattr_t mattr;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

row titleRow;
row *data;

struct arg_struct {
	char* fileName;
};

struct dirArg_struct {
	char* dirName;
	char* fileName;
	char* selectedColumn;
	char* outputDir;
	int depth;
	int dUsed;
	int oUsed;
	pthread_t* threads;
};

int isString(char* string) {

	int i = 0;
	int decimal = 0;

	// empty string
	if (string == NULL || string == "") {
		return 1;
	}

	// goes character by character to check if its a number or string
	while (i < strlen(string)) {

		if (!isdigit(string[i])) {
			if (string[i] == '.' && !decimal) {
				if (i == strlen(string) - 1 || !isdigit(string[i + 1])) {
					return 1;
				}
				decimal = 1;
				i++;
				continue;
			}
			return 1;
		}
		i++;
	}
	return 0;
}

// trims trailing and leading blank spaces in a string
char* removeWhitespace(char *string, int i) {

	char *final;

	while (isspace((unsigned char) *string)) {
		string++;
	}

	if (*string == 0) {
		return string;
	}
	final = string + i;
	while (final > string && isspace((unsigned char) *final)) {
		final--;
	}
	*(final + 1) = 0;
	return string;
}

// splits row by commas and places them into the structs
char** customStrTok(char* line, int sortedColumnNum) {

	int i = 0;
	int j = 0;
	int k = 0;

	// stores resulting fields
	char** result = (char**) malloc(sizeof(char*) * (sortedColumnNum + 1));

	char* container = (char*) malloc(500);

	// checks for quotation marks in string
	int boolIsQuote = 0;

	//go through each character
	while (i < strlen(line)) {

		if (line[i] == '"' && boolIsQuote == 0) {
			boolIsQuote = 1;
		}

		else if (line[i] == '"' && boolIsQuote == 1) {
			//store value in result
			result[k] = (char*) malloc((j + 1) * sizeof(char));
			container = removeWhitespace(container, j - 1);
			strcpy(result[k], container);
			memset(&container[0], 0, strlen(container));
			boolIsQuote = 0;
			j = 0;
			k++;
			i++;
		}

		//splits row by columns
		else if ((line[i] == ',' || i == strlen(line) - 1)
				&& boolIsQuote != 1) {
			//if there is no character; (eg: ,,)
			if (!container) {
				container[0] = '\0';
			}
			if (i == strlen(line) - 1 && line[i] != '\n') {
				container[j] = line[i];
				j++;
			}
			// copy into result array
			result[k] = (char*) malloc((j + 1) * sizeof(char));
			container = removeWhitespace(container, j - 1);

			strcpy(result[k], container);

			memset(&container[0], 0, strlen(container));

			j = 0;
			k++;

			// if comma is at the end
			if (line[i] == ',' && i == strlen(line) - 2) {

				container[0] = '\0';

				result[k] = (char*) malloc((j + 1) * sizeof(char));

				strcpy(result[k], container);
				memset(&container[0], 0, strlen(container));
			}

		} else {

			//copy into container
			if (j == 0) {
				if (line[i] == ' ') {
					i++;
					continue;
				}
			}
			container[j] = line[i];
			j++;
		}
		i++;
	}
	i = 0;

	return result;
}

void populateStructTitles(char* dirName, char* selectedColumn){

	//printf("Searching through directory %s\n\n", dirName);
		DIR *dir;
		DIR *dir2;
		struct dirent *ent;
		if ((dir = opendir(dirName)) != NULL) {
			//printf("Curretly in dir %s\n", dirName);
			/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != NULL) {

				if (titleCompiled == 1){
					return;
				}
				//wait(getppid());
				char* itemName = ent->d_name;
				//printf("%s\n", itemName);
				int length = strlen(itemName);
				int pid, pid2;
				char* temp;

				//CSV FILE FOUND
				if (length > 5 && itemName[length - 1] == 'v'
						&& itemName[length - 2] == 's'
						&& itemName[length - 3] == 'c'
						&& itemName[length - 4] == '.'
						&& strstr(itemName, "sorted") == NULL) {


					/*printf("Found filename: %s\n", itemName);


					printf("Making item path\n");
					printf("dirName: %s\n", dirName);
					printf("itemName: %s\n", itemName);*/
					char* itemPath = malloc(
													strlen(dirName) + strlen(itemName) + strlen("/")
															+ 20);
					itemPath[0] = '\0';
											//strcat(dirPath, "./");
											strcat(itemPath, dirName);

											//strcat(dirPath, "/");
											strcat(itemPath, itemName);

											//strcat(itemPath, "/");


					FILE* fp;

						fp = fopen(itemPath, "r");



						sortedColumnNum = 1;
						char *token;

						//DO everything from here up until line

						// sets up the row of column titles

						titleRow.rowValue = (char*) malloc(sizeof(char) * 1000);

						fgets(titleRow.rowValue, 999, fp);

						titleRow.rowLength = strlen(titleRow.rowValue);
						titleRow.fields = (char**) malloc(sizeof(char *) * titleRow.rowLength);

						token = strtok(titleRow.rowValue, ",");
						titleRow.fields[0] = token;

						//Beginning splitting the tokens and check if the column name entered exists
						int selectedColumnExist = 0;

						while ((token = strtok(NULL, ","))) {
							//printf("Field name: %s\n", token);
							titleRow.fields[sortedColumnNum] = token;

							//This removes the last whitespace value (\n) because for the last column in the CSV, the check would fail
							titleRow.fields[sortedColumnNum] = removeWhitespace(
									titleRow.fields[sortedColumnNum],
									strlen(titleRow.fields[sortedColumnNum]) - 1);

							if (strcmp(titleRow.fields[sortedColumnNum], selectedColumn) == 0) {
								//the column exists
								selectedColumnExist = 1;

							}

							sortedColumnNum++;
						}

						if (selectedColumnExist != 1) {
							printf("Sorry, the column you entered doesn't exist in the csv\n");
							success = 0;
							exit(0);
							//return;
						}

						titleRow.sortedColumnNum = sortedColumnNum;

						int length = strlen(titleRow.fields[sortedColumnNum - 1]);
						if (titleRow.fields[sortedColumnNum - 1][length - 1] == '\n') {
							titleRow.fields[sortedColumnNum - 1][length - 2] = '\0';
						}

						if (sortedColumnNum != 28) {
							printf("Sorry, the CSV input is not valid.\n");
							//printf("Sorting on process %d has failed due to invalid input.\n",
									//getpid());

							//kill(getpid(), SIGTERM);
							return;
						}

						// trim column titles
						int i = 0;
						while (i < sortedColumnNum) {
							titleRow.fields[i] = removeWhitespace(titleRow.fields[i],
									strlen(titleRow.fields[i]) - 1);
							i++;
						}



						titleCompiled = 1;

				} else {
					int i;
					int directory = 1;

					for (i = 0; i < length; i++) {

						if (itemName[i] == '.') {
							directory = 0;
						}

					}

					if (directory == 1) {

						//printf("Potential directory found!\n");

						//(ent = readdir (dir)) != NULL)

						//fix this
						char* dirPath = malloc(
								strlen(dirName) + strlen(itemName) + strlen("/")
										+ 2);
						//strcat(dirPath, "./");
						strcat(dirPath, "./");
						//strcat(dirPath, "/");
						strcat(dirPath, itemName);
						strcat(dirPath, "/");

						if ((dir2 = opendir(dirPath)) == NULL) {

							if (ENOENT == errno) {
								closedir(dir2);
								continue;
							}

						} else {
							//printf("got here\n");
							//printf("%s\n", dirPath);

							populateStructTitles(dirPath, selectedColumn);

						}

					} else {
						continue;
					}

				}

			}

			closedir(dir);
			return;
		} else {
			/* could not open directory */
			perror("");
			return;
		}


}

void exportToFile(char* selectedColumn, char* dirName, char* fileName, char* outputDir, int depth,
		 int dUsed, int oUsed){



	//Export to a new file

		FILE *fp2;
		char* filename2;

	char* new_str;
	char* tempString = fileName;
		//tempString[strlen(tempString) - 4] = 0;

		/*char* tempString2 = ogFileName;
		tempString2[strlen(tempString2) - 4] = 0;*/

		/*if ((new_str = malloc(strlen(tempString) + strlen(selectedColumn) + 13))
				!= NULL) {
			new_str[0] = '\0';

			if (depth > 0 || dUsed == 1) {
				if (oUsed == 0){
					strcat(new_str, fileName);
				} else {
								//strcat(new_str, ogFileName);
				}

			} else {
				strcat(new_str, fileName);
			}

			strcat(new_str, "-sorted-");
			strcat(new_str, selectedColumn);
			strcat(new_str, ".csv");
		} else {
			printf("%s\n", "malloc failed");
			// exit?
		}*/

		if ((strcmp(outputDir, "-n")) == 0) {

			char* fullPath = malloc(
								strlen(dirName) + strlen(fileName) + (strlen("../") * depth)
										+ 2);
						strcpy(fullPath, "");

						strcat(fullPath, dirName);
						//printf("%s\n", outputDir);
						strcat(fullPath, fileName);

						//printf("Full output path: %s\n", fullPath);
						//printf("outputdir: %s\n", outputDir);
						//printf("newstr: %s\n", new_str);
						//printf("OGstr: %s\n", ogFileName);

						fp2 = fopen(fullPath, "w+");

		} else {
			char* fullPath = malloc(
					strlen(outputDir) + strlen(fileName) + (strlen("../") * depth)
							+ 2);
			strcpy(fullPath, "");

			strcat(fullPath, outputDir);
			//printf("%s\n", outputDir);
			strcat(fullPath, fileName);

			//printf("Full output path: %s\n", fullPath);
			//printf("outputdir: %s\n", outputDir);
			//printf("newstr: %s\n", new_str);
			//printf("OGstr: %s\n", ogFileName);

			fp2 = fopen(fullPath, "w+");

		}

		int vv, zz;
		vv = 0;

		while (vv < sortedColumnNum) {

			fprintf(fp2, titleRow.fields[vv]);

			if (vv != sortedColumnNum - 1) {
				fprintf(fp2, ",");
			} else {
				fprintf(fp2, "\n");
			}

			vv++;
		}

		vv = 0;
		zz = 0;

		while (vv < numberOfRows) {

			while (zz < sortedColumnNum) {

				fprintf(fp2, data[vv].fields[zz]);

				if (zz != sortedColumnNum - 1) {
					fprintf(fp2, ",");
				} else {
					fprintf(fp2, "\n");
				}

				zz++;
			}
			vv++;
			zz = 0;
		}

		fclose(fp2);

}

void* beginSort(void *args) {

	pthread_mutex_lock(&m);
	numberOfRows--;
	struct arg_struct *arg = args;

	printf("%ld,",(unsigned long int)pthread_self());


	//printf("%s\n", arg->fileName);

	//printf("%s\n", "Begin sort called");

	FILE* fp;

		fp = fopen(arg->fileName, "r");




		//wait(3);

	// non title rows, aka all the other ones
	row regularRow;
	regularRow.rowValue = (char*) malloc(sizeof(char) * 1000);

	//printf("number of rows: %d\n", numberOfRows);
	int currentRow = numberOfRows + 1;



	fgets(regularRow.rowValue, 999, fp);


	while (fgets(regularRow.rowValue, 999, fp) != NULL) {

		regularRow.rowLength = strlen(regularRow.rowValue);
		regularRow.fields = (char**) malloc(
				sizeof(char *) * (sortedColumnNum + 1));
		regularRow.fields = customStrTok(regularRow.rowValue, sortedColumnNum);
		//printf("Data 0: %s\n", regularRow.rowValue);
		//printf("made it here: %s\n", regularRow.fields[currentRow]);

		//printf("%s: %s\n", arg->fileName, regularRow.rowValue);
		//printf("%d\n", currentRow);
		data[currentRow++] = regularRow;
		//count++;
		//printf("%d\n", count);
		//printf("Row Value: %d and Data: %s\n", currentRow -1, data[currentRow-1].rowValue);

	}

	numberOfRows = currentRow;
	//printf("Number of Rows: %d + Title Row\n", numberOfRows);
	//printf("size: %d\n", sizeof(data[0]));
	pthread_mutex_unlock(&m);
	return NULL;

}

void* traverseDirectory(void* args) {

	struct dirArg_struct *arg = args;

	if (firstTraverseCall != 0){
		printf("%ld,",(unsigned long int)pthread_self());
	}

	firstTraverseCall = 1;

	//printf("Searching through directory %s\n\n", dirName);
	DIR *dir;
	DIR *dir2;
	struct dirent *ent;
	if ((dir = opendir(arg->dirName)) != NULL) {
		//printf("Curretly in dir %s\n", dirName);
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {

			char* itemName = ent->d_name;
			int length = strlen(itemName);
			char* temp;

			//CSV FILE FOUND
			if (length > 5 && itemName[length - 1] == 'v'
					&& itemName[length - 2] == 's'
					&& itemName[length - 3] == 'c'
					&& itemName[length - 4] == '.'
					&& strstr(itemName, "sorted") == NULL) {

				//printf("Found CSV: %s\n", itemName);
					temp = malloc(strlen(arg->dirName) + strlen(itemName) + 4);
					strcat(temp, arg->dirName);
					strcat(temp, itemName);

					fflush(stdout);

					//printf("Calling begin sort and creating thread\n");

					 pthread_t thr;
					 struct arg_struct *args = malloc(sizeof(struct arg_struct));

					 args->fileName = temp;

					 pthread_create (&thr, NULL, beginSort, (void *)args);
					  ++(*threadCount);
					      arg->threads[*threadCount] = thr;


			} else {
				int i;
				int directory = 1;

				for (i = 0; i < length; i++) {

					if (itemName[i] == '.') {
						directory = 0;
					}

				}

				if (directory == 1) {

					//printf("Directory found!\n\n\n");

					//(ent = readdir (dir)) != NULL)

					//fix this
					char* dirPath = malloc(
							strlen(arg->dirName) + strlen(itemName) + strlen("/")
									+ 2);
					dirPath[0] = '\0';
					//strcat(dirPath, "./");
					strcat(dirPath, arg->dirName);
					//strcat(dirPath, "/");
					strcat(dirPath, itemName);
					strcat(dirPath, "/");

					if ((dir2 = opendir(dirPath)) == NULL) {

						if (ENOENT == errno) {
							closedir(dir2);
							continue;
						}

					} else {

						//printf("This is the full dirpath being passed: %s\n", dirPath);
						/*printf("\nDirectory found: %s in process: %d in dir %s\n",
								dirPath, getpid(), dirName);*/

						if (strstr(dirPath, arg->outputDir) != NULL) {
							sleep(1);
						}


						//printf("Calling begin sort and creating thread\n");

						pthread_t dirThr;
						struct dirArg_struct *dirArgs = malloc(sizeof(struct dirArg_struct));

						dirArgs->dirName = dirPath;
						dirArgs->selectedColumn = arg->selectedColumn;
						dirArgs->outputDir = arg->outputDir;
						dirArgs->depth = arg->depth;
						dirArgs->dUsed = arg->dUsed;
						dirArgs->oUsed = arg->oUsed;
						dirArgs->threads = arg->threads;

						pthread_create (&dirThr, NULL, traverseDirectory, (void *)dirArgs);
						++(*threadCount);
						arg->threads[*threadCount] = dirThr;

					}

				} else {
					continue;
				}

			}

		}

		closedir(dir);
		return NULL;
	} else {
		/* could not open directory */
		perror("");
		return NULL;
	}

}

int main(int argc, char* argv[]) {

	firstTraverseCall = 0;
	ret = 0;
	ret = pthread_mutex_init(&m, &mattr);
	success = -1;

	titleCompiled = 0;
	threadCount = (int *)malloc(sizeof(int));
	shared = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	numberOfRows = 0;

	char* fileName;

	  *threadCount = -1;
	  pthread_t * threads = (pthread_t *)malloc(sizeof(pthread_t) * 2048);

	//globla struct to store rows
	data = (row*) malloc(sizeof(row) * 40000); //size matters

	// column to sort by
	char* selectedColumn;
	//int initialPid = getpid();
	int depth = 0;



	if (argc != 3 && argc != 5 && argc != 7) {
		printf("Invalid argument size\n");
		return 1;
	}



	selectedColumn = "noColumnSelected";

	//Set C flag
	if (strcmp(argv[1], "-c") == 0){
		selectedColumn = argv[2];
	} if (argc > 3){
		if (strcmp(argv[3], "-c") == 0){
		selectedColumn = argv[4];
		}
	} if (argc > 5){
		if (strcmp(argv[5], "-c") == 0){
		selectedColumn = argv[6];
		}
	}

	if (strcmp(selectedColumn, "noColumnSelected") == 0){
		printf("Sorry, the -c flag is mandatory. Please use the -c flag to declare a column to sort\n");
		return -1;
	}


	 printf("Initial TID: %ld\n", (unsigned long int)pthread_self());
		 printf("TIDS of all child threads: ");
		  fflush(stdout);


	struct dirArg_struct *dirArgs1 = malloc(sizeof(struct dirArg_struct));

				dirArgs1->dirName = "./";
				dirArgs1->selectedColumn = selectedColumn;
				dirArgs1->outputDir = "-n";
				dirArgs1->depth = depth;
				dirArgs1->dUsed = 0;
				dirArgs1->oUsed = 0;
				dirArgs1->threads = threads;

	//Set O Flag

	if (strcmp(argv[1], "-o") == 0){

		dirArgs1->oUsed = 1;
		char* outputDir = malloc(
							strlen(argv[4] + strlen("./") + strlen("/") + 2));

					if (argv[2][0] == '/') {

						strcpy(outputDir, ".");
						strcat(outputDir, argv[2]);
						strcat(outputDir, "/");

						dirArgs1->outputDir = outputDir;
						//printf("slash at beginning: %s\n", startingPath);
					} else {

						strcpy(outputDir, argv[2]);
						strcat(outputDir, "/");
						dirArgs1->outputDir = outputDir;
					}

					if (strlen(argv[2]) == 0) {
						printf("error");
						return 0;
					}

	} else if (argc > 3){
		if (strcmp(argv[3], "-o") == 0){
		dirArgs1->oUsed = 1;
		char* outputDir = malloc(
							strlen(argv[4] + strlen("./") + strlen("/") + 2));

					if (argv[4][0] == '/') {

						strcpy(outputDir, ".");
						strcat(outputDir, argv[4]);
						strcat(outputDir, "/");
						dirArgs1->outputDir = outputDir;
						//printf("slash at beginning: %s\n", startingPath);
					} else {

						strcpy(outputDir, argv[4]);
						strcat(outputDir, "/");
						dirArgs1->outputDir = outputDir;
					}

					if (strlen(argv[4]) == 0) {
						printf("error");
						return 0;
					}
		}
	} if (argc > 5){
		//printf("it was greater\n");
		if ((strcmp(argv[5], "-o") == 0)){
		dirArgs1->oUsed = 1;
		char* outputDir = malloc(
							strlen(argv[6] + strlen("./") + strlen("/") + 2));

					if (argv[6][0] == '/') {

						strcpy(outputDir, ".");
						strcat(outputDir, argv[6]);
						strcat(outputDir, "/");
						dirArgs1->outputDir = outputDir;
						//printf("slash at beginning: %s\n", startingPath);
					} else {

						strcpy(outputDir, argv[6]);
						strcat(outputDir, "/");
						dirArgs1->outputDir = outputDir;
					}

					if (strlen(argv[6]) == 0) {
						printf("error");
						return 0;
					}
		}
	}

	//Set D Flag

	if (strcmp(argv[1], "-d") == 0){
		dirArgs1->dUsed = 1;
		char* startingPath = malloc(
							strlen(argv[2] + strlen("./") + strlen("/") + 2));

					if (argv[2][0] == '/') {

						strcpy(startingPath, ".");
						strcat(startingPath, argv[2]);
						strcat(startingPath, "/");
						dirArgs1->dirName = startingPath;
						//printf("slash at beginning: %s\n", startingPath);
					} else {

						strcpy(startingPath, argv[2]);
						strcat(startingPath, "/");
						dirArgs1->dirName = startingPath;
					}

	} if (argc > 3){
		if ((strcmp(argv[3], "-d") == 0)){
		dirArgs1->dUsed = 1;
		char* startingPath = malloc(
							strlen(argv[4] + strlen("./") + strlen("/") + 2));

					if (argv[4][0] == '/') {

						strcpy(startingPath, ".");
						strcat(startingPath, argv[4]);
						strcat(startingPath, "/");
						dirArgs1->dirName = startingPath;
						//printf("slash at beginning: %s\n", startingPath);
					} else {

						strcpy(startingPath, argv[4]);
						strcat(startingPath, "/");
						dirArgs1->dirName = startingPath;
					}
		}
	} if (argc > 5){
		if ((strcmp(argv[5], "-d") == 0 )){
		dirArgs1->dUsed = 1;
		char* startingPath = malloc(
							strlen(argv[6] + strlen("./") + strlen("/") + 2));

					if (argv[4][0] == '/') {

						strcpy(startingPath, ".");
						strcat(startingPath, argv[6]);
						strcat(startingPath, "/");
						dirArgs1->dirName = startingPath;
						//printf("slash at beginning: %s\n", startingPath);
					} else {

						strcpy(startingPath, argv[6]);
						strcat(startingPath, "/");
						dirArgs1->dirName = startingPath;
					}
	}
	}


		populateStructTitles(dirArgs1->dirName, selectedColumn);
		int columnToSort = 0;
		while (columnToSort < titleRow.sortedColumnNum) {
			if (strcmp(titleRow.fields[columnToSort], selectedColumn) == 0) {
				break;
			}
			columnToSort++;
		}

		/*printf("dirArgs dUsed: %d\n", dirArgs1->dUsed);
		printf("dirArgs oUsed: %d\n", dirArgs1->oUsed);
		printf("dirArgs depth: %d\n", dirArgs1->depth);
		printf("dirArgs dirName: %s\n", dirArgs1->dirName);
		printf("dirArgs fileName: %s\n", dirArgs1->fileName);
		printf("dirArgs outputDir: %s\n", dirArgs1->outputDir);
		printf("dirArgs selectedCoulmn: %s\n", dirArgs1->selectedColumn);*/

		traverseDirectory(dirArgs1);

		 int counter = *threadCount;
		    while(counter >= 0){
		    	int* ret;

		      pthread_join(threads[counter--], (void *)&ret);

		    }


		mergeSort(data, columnToSort, numberOfRows);


		char* outputName = malloc(20 + strlen(dirArgs1->selectedColumn) + 10);
		outputName[0] = '\0';
		strcat(outputName, "AllFiles-sorted-");
		strcat(outputName, dirArgs1->selectedColumn);
		strcat(outputName, ".csv");

		//AllFiles-sorted-<fieldname>.csv
		//fileName = "sortedFiles.csv";

		if (success != 0){

			/*printf("dirArgs dirName: %s\n", dirArgs1->dirName);
					printf("dirArgs fileName: %s\n", outputName);
					printf("dirArgs outputDir: %s\n", dirArgs1->outputDir);
					printf("dirArgs oUsed: %d\n", dirArgs1->oUsed);
					printf("dirArgs selectedCoulmn: %s\n", dirArgs1->selectedColumn);*/




		exportToFile(dirArgs1->selectedColumn, dirArgs1->dirName, outputName, dirArgs1->outputDir, dirArgs1->depth,
				 dirArgs1->dUsed, dirArgs1->oUsed);
		}

		printf("\n\nTotal number of threads: %d\n", *threadCount + 1);

	return 0;
}
