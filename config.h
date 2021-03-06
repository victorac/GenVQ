/* 
 * File:   config.h
 * Author: victor
 *
 * Created on January 12, 2016, 3:04 PM
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    //CHECK THESE
    //const char* OCTAVE_PATH = "/home/victor/Dropbox/ssh";
    #define ROOT_DIR "/home/victor/Documents/Mestrado/Projeto/datasets/LapsBM1.4"
    #define CORPUS_NAME "Laps-LPCC"
    #define REF_DIR "/home/victor/Documents/Mestrado/Projeto/março2016/refDir"
    #define LIST_DIR REF_DIR "/" CORPUS_NAME
    #define REF_FILE LIST_DIR "/trecRefGenVq.txt"
    #define VQ_DIR LIST_DIR "/cb"
    
    
    #define MFC_MAKE_CONFIG_PATH "/home/victor/Dropbox/Projeto2016/ssh/cfg/config0"
    const char* VQ_MAKE_CONFIG_PATH = "/home/victor/Dropbox/Projeto2016/ssh/cfg/vqMakeConfig";
    const int step = 1;
    
    
#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

