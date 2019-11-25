#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_LIMIT 19999

int isQuoted = 0;

int getWordCount(const char* text, int length)
{
        int wordCount = 0;
        if(length >= 2) //Due to quotes;
        {
                wordCount += 1;
        }
        int i = 0;
        for(i = 0; i < length; i++){
                if(' ' == text[i])
                {
                        wordCount++;
                }
        }
        return wordCount;
}

char* getname(char* line, int num, int numSections)
{
        int curIndex = -1;
        char* tok;
        char* retString;
        //for (tok = strtok(line, ","); tok && *tok; tok = strtok(NULL, ",\n"))
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
                if (curIndex == num){
                        retString = tok;
                }
        }
        // if there was nothing in the line
        return NULL;
}

int getnameindex(char* header)
{
        int curIndex = -1;

        // tracks number of columns w/ "name" as column header
        int nameCount = 0;
        int nameIndex = -1;

        const char* tok;
        //for (tok = strtok(line, ","); tok && *tok; tok = strtok(NULL, ",\n"))
        for (tok = strtok(header, ","); ; tok = strtok(NULL, ",\n"))
        {
                if(tok != NULL){
                        curIndex++;
                        // if multiple columns are called "name"
                        if ((strcmp(tok, "\"name\"") == 0) || (strcmp(tok, "name") == 0)){
                                if(strcmp(tok, "\"name\"") == 0){
                                        isQuoted = 1;
                                }
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
        return nameIndex;
}

int getnumSections(char* header)
{
        int index = -1;
        const char* tok;
        //for (tok = strtok(line, ","); tok && *tok; tok = strtok(NULL, ",\n"))
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
        int nameIndex = getnameindex(tmp);
        tmp = strdup(line);
        int numSections = getnumSections(tmp);
        free(tmp);
        if(nameIndex == -1){
                printf("Invalid Input Format\n");
                exit(0);
        }

        // while loop starts at first tweet, not header
        while (fgets(line, 1024, stream))
        {
                char* tmp = strdup(line);
                char* out = getname(tmp, nameIndex, numSections);
                if(out == NULL){
                        free(tmp);
                        free(out);
                        printf("Invalid Input Format\n");
                        exit(0);
                }

                float length = strlen(out);

                // if tweet is supposed to be within quotes
                if(isQuoted){
                        // if the tweet doesn't start and end in quotes
                        if(length < 2.0f || !(out[0] == '\"' && out[(int)length -1] == '\"')){
                                free(tmp);
                                free(out);
                                printf("Invalid Input Format\n");
                                exit(0);
                        }
                        // otherwise get rid of the quotation marks
                        out++;
                        out[(int)length - 2] = '\0';
                }

                // if tweet isn't supposed to be in quotes but starts or ends in them
                else if(length > 0.0f && (out[0] == '\"' || out[(int)length - 1] == '\"')){
                        free(out);
                        free(tmp);
                        printf("Invalid Input Format\n");
                        exit(0);
                }

                lines++;
                printf("Text: %s\n", out);
                printf("Length: %f\n", length);

                float wC = getWordCount(out, length);
                printf("Word Count: %f\n", wC);
                int aveCPW = (length - (wC-1))/wC;
                printf("Chars per word: %d\n", aveCPW);
                char* wordTwoGuess = (char *) malloc(aveCPW+1 * sizeof(char));
                memcpy(wordTwoGuess, &out[aveCPW + 1], aveCPW);
                printf("Guess at second word: %s\n", wordTwoGuess);


                if(wC == 2)
                {
                        printf("%c\n", out[2000048]);
                }

                lenTotal += length;
                // NOTE strtok clobbers tmp
                free(tmp);
        }

        printf("Average Tweet Length: %f\n", lenTotal/lines);
}
