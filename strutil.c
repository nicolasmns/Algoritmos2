#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "strutil.h"
#include <unistd.h>

char* substr(const char *str, size_t n){
  char* str2 = malloc(sizeof(char) * (n+1));
  if(str2 == NULL) return NULL;
  strncpy(str2, str, n);
  str2[n] = '\0';
  return str2;
}

char** split(const char *str, char sep){
    size_t largo = strlen(str);
    size_t cant_sep = 0;

    for(int i = 0; i < largo; i++){
      if(str[i] == sep) cant_sep++;
    }
    char **subcadenas = malloc(sizeof(char *) * (cant_sep + 2));
    if(!subcadenas) return NULL;
    size_t desde = 0;
    size_t hasta = 0;
    size_t pos = 0;
    for(size_t i=0; i < largo+1; i++){
      if(str[i] == sep || str[i] == '\0'){
        hasta = i - 1;
        subcadenas[pos] = substr(str+desde, hasta-desde+1);
        desde = i + 1;
        pos++;
      }
    }
    subcadenas[pos]=NULL;
    return subcadenas;
}

char* join(char **strv, char sep){
  size_t pos = 0;
  size_t cant_letras = 0;
  while(strv[pos]){
    cant_letras += strlen(strv[pos]);
    pos++;
  }
  size_t largo = cant_letras + pos;
  char* str = malloc(sizeof(char) * largo+1);
  size_t posicion_actual = 0;

  for (size_t i= 0; strv[i]; i++){
    if(sep == '\0' && posicion_actual > 0) strcpy(str+posicion_actual - i, strv[i]);
    else strcpy(str+posicion_actual, strv[i]);
    posicion_actual += strlen(strv[i]) + 1;
    if(strv[i+1]) str[posicion_actual - 1] = sep;
  }
  str[largo] = '\0';
  return str;
}

void free_strv(char *strv[]){
  int i = 0;
  while(strv[i]){
    free(strv[i]);
    i++;
  }
  free(strv);
}
