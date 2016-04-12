/* 
 * File:   main.cpp
 * Author: victor
 *
 * Created on January 12, 2016, 3:03 PM
 */

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <bits/time.h>
#include "config.h"

using namespace std;

int runHQuant();
int parseFeatures();
int parseVqTable();
/*
 STEP 1
 */
int readRoot(){
    DIR* rootDir;
    DIR* audioDir;
    struct dirent* ent;
    struct dirent* entChild;
    FILE* listFile;
    char audioDirPath[100];
    char line[200];
    char dirPath[120];
    FILE* refFile;
    int docCount, queryCount=0, base=0;
    
    rootDir=opendir(ROOT_DIR);
    if(rootDir==NULL){
        perror("Não conseguiu abrir o diretorio raiz");
        return -9;
    }
    sprintf(dirPath, "%s/%s.txt", LIST_DIR, CORPUS_NAME);
    listFile=fopen(dirPath, "w");
    if(listFile==NULL){
        printf("Dir path:%s\n",dirPath);
        perror("Não conseguiu criar arquivo de lista");
        return -10;
    }
    refFile = fopen(REF_FILE, "w");
    if(refFile==NULL){
        perror("Não conseguiu criar arquivo de referência");
        return -11;
    }
    while((ent=readdir(rootDir))!=NULL){
        if(strlen(ent->d_name)>2){
            sprintf(audioDirPath, "%s/%s", ROOT_DIR, ent->d_name);
            audioDir=opendir(audioDirPath);
            if(audioDir==NULL){
                perror("Não conseguiu abrir o diretorio filho");
                continue;
            }
            docCount=0;
            while((entChild=readdir(audioDir))!=NULL){
                if(entChild->d_name[strlen(entChild->d_name)-3]=='w' &&
                entChild->d_name[strlen(entChild->d_name)-2]=='a' &&
                entChild->d_name[strlen(entChild->d_name)-1]=='v'){
                    sprintf(line, "%s/%s\n", audioDirPath, entChild->d_name);
                    fputs(line, listFile);
                    docCount++;
                }
            }
            for(int i=0;i<docCount;i++){
                queryCount++;
                for(int j=0;j<docCount;j++){
                    fprintf(refFile, "%d 0 %d 1\n", queryCount, base+j);
                }
            }
            base+=docCount;
            closedir(audioDir);
        }
    }
    closedir(rootDir);
    fclose(listFile);
    fclose(refFile);
}

/*
 STEP 2, 3, 4
 */

int runHCopy(){
    FILE* list;
    FILE* script;
    char line[120], newLine[240];
    char mfcFile[60];
    char* aux;
    char dirPath[120];
    char scriptPath[100];
    char command[500];
    
    sprintf(scriptPath, "%s/%s.scp", LIST_DIR, CORPUS_NAME);
    if(step<=4){
    sprintf(dirPath, "%s/%s.txt", LIST_DIR, CORPUS_NAME);
    list = fopen(dirPath,"r");
    if(list==NULL){
        perror("Não conseguiu abrir o arquivo de lista");
        return -11;
    }
    
    
    script = fopen(scriptPath, "w");
    if(script==NULL){
        perror("Não conseguiu criar arquivo de script");
        return -12;
    }
    /*create script file in the same folder
     format:audioFile mfcName\n
     * make dir mfc
     */
    while(fgets(line, sizeof(line), list)!=NULL){
        line[strlen(line)-1]=0;
        aux=strrchr(line, '/');
        strcpy(mfcFile, aux+1);
        strcpy(mfcFile+(strlen(mfcFile)-3), "mfc");
        sprintf(newLine, "%s %s/mfc/%s\n", line, LIST_DIR,mfcFile);
        //printf("line: %s\n", newLine);
        fputs(newLine, script);
    }
    fclose(script);
    fclose(list);
    
    
    /*make mfc directory*/
    sprintf(command, "mkdir %s/mfc", LIST_DIR);
    int j=system(command);
    printf("%d\n",j);
    
    /*run HCopy using the one script file */
    sprintf(command,"HCopy -A -D -T 1 -C %s -S %s",MFC_MAKE_CONFIG_PATH,scriptPath);
    j=system(command);
    printf("%d\n",j);
    }
    
    runHQuant();
    parseFeatures();
}

int runHQuant(){
    
    char buffer[60], command[300];
    char trainFile[100], vqTableFile[100];
    FILE* listFile;
    
    printf("Running HQuant\n");
    listFile = fopen(LIST_PATH, "r");
    if(listFile==NULL){
        perror("Não conseguiu abrir o arquivo de lista");
        return -8;
    }
    
    sprintf(command, "mkdir %s/cb_%s", REF_PATH, FEATURE);
    system(command);
    
    while(fgets(buffer,sizeof(buffer), listFile)!=NULL){
        buffer[strlen(buffer)-1]=0;
        sprintf(trainFile, "%s/%s.prm", FEAT_PATH,
                buffer);
        sprintf(vqTableFile, "%s/cb_%s/%s_cb", REF_PATH, FEATURE, buffer);
        sprintf(command,"HQuant -n 1 20 %s %s ", vqTableFile, trainFile);
        printf("Command: %s\n", command);
        system(command);
    }
    
    fclose(listFile);
    
    parseVqTable();
    
}

int parseVqTable(){
    
    FILE* listFile;
    FILE* vqfile;
    FILE* vqcsv;   
    char csvLine[1000];
    char command[100];
    char buffer[100];
    char fileName[100];
    char line[1000];
    char* token;
    int len;
    
    printf("Parsing VQ table\n");
    listFile = fopen(LIST_PATH,"r");
    if(listFile == NULL){
        perror("Nao conseguiu abrir o arquivo de lista");
        return -1;
    }
    
    sprintf(command, "mkdir %s/cb_%s/csv", REF_PATH, FEATURE);
    system(command);
    
    while(fgets(buffer, sizeof(buffer), listFile)!=NULL){
        buffer[strlen(buffer)-1]=0;
        sprintf(fileName, "%s/cb_%s/%s_cb", REF_PATH, FEATURE, buffer);
        vqfile=fopen(fileName, "r");
        if(vqfile==NULL){
            perror("Couldn't open the cb file");
            return -2;
        }
        sprintf(fileName, "%s/cb_%s/csv/%s.csv", REF_PATH, FEATURE, buffer);
        vqcsv=fopen(fileName, "w");
        if(vqcsv==NULL){
            perror("Couldn't create csv file");
            return -3;
        }
        csvLine[0]=0;
        while(fgets(line, sizeof(line), vqfile)){
            if(line[0]==' '){
                token=strtok(line, " ");
                while(token!=NULL){
                    strcat(csvLine,token);
                    len =strlen(csvLine); 
                    csvLine[len]=',';
                    csvLine[len+1]=0;
                    token=strtok(NULL, " ");
                }
                csvLine[strlen(csvLine)-1]=0;//cuts the comma
                fputs(csvLine, vqcsv);
                csvLine[0]=0;
            }
        }
        fclose(vqcsv);
        fclose(vqfile);
                
    }
    fclose(listFile);
}
 
int parseFeatures(){
    
    FILE* listFile;
    FILE* textFile;
    FILE* csvFile;
    char command[500];
    char buffer[60];
    char textLine[1000], csvLine[1000];
    char* token;
    
    printf("Parsing features\n");
    listFile = fopen(LIST_PATH, "r");
    if(listFile==NULL){
        perror("Não conseguiu abrir o arquivo de script");
        return -4;
    }
    while(fgets(buffer, sizeof(buffer), listFile)!=NULL){
        buffer[strlen(buffer)-1]=0;
        sprintf(command,"HList -i %i %s/%s.prm > %s/%s.txt", FEAT_COUNT,
                FEAT_PATH, buffer, FEAT_PATH, buffer);
        printf("Command: %s\n", command);
        system(command);
        sprintf(command,"%s/%s.txt", FEAT_PATH, buffer);
        textFile = fopen(command, "r");
        if(textFile==NULL){
            perror("Couldn't open text feat file");
            return -2;
        }
        sprintf(command, "%s/%s.csv", FEAT_PATH, buffer);
        csvFile=fopen(command, "w");
        if(csvFile==NULL){
            perror("Couldn't create csv file");
            return -3;
        }
        fgets(textLine, sizeof(textLine), textFile);//discart first line
        while(fgets(textLine, sizeof(textLine), textFile)!=NULL){
            if(isdigit(textLine[0])){
                token=strtok(textLine, " ");
                token=strtok(NULL, " ");
                csvLine[0]=0;
                while(token!=NULL){
                    strcat(csvLine, token);
                    strcat(csvLine, ",");
                    token=strtok(NULL, " ");
                }
                csvLine[strlen(csvLine)-1]=0;
                fputs(csvLine,csvFile);
            }
        }
        fclose(csvFile);
        fclose(textFile);
        
    }
    fclose(listFile);
    
    
}



int organizer(){
    FILE* listFile;
    FILE* mfccList;
    FILE* cbList;
    char mfcPath[160];
    char cbPath[160];
    char buffer[160];
    char command[320];
    char* aux;
    char** fnames=NULL;
    int r, counter;
    clock_t time;
    
    sprintf(buffer, "%s/%s.txt",LIST_DIR, CORPUS_NAME);
    listFile = fopen(buffer, "r");
    if(listFile==NULL){
        perror("Não conseguiu abrir o arquivo de lista");
        return -15;
    }
    sprintf(buffer, "%s/mfccList.csv", LIST_DIR);
    mfccList = fopen(buffer, "w");
    sprintf(buffer, "%s/cbList.csv", LIST_DIR);
    cbList = fopen(buffer, "w");
    if(mfccList == NULL || cbList == NULL){
        perror("Não conseguiu criar os arquivos de lista");
        return -20;
    }
    while(fgets(buffer, sizeof(buffer), listFile)!=NULL){
        aux=strrchr(buffer, '/');
        aux[strlen(aux)-5]=0;
        sprintf(mfcPath, "%s/mfc/csv/%s.csv\n", LIST_DIR,aux+1);
        sprintf(cbPath, "%s/cb/csv/%s_cb.csv\n", LIST_DIR,aux+1);
        fputs(mfcPath, mfccList);
        fputs(cbPath, cbList);
    }
    fclose(listFile);
    fclose(mfccList);
    fclose(cbList);
    //sprintf(buffer, "%s/distanceMatrix", LIST_DIR);
//    sprintf(command, "octave -q %s/distanceSpeakers.m %s/mfccList.csv %s/cbList.csv %s", 
//            OCTAVE_PATH, LIST_DIR, LIST_DIR, LIST_DIR);
//    r=system(command);
//    
}


/*
 * UPDATE 21/01/2016
 * input = directory containing all speakers folders
 * pre-requisite: folder named after the corpus, specified at config.h
 * Steps:
 *  1-create a file containing all the paths to all the files;
 *  2-create a script file based on the list created on the previous step. This
 * Script is used by HCopy to extract the mfcc of them all;
 *  3-create a directory on the same path as the script and the file containing
 * the list of all audios, to keep the mfcc extracted by HCopy
 *  4- run HCopy with the configuration file specified in config.h and the script
 * file;
 *  5- Browse the script file again to find the audio files' names and one by one
 * run HQuant specifying the size of the codebook (20, universal), the configuration file
 * defined on config.h (universal), the path to the created codebook and the mfc file;
 *  6- Parsing TIME: parse the mfc files and codebook files into csv, readying then for
 * matlab processing;
 *  
 * 
 */
int main(int argc, char** argv) {
    
    runHQuant();
    parseFeatures();
    return 0;
}

