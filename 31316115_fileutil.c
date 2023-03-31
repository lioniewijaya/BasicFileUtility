/* 31316115_fileutil.c 
*
* Name:               Lionie Annabella Wijaya
* Student ID:         31316115
* Start date:         17 August 2021
* Last modified date: 26 August 2021
*
* 31316115_fileutil.c program is a multipurpose utility program that provides simple implementation of head, tail, and copy with optional custom number of lines if provided.
* By default functionality is set to view, head, and default number of lines is 10. User can use other functionality with associated flags which are -L (tail), -d (copy), and -n(number of lines).
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>   /* change to <sys/fcntl.h> for System V */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <dirent.h>

/*
 * Function: isNumber
 * --------------------
 *  checks if a string is numeric
 *
 *  s: string to be checked
 *
 *  returns: integer representing boolean value (1: True, 0: false)
 */
int isNumber(char* s);

/*
 * Function: setTail
 * --------------------
 *  set infile to start reading from last n lines instead of default first n lines
 *
 *  infile: file to be read from
 *  lines: number of lines to be read
 *
 *  returns: void
 */
void setTail (int infile, int lines);

/*
 * Function: viewFile
 * --------------------
 *  display n lines from infile to the terminal (lines can start from head or tail, this is set prior to calling this function)
 *
 *  infile: file to be read from
 *  lines: number of lines to be read
 *
 *  returns: void
 */
void viewFile (int infile, int lines);

/*
 * Function: copyFile
 * --------------------
 *  copy n lines from infile to a new copy of file with the same name to infilename in a different directory (lines can start from head or tail, this is set prior to calling this function)
 *
 *  infile: file to be read from
 *  lines: number of lines to be read
 *  outfile: file to be written to
 *
 *  returns: void
 */
void copyFile (int infile, int outfile, int lines);

int main (int argc, char *argv[])
{
    int infile, outfile;
    char* errorMessage;
    DIR* directory;
    char *infilename = "sample.txt";    // Default infile name is "sample.txt" unless user provide source file
    char *outfilename;                  // Default outfile name is NULL unless user select copy functionality
    int lines = 10;                     // Default number of lines is 10 unless user provide number of lines
    int givenFile = 0;                  // Default infile is not a given source file (0: not given, 1: given)
    int tail = 0;                       // Default read or copy functionality takes the head or first (lines) lines (0: head, 1: tail)
    int copy = 0;                       // Default functionality is view file (0: view, 1: copy)

    // Check if user provides source file
    if (argc > 1) {
        if (strcmp(argv[1],"-d") != 0 && strcmp(argv[1],"-n") != 0 && strcmp(argv[1],"-L") != 0) {
            // Set infilename to source file given
            givenFile = 1;
            infilename = argv[1];
        }
    }

    // Open infile
    if ((infile = open((infilename), O_RDONLY)) < 0) { 
        if (givenFile) {
            errorMessage = "Invalid file: failed to open given source file\n";
        }
        else {
            errorMessage = "Invalid file: failed to open sample.txt\n";
        }
        write(2,errorMessage,strlen(errorMessage));
        exit(1);
    }

    // Iterate through commands to process flags and arguments
    for (int i = givenFile+1; i<argc; i++) {
        // Copy file functionality with flag -d
        if (strcmp(argv[i],"-d") == 0) {
            copy = 1;
            // Check for valid argument
            if (i+1 >= argc || strcmp(argv[i+1],"-n") == 0 || strcmp(argv[i+1],"-L") == 0) {
                errorMessage = "Invalid argument: immediately after -d, a directory path was expected\n";
                write(2,errorMessage,strlen(errorMessage));
                exit(1);
            }
            // Check if directory exists  
            if ((directory = opendir(argv[i+1])) == NULL) {        
                errorMessage = "Invalid directory: destination directory does not exist\n";
                write(2,errorMessage,strlen(errorMessage));
                exit(2);
            }
            else {
                closedir(directory);
            }
            // Formatted infilename to include base name only excluding path
            char* baseinfilename = basename(infilename);
            // Set outfilename to include directory path given and infilename to be copied
            outfilename = (char*) malloc (strlen(argv[i+1])+strlen(baseinfilename)+1);
            strcpy(outfilename, argv[i+1]);
            strcat(outfilename,baseinfilename);
            i++;
        }
        // Custom number of lines functionality with flag -n
        else if (strcmp(argv[i],"-n") == 0) { 
            // Check for valid argument
            if (i+1 >= argc || isNumber(argv[i+1]) == 0) {
                errorMessage = "Invalid argument: no line numbers specified after the -n argument\n";
                write(2,errorMessage,strlen(errorMessage));
                exit(1);
            }
            // Set number of lines
            lines = atoi(argv[i+1]);
            i++;
        }
        // Tail functionality with flag -L
        else if (strcmp(argv[i],"-L") == 0) { 
            // Set functionality to start from tail
            tail = 1;
        }     
    }

    // Apply functionality according to provided arguments and flags
    if (tail) {
        setTail(infile,lines);
    }
    if (!copy) {
        viewFile(infile,lines);
    }
    else {
        // Open outfile
        if ((outfile = open((outfilename), O_WRONLY | O_CREAT | O_EXCL, 0663)) < 0) {
            errorMessage = "Cannot copy: destination directory contains a file with a similar name\n";
            free(outfilename);
            write(2,errorMessage,strlen(errorMessage));
            exit(2);
        }
        free(outfilename);
        copyFile(infile,outfile,lines);
        close(outfile);
    }

    close(infile);
    exit(0);
}

int isNumber(char* s) {
    // Loop each character in string to check if it is a digit
    for (int i = 0; s[i]!= '\0'; i++) {
        if (isdigit(s[i]) == 0)
            return 0;
    }
    return 1;
}

void setTail (int infile, int lines) { 
    char buff[1];
    int lines_read = -1;
    int charRead = 0;
    int start = -1; 
    int totalChar;

    // Find total characters inside infile
    off_t begin = lseek(infile,0,SEEK_CUR);
    lseek(infile,0,SEEK_END);
    off_t end = lseek(infile,0,SEEK_CUR);
    totalChar = (end-begin)/sizeof(char);

    // Set infile to be read from end of file
    lseek(infile,start,SEEK_END);

    // Infile is read backwards up to the number of lines given
    while (read(infile,&buff,sizeof(char)) == 1) {
        charRead++;
        if (buff[0] == '\n') {
            lines_read += 1;
            if (lines == lines_read) {
                break;
            }
        }
        start--;
        lseek(infile,start,SEEK_END);
        // When characters read is equal to total characters, start of file is reached and loop exits
        if (charRead == totalChar) {
            break;
        }
    }
}

void viewFile (int infile, int lines) {
    char buff[1];
    int lines_read = 0;

    //Write to terminal character by character until number of lines required or all lines are read
    while (read(infile,&buff,sizeof(char)) != 0) {
        write(1,buff,sizeof(char));
        if (buff[0] == '\n') {
            lines_read += 1;
            if (lines == lines_read) {
                break;
            }
        }
    }
}

void copyFile (int infile, int outfile, int lines) {
    char buff[1];
    int lines_read = 0;

    //Write to outfile character by character until number of lines required or all lines are read
    while (read(infile,&buff,sizeof(char)) != 0) {
        write(outfile,buff,sizeof(char));
        if (buff[0] == '\n') {
            lines_read += 1;
            if (lines == lines_read) {
                break;
            }
        }
    }

    //Indicate copy is successful
    char *successfulErrorMessage = "Copy successful\n";
    write(1,successfulErrorMessage,strlen(successfulErrorMessage));
}