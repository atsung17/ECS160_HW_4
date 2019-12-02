#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_LIMIT 19999
#define NUM_TOP_ELEMENTS 10
#define LINE_SIZE 1024

int isQuoted = 0;

// Input: char* line: the character string in the current row
//      int num: index of the column number
//      int nameIndex: used to check if the current row has the same number of sections as the header
// Output: extracts name column from the current row
char* getname(char* line, int nameIndex, int numSections)
{
        int curIndex = 0;
        char* tok;
        char* retString = (char*)malloc(sizeof(char) * LINE_SIZE);

        // check if same number of sections as header
        int tempCount = 0;
        char* temp = strdup(line);
        while(temp[0] != '\0' && temp[0] != '\n'){
                if(temp[0] == ','){
                        tempCount++;
                }
                temp++;
        }
        if(tempCount != numSections){
                printf("Invalid Input Format\n");
                exit(0);
        }

        // return the proper name
        if(line == NULL){
                return NULL;
        }

        // get to proper column
        char* itr = strdup(line);
        while(curIndex < nameIndex){
                if(itr[0] == ','){
                        curIndex++;
                }
                itr++;
        }

        // at first element of name
        if(itr[0] == '\0' || itr[0] == ',' || itr[0] == '\n'){
                return "";
        }
        curIndex = 0;

        while(itr[0] != '\0' && itr[0] != ',' && itr[0] != '\n'){
                retString[curIndex] = itr[0];
                curIndex++;
                itr++;
        }

        // if there was nothing in the line
        return retString;
}

//Input: char* header: character array of the header row
//      int numSections: used to determine which algorithm to use depending if there are 1 or more than 1 columns
int getnameindex(char* header, int numSections)
{

        // Algorithm for if there is 1 column in the csv
        if(numSections == 0){

                // check if the header is name
                if ((strcmp(header, "\"name\"\n") == 0) || (strcmp(header, "name\n") == 0)){
                        if(strcmp(header, "\"name\"\n") == 0){
                                isQuoted = 1;
                        }
                }
                else{
                        printf("Invalid Input Format\n");
                        exit(0);
                }
                return 0;
        }

        // Otherwise there is more than 1 column
        int curIndex = -1;

        // tracks number of columns w/ "name" as column header
        int nameCount = 0;
        int nameIndex = 0;

        const char* tok;
        const char* nameType;
        char* copy = strdup(header);

        // make sure there is only one name column
        for (tok = strtok(header, ","); ; tok = strtok(NULL, ",\n"))
        {
                if(tok != NULL){
                        curIndex++;
                        // if column header is any form of name
                        if ((strcmp(tok, "\"name\"") == 0) || (strcmp(tok, "\"name\"\n") == 0) || (strcmp(tok, "name") == 0) || (strcmp(tok, "name\n") == 0)){
                                if((strcmp(tok, "\"name\"") == 0) || (strcmp(tok, "\"name\"\n") == 0)){
                                        isQuoted = 1;
                                }

                                // if multiple "name" columns
                                if(nameCount >= 1){
                                        printf("Invalid Input Format\n");
                                        exit(0);
                                }
                                nameType = tok;
                                nameCount++;
                        }
                }
                else {
                        break;
                }
        }

        if(nameType == NULL){
                return -1;
        }

        while(copy[0] != '\0' && copy[0] != '\n'){
                if(copy[0] == ','){
                        nameIndex++;
                }
                else if(strncmp(copy, nameType, strlen(nameType)) == 0){
                        break;
                }
                copy++;
        }

        return nameIndex;
}

// Output: returns index of last column (ie if there are 5 total rows, will return 4)
int getnumSections(char* header)
{
        // gets number of sections based on number of commas
        int index = 0;
        while(header[0] != '\0' && header[0] != '\n'){
                if(header[0] == ','){
                        index++;
                }
                header++;
        }
        return index;
}

int main(int argc, char** argv)
{
        printf("Input file: %s\n", argv[1]);
        FILE* stream = fopen(argv[1], "r");

        char line[1024];

        // test header
        fgets(line, 1024, stream);

        char* tmp = strdup(line);
        int numSections = getnumSections(tmp);
        char* tmp2 = strdup(line);
        int nameIndex = getnameindex(tmp2, numSections);

        if(nameIndex == -1){
                printf("Invalid Input Format\n");
                exit(0);
        }

        //make parallel name and count arrays, which act as our mapping of tweeters to their number of tweets
        const char* names[FILE_LIMIT];
        int count[FILE_LIMIT];
        int index;

        //initialize counts as -1, which marks that there is no tweeter associated with it
        for(index = 0; index < FILE_LIMIT; index++){
                count[index] = -1;
        }

        // while loop parses starting at first tweet
        while (fgets(line, 1024, stream))
        {
                char* row = strdup(line);
                char* out = getname(row, nameIndex, numSections);
                if(out == NULL){
                        printf("Invalid Input Format\n");
                        exit(0);
                }

                float length = strlen(out);

                // if name is supposed to be within quotes
                if(isQuoted){
                        // if the tweet doesn't start and end in quotes
                        if(length < 2.0f || !(out[0] == '\"' && out[(int)length -1] == '\"')){
                                printf("Invalid Input Format\n");
                                exit(0);
                        }
                        // otherwise get rid of the quotation marks
                        out++;
                        out[(int)length - 2] = '\0';
                }

                // if tweet isn't supposed to be in quotes but starts or ends in them
                else if(length > 0.0f && (out[0] == '\"' || out[(int)length - 1] == '\"')){
                        printf("Invalid Input Format\n");
                        exit(0);
                }

                // potentially remove trailing newline
                if(strlen(out) > 0 && out[strlen(out) - 1] == '\n'){
                        out[strlen(out) - 1] = '\0';
                }

                // account for the current name using the name/count arrays
                for(index = 0; index < FILE_LIMIT; index++){
                        // if names match, increment its count array
                        if(count[index] != -1){
                                if(strcmp(names[index], out) == 0){
                                        count[index] = count[index] + 1;
                                        break;
                                }
                        }

                        // otherwise add it as a new row in the name/count arrays
                        else{
                                names[index] = strdup(out);
                                count[index] = 1;
                                break;
                        }
                }
        }
        // end parsing

        // start of finding top 10 elements
        int top_indices[NUM_TOP_ELEMENTS];
        int top_count[NUM_TOP_ELEMENTS];
        int min_index  = -1;
        int min_count = 0;
        int num_added = 0;

        // initialize top_count
        for(index = 0; index < NUM_TOP_ELEMENTS; index++){
                top_count[index] = -1;
        }

        // find top 10
        for(index = 0; index < FILE_LIMIT; index++){
                // if we reached the end of the list of names
                if(count[index] == -1){
                        break;
                }

                // otherwise potentially add to top_count
                // if haven't added enough to top elements yet
                if(num_added < NUM_TOP_ELEMENTS){
                        // add to next available in top_list
                        for(int j = 0; j < NUM_TOP_ELEMENTS; j++){
                                if(top_count[j] == -1){
                                        top_count[j] = count[index];
                                        top_indices[j] = index;
                                        if(min_count > count[index]){
                                                min_count = count[index];
                                                min_index = index;
                                        }
                                        num_added++;
                                        break;
                                }
                        }
                }

                // if we already added enough elements, need to check if min of the maximums is replaced
                else if(count[index] > min_count){
                        // replace min
                        top_count[min_index] = count[index];
                        top_indices[min_index] = index;

                        // get new min
                        min_count = count[index];
                        min_index = index;
                        for(int j = 0; j < NUM_TOP_ELEMENTS; j++){
                                if(min_count > top_count[j]){
                                        min_count = top_count[j];
                                        min_index = j;
                                }
                        }
                }
        }
        // done finding top 10

        // print top 10 in order
        if(num_added > 0){
                int accounted_for[NUM_TOP_ELEMENTS];
                int nextMax = top_count[0];
                int nextIndex = 0;
                for(int i = 0; i < num_added; i++){
                        // get first unaccounted for element
                        for(int j = 0; j < num_added; j++){
                                if(!accounted_for[j]){
                                        nextMax = top_count[j];
                                        nextIndex = j;
                                        break;
                                }
                        }
                        // find max out of remaining unaccounted for elements
                        for(int j = nextIndex; j < num_added; j++){
                                if(!accounted_for[j] && top_count[j] > nextMax){
                                        nextMax = top_count[j];
                                        nextIndex = j;
                                }
                        }

                        // mark the index as accounted for before printing it
                        accounted_for[nextIndex] = 1;
                        printf("%s: %i\n", names[top_indices[nextIndex]], count[top_indices[nextIndex]]);
                }
        }
}
