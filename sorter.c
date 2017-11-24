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

int* shared;
int titleCompiled;
int sortedColumnNum;
int numberOfRows;

row titleRow;
row *data;

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
							return;
						}

						titleRow.sortedColumnNum = sortedColumnNum;

						int length = strlen(titleRow.fields[sortedColumnNum - 1]);
						if (titleRow.fields[sortedColumnNum - 1][length - 1] == '\n') {
							titleRow.fields[sortedColumnNum - 1][length - 2] = '\0';
						}

						if (sortedColumnNum != 28) {
							printf("Sorry, the CSV input is not valid.\n");
							printf("Sorting on process %d has failed due to invalid input.\n",
									getpid());

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
							printf("got here\n");
							printf("%s\n", dirPath);

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

void exportToFile(char* selectedColumn, char* fileName, char* outputDir, int depth,
		 int dUsed, int oUsed){

	printf("this ran0\n");

	//Export to a new file

		FILE *fp2;
		char* filename2;

	char* new_str;
	char* tempString = fileName;
	//	tempString[strlen(tempString) - 4] = 0;

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
			printf("this ran\n");
			fp2 = fopen(fileName, "w+");

		} else {
			char* fullPath = malloc(
					strlen(outputDir) + strlen(new_str) + (strlen("../") * depth)
							+ 2);
			strcpy(fullPath, "");

			strcat(fullPath, outputDir);
			//printf("%s\n", outputDir);
			strcat(fullPath, new_str);

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

void beginSort(char* selectedColumn, char* fileName, char* outputDir, int depth,
		char* ogFileName, int dUsed, int oUsed) {

	//printf("%s\n", "Begin sort called");

	FILE* fp;

		fp = fopen(fileName, "r");


		//wait(3);

	// non title rows, aka all the other ones
	row regularRow;
	regularRow.rowValue = (char*) malloc(sizeof(char) * 1000);
	int currentRow = numberOfRows + 1;

	while (fgets(regularRow.rowValue, 999, fp) != NULL) {
		regularRow.rowLength = strlen(regularRow.rowValue);
		regularRow.fields = (char**) malloc(
				sizeof(char *) * (sortedColumnNum + 1));
		regularRow.fields = customStrTok(regularRow.rowValue, sortedColumnNum);
		//printf("Data 0: %s\n", regularRow.rowValue);
		data[currentRow++] = regularRow;
		//printf("Row Value: %d and Data: %s\n", currentRow -1, data[currentRow-1].rowValue);

	}

	numberOfRows = currentRow;
	printf("num of rows: %d\n", numberOfRows);
	//printf("size: %d\n", sizeof(data[0]));

	return;

}

void traverseDirectory(char* dirName, char* selectedColumn, char* outputDir,
		int depth, int dUsed, int oUsed) {

	//printf("Searching through directory %s\n\n", dirName);
	DIR *dir;
	DIR *dir2;
	struct dirent *ent;
	if ((dir = opendir(dirName)) != NULL) {
		//printf("Curretly in dir %s\n", dirName);
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {

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

				printf("Found CSV: %s\n", itemName);

				//pid = fork();
//--------------------Create thread instead


					(*shared)++;
					temp = malloc(strlen(dirName) + strlen(itemName) + 4);
					strcat(temp, dirName);
					strcat(temp, itemName);
					//int pidt = getpid();
//--------------------Print thread ID instead
					//printf("%d,", 0);
					fflush(stdout);
					/*printf(
							"New sorting process created with Process ID %d for file %s\n\n",
							getpid(), temp);*/
					printf("begin sort called\n");

					beginSort(selectedColumn, temp, outputDir, depth, itemName, dUsed, oUsed);


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
					dirPath[0] = '\0';
					//strcat(dirPath, "./");
					strcat(dirPath, dirName);
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

						if (strstr(dirPath, outputDir) != NULL) {
							sleep(1);
						}

						//pid2 = fork();
						pid2 = 0;
//--------------------Create thread2 instead
						switch (pid2) {
						case 0:
							(*shared)++;
							//int pidt = getpid();
							printf("%d,", 0);
//--------------------Print thread id instead
							fflush(stdout);
							/*printf(
									"New directory traversal process created with Process ID %d\n\n",
									getpid());*/
							//wait();

							traverseDirectory(dirPath, selectedColumn,
									outputDir, depth + 1, dUsed, oUsed);
							exit(0);
							//return;

						case -1:
							perror("Error creating fork");
							break;

						default:
							//(*sortProcessCountPtr)++;
							break;
						}

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

int main(int argc, char* argv[]) {
	titleCompiled = 0;
	shared = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	numberOfRows = -1;

	char* fileName;

	//globla struct to store rows
	data = (row*) malloc(sizeof(row) * 30000); //size matters

	// column to sort by
	char* selectedColumn;
	//int initialPid = getpid();
	int depth = 0;

	if (strcmp(argv[1], "-c") != 0) {
		printf("Sorry, you must use the -c flag to declare a column\n");
		exit(2);
	}

	if (argc != 3 && argc != 5 && argc != 7) {
		printf("Invalid argument size\n");
		return 1;
	}

	//printf("Initial PID: %d\n", initialPid);
	//printf("PIDs of all child processes: ");
	//fflush(stdout);

	selectedColumn = argv[2];

	if (argc == 3) {

		//wait(NULL);


		populateStructTitles("./", selectedColumn);

		int columnToSort = 0;

		while (columnToSort < titleRow.sortedColumnNum) {
			if (strcmp(titleRow.fields[columnToSort], selectedColumn) == 0) {
				break;
			}
			columnToSort++;
		}

		traverseDirectory("./", selectedColumn, "-n", depth, 0, 0);




		printf("callig merge\n");
		printf("columntoSort: %d\n", columnToSort);
		printf("num rows: %d\n", numberOfRows);
		mergeSort(data, columnToSort, numberOfRows);
		printf("made it here2\n");


		//AllFiles-sorted-<fieldname>.csv

		fileName = "sortedFiles.csv";
		printf("made it here3\n");
		exportToFile(selectedColumn, fileName, "-n", depth,
				 0, 0);



		/*traverseDirectory("./", selectedColumn, "-n", depth, 0, 0);
		if (initialPid == getpid()) {
			wait();
			printf("\nTotal number of processes: %d\n", *shared + 1);
		}
		exit(0);*/
	}

	if (argc == 5) {

		if (strcmp(argv[3], "-d") == 0) {
			//printf("Start from new directory: %s \n", argv[4]);

			char* startingPath = malloc(
					strlen(argv[4] + strlen("./") + strlen("/") + 2));

			if (argv[4][0] == '/') {

				strcpy(startingPath, ".");
				strcat(startingPath, argv[4]);
				strcat(startingPath, "/");
				//printf("slash at beginning: %s\n", startingPath);
			} else {

				strcpy(startingPath, argv[4]);
				strcat(startingPath, "/");
			}

			traverseDirectory(startingPath, selectedColumn, "-n", depth, 0, 0);
			/*if (initialPid == getpid()) {
				wait();
				printf("\nTotal number of processes: %d\n", *shared + 1);
			}*/
			exit(0);

		} else if (strcmp(argv[3], "-o") == 0) {
			//printf("Output to this directory: %s\n", argv[4]);

			char* outputDir = malloc(
					strlen(argv[4] + strlen("./") + strlen("/") + 2));

			if (argv[4][0] == '/') {

				strcpy(outputDir, ".");
				strcat(outputDir, argv[4]);
				strcat(outputDir, "/");
				//printf("slash at beginning: %s\n", startingPath);
			} else {

				strcpy(outputDir, argv[4]);
				strcat(outputDir, "/");
			}

			if (strlen(argv[4]) == 0) {
				printf("error");
				return 0;
			}

			traverseDirectory("./", selectedColumn, outputDir, depth, 0, 1);
			/*if (initialPid == getpid()) {
				wait();
				printf("\nTotal number of processes: %d\n", *shared + 1);
			}*/
			exit(0);

			//exit(2);
		} else {
			printf("Wrong input\n");
			return 0;
		}

	}

	if (argc == 7) {

		if ((strcmp(argv[3], "-d") == 0) && (strcmp(argv[5], "-o") == 0)) {
			/*printf(
					"Start from new directory: %s and export to this directory: %s \n",
					argv[4], argv[6]);*/

			char* startingPath = malloc(
					strlen(argv[4] + strlen("./") + strlen("/") + 2));

			if (argv[4][0] == '/') {

				strcpy(startingPath, ".");
				strcat(startingPath, argv[4]);
				strcat(startingPath, "/");
				//printf("slash at beginning: %s\n", startingPath);
			} else {

				strcpy(startingPath, argv[4]);
				strcat(startingPath, "/");
			}



			char* outputDir = malloc(
					strlen(argv[6] + strlen("./") + strlen("/") + 2));

			if (argv[6][0] == '/') {

				strcpy(outputDir, ".");
				strcat(outputDir, argv[4]);
				strcat(outputDir, "/");
				//printf("slash at beginning: %s\n", startingPath);
			} else {

				strcpy(outputDir, argv[6]);
				strcat(outputDir, "/");
			}

			if (strlen(argv[6]) == 0) {
				printf("error");
								return 0;
			}

			//printf("starting path: %s\n", startingPath);
			//printf("output dir: %s\n", outputDir);

			traverseDirectory(startingPath, selectedColumn, outputDir, depth, 1, 1);
		/*	if (initialPid == getpid()) {
				wait();
				printf("\nTotal number of processes: %d\n", *shared + 1);
			}*/
			exit(0);

		} else {
			printf("Wrong input\n");
			return 0;
		}

	}

	return 0;
}
