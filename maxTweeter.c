#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_LIMIT 19999
#define NUM_TOP_ELEMENTS 10

int isQuoted = 0;

// Input: char* line: the character string in the current row
//      int num: index of the column number
//      int numSections: used to check if the current row has the same number of sections as the header
// Output: extracts name column from the current row
char* getname(char* line, int num, int numSections)
{
        int curIndex = -1;
        char* tok;
        char* retString;

        // account for edge case if row ends in ,
        if(line[strlen(line) - 2] == ','){
                retString = "";
                numSections--;
        }

        // extract each section of the csv
        for (tok = strtok(line, ","); ; tok = strtok(NULL, ",\n"))
        {
                if(tok != NULL){
                        curIndex++;
                }

                // if the number of sections doesn't match the number of header sections
                else if (curIndex != numSections) {
                        return NULL;
                } else{
                        return retString;
                }

                // if at the appropriate column, set the return string
                if (curIndex == num){
                        retString = tok;
                }
        }
        // if there was nothing in the line
        return NULL;
}

//Input: char* header: character array of the header row
//      int numSections: used to determine which algorithm to use depending if there are 1 or more than 1 columns
int getnameindex(char* header, int numSections)
{

        int curIndex = -1;

        // tracks number of columns w/ "name" as column header
        int nameCount = 0;
        int nameIndex = -1;

        // account for edge case if row ends in ,
        if(header[strlen(header) - 2] == ','){
                numSections++;
        }

        const char* tok;
        // Algorithm for if more than 0 columns
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
                                nameIndex = curIndex;
                                nameCount++;
                        }
                }
                else {
                        break;
                }
        }

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
                return curIndex;
        }

        return nameIndex;
}

// Output: returns index of last column (ie if there are 5 total rows, will return 4)
int getnumSections(char* header)
{
        int index = -1;
        const char* tok;

        // accounts for edge case if row ends in ,
        if(header[strlen(header) - 2] == ','){
                index++;
        }

        // increments index for each section in the row
        for (tok = strtok(header, ","); ; tok = strtok(NULL, ",\n"))
        {
                if(tok != NULL){
                        index++;
                }
                else {
                        break;
                }
        }
        return index;
}

int main(int argc, char** argv)
{
        printf("Input file: %s\n", argv[1]);
        FILE* stream = fopen(argv[1], "r");

        char line[1024];
        float lines = 0;
        float lenTotal = 0;

        // test header
        fgets(line, 1024, stream);
        char* tmp = strdup(line);
        int numSections = getnumSections(tmp);
        tmp = strdup(line);
        int nameIndex = getnameindex(tmp, numSections);

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
                char* tmp = strdup(line);
                char* out = getname(tmp, nameIndex, numSections);
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

                // remove trailing newline
                if(out[strlen(out) - 1] == '\n'){
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
