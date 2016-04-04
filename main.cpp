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

int runHQuantv2(char* scriptFileName);
int parseMFCCv2(char* scriptFilePath, char* mfcDir);
int parseVqTablev2();
int parseMFCC2v2(char* mfcDirPath);
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
    
    
    
    sprintf(command, "%s/mfc", LIST_DIR);
    runHQuantv2(scriptPath);
    parseMFCCv2(scriptPath, command);
    parseMFCC2v2(command);
}

int runHQuantv2(char* scriptFileName){
    
    char command[300],buffer[200];
    char* aux;
    char trainFile[100], vqTableFile[100];
    FILE* scriptFile;
    int index;
    char number[2];
    char prefix[4]; 
    
    scriptFile = fopen(scriptFileName, "r");
    if(scriptFile==NULL){
        perror("Não conseguiu abrir o arquivo de script temporario");
        return -8;
    }
    index=0;
    
    sprintf(command, "mkdir %s/cb", LIST_DIR);
    system(command);
    
    while(fgets(buffer,sizeof(buffer), scriptFile)!=NULL){
        index++;
        aux=strrchr(buffer, ' ');
        strcpy(trainFile, aux+1);
        trainFile[strlen(trainFile)-1]=0;
        aux=strrchr(trainFile, '/');
        sprintf(vqTableFile, "%s/cb%s", LIST_DIR, aux);
        int len = strlen(vqTableFile);
        vqTableFile[len-4]=0;
        strcat(vqTableFile, "_cb");
        //printf("Name: %s\n", vqTableFile);
        sprintf(command,"HQuant -C %s -n 1 20 %s %s ",VQ_MAKE_CONFIG_PATH, vqTableFile, trainFile);
        printf("Command: %s\n", command);
        int j=system(command);
        printf("%d\n",j);
    }
    
    fclose(scriptFile);
    
    parseVqTablev2();
    
}

int parseVqTablev2(){
    
    FILE* vqfile;
    
    int order=0;
    char filePath[200];
    char line[1000];
    FILE* vqcsv;
    
    char* cluster;
    char csvLine[1000];
    DIR* vqDir;
    struct dirent* ent;
    int len;
    char command[100];

    vqDir = opendir(VQ_DIR);
    if(vqDir == NULL){
        perror("Nao conseguiu abrir o diretorio de VQ");
        return -5;
    }
    sprintf(command, "mkdir %s/csv", VQ_DIR);
    system(command);
    
    while((ent = readdir(vqDir)) != NULL){
       
        if(strlen(ent->d_name)>3){
           
            sprintf(filePath, "%s/%s", VQ_DIR, ent->d_name);
            printf("File: %s\n", filePath);
            vqfile = fopen(filePath, "r");
            sprintf(filePath, "%s/csv/%s.csv", VQ_DIR, ent->d_name);
            vqcsv = fopen(filePath, "w");
            if(vqfile == NULL){
                perror("Não conseguiu abrir a vqtable");
                return -6;
            }
            if(vqcsv ==NULL){
                perror("Não conseguiu criar o arquivo csv");
                return -7;
            }
            order=0;
            while(fgets(line,sizeof(line),vqfile)!=NULL){
                if(order<2){order++;continue;}
                cluster=strtok(line, " ");
                while(cluster!=NULL){
                    strcat(csvLine,cluster);
                    len =strlen(csvLine); 
                    csvLine[len]=',';
                    csvLine[len+1]=0;
                    cluster=strtok(NULL, " ");
                }
                csvLine[strlen(csvLine)-1]=0;
                //printf("Line: %s", csvLine);
                fputs(csvLine, vqcsv);
                csvLine[0]=0;
                order=0;
            }
            
            fclose(vqcsv);
            fclose(vqfile);
        }
    }
    closedir(vqDir);
}

int parseMFCCv2(char* scriptFilePath, char* mfcDir){
    FILE* scriptFile;
    char command[500];
    
    scriptFile = fopen(scriptFilePath, "r");
    if(scriptFile==NULL){
        perror("Não conseguiu abrir o arquivo de script");
        return -4;
    }
    char buffer[200];
    char mfccFileName[60];
    char* aux;
    char* mfccPath;
    sprintf(command,"mkdir %s/text", mfcDir);
    system(command);
    while(fgets(buffer,sizeof(buffer), scriptFile)!=NULL){
        mfccPath=strchr(buffer,' ')+1;
        aux=strrchr(mfccPath,'/')+1;
        strncpy(mfccFileName,aux,strlen(aux)-5);
        mfccFileName[strlen(aux)-5]=0;
        mfccPath[strlen(mfccPath)-1]=0;
        //printf("name: %s", mfccPath);
        sprintf(command,"HList -i 12 %s > %s/text/%s.txt", mfccPath,mfcDir,mfccFileName);
        printf("Command: %s\n", command);
        system(command);
    
    }
    fclose(scriptFile);
    
    
}

/*
 text to csv
 */
int parseMFCC2v2(char* mfcDirPath){
    DIR* mfcDir;
    struct dirent* ent;
    FILE* mfcFile;
    FILE* mfcParsedFile;
    char fParsedPath[160];
    char fpath[160];
    char textPath[160];
    char fname[30];
    char buffer[600];
    char* aux;
    char line[1000];
    char command[100];
    
    
    sprintf(textPath, "%s/text",mfcDirPath);
    mfcDir = opendir(textPath);
    if(mfcDir==NULL){
        perror("Não conseguiu abrir o diretorio");
        return -4;
    }
    sprintf(command, "mkdir %s/csv", mfcDirPath);
    system(command);
    
    while((ent = readdir(mfcDir)) != NULL){
        if(strlen(ent->d_name)>2){
            sprintf(fpath, "%s/%s",textPath, ent->d_name);
            mfcFile = fopen(fpath, "r");
            if(mfcFile==NULL){
                perror("Não conseguiu abrir o arquivo MFCC txt");
                return -6;
            }
            strcpy(fname, ent->d_name);
            strcpy((fname+strlen(fname)-4), ".csv");
            //printf("File name: %s\n",fname);
            sprintf(fParsedPath, "%s/csv/%s",mfcDirPath, fname);
            mfcParsedFile = fopen(fParsedPath, "w");
            if(mfcParsedFile==NULL){
                perror("Não conseguiu criar arquivo csv para MFCC");
                return -7;
            }
            while(fgets(buffer, sizeof(buffer), mfcFile)!=NULL){
                if(isdigit(buffer[0])){
                    aux=strtok(buffer, " ");
                    aux=strtok(NULL, " ");
                    line[0]=0;
                    while(aux!=NULL){    
                        strcat(line, aux);
                        int len = strlen(line);
                        line[len]=',';
                        line[len+1]=0;
                        aux=strtok(NULL, " ");
                    }
                    line[strlen(line)-1]=0;
                    //printf("Line: %s",line);
                    fputs(line,mfcParsedFile);
                }
            }
            fclose(mfcFile);
            fclose(mfcParsedFile);
        }
    }
    closedir(mfcDir);
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
    
    FILE* mfcFileList;
    char** fileList = NULL;
    char command[500];
    int fileCount;
    
    readRoot();
    runHCopy();
    organizer();
    //parseVqTablev2();
    return 0;
}

