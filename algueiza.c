#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "strutil.h"
#include "abb.h"
#include "hash.h"
#include "lista.h"
#include "heap.h"

#define FIN -1

#define FLIGHT_NUMBER 0
#define DATE 6
#define CANCELLED 9

void func_destruir(void* a_borrar){
  char** borrar = (char**) a_borrar;
  free_strv(borrar);
}

void sacar_fin_linea(char* subcadena){
  size_t len = strlen(subcadena);
  if(subcadena[len-1] == '\n') subcadena[len-1] = 0; //Le saco el \n
}

bool agregar_archivo(char* archivo_csv, hash_t* hash, abb_t* abb){
  FILE* archivo = fopen(archivo_csv, "r");
  if(!archivo) return false;
  char* linea = NULL;
  size_t tam = 0;
  size_t linea_archivo = getline(&linea , &tam, archivo);
  while(linea_archivo != FIN){
    char** subcadenas = split(linea, ',');
    if(!subcadenas) return false;
    sacar_fin_linea(subcadenas[CANCELLED]);
    char* clave_hash = subcadenas[FLIGHT_NUMBER]; //codigo de vuelo
    char* subcadena_abb[3] = {subcadenas[DATE], subcadenas[FLIGHT_NUMBER], NULL};
    char* clave_abb = join(subcadena_abb, ' '); //fecha
    if(!clave_abb) return false;
    if(hash_pertenece(hash, clave_hash)){
      char** dato_hash = hash_obtener(hash, subcadenas[FLIGHT_NUMBER]);
      char* subcadena_abb_nueva[3] = {dato_hash[DATE], dato_hash[FLIGHT_NUMBER], NULL};
      char* clave_abb_anterior = join(subcadena_abb_nueva, ' ');
      abb_borrar(abb, clave_abb_anterior);
      free(clave_abb_anterior);
    }
    abb_guardar(abb, clave_abb, subcadenas);
    hash_guardar(hash, clave_hash, subcadenas);
    free(clave_abb);
    linea_archivo = getline(&linea , &tam, archivo);
  }
  fclose(archivo);
  free(linea);
  return true;
}

bool ver_tablero(char* cantidad, char* modo, char* desde, char* hasta, abb_t* abb){ //imprime fecha despegue y codigos de vuelo
  int cant = atoi(cantidad);
	if(cant < 1) return false;
	bool modo_ok = false;
	if((strcmp(modo, "asc") == 0) || (strcmp(modo, "desc") == 0)) modo_ok = true;
	if(!modo_ok) return false;
  abb_iter_t* iter = abb_iter_in_crear(abb, desde, hasta, modo);
  if(!iter) return false;
  for(size_t contador = 0; !abb_iter_in_al_final(iter) && contador<cant; contador++){
    const char* clave = abb_iter_in_ver_actual(iter);
    char** linea = abb_obtener(abb, clave);
    fprintf(stdout,"%s - %s\n", linea[DATE], linea[FLIGHT_NUMBER]);
    abb_iter_in_avanzar(iter);
	}
	abb_iter_in_destruir(iter);
  return true;
}

bool info_vuelo(char* codigo, hash_t* hash){
	char** dato = hash_obtener(hash, codigo);
	if(!dato) return false;
	char* linea = join(dato, ' ');
	fprintf(stdout,"%s\n", linea);
	free(linea);
	return true;
}

int comparacion(const void* valor_1, const void* valor_2){ //funcion de comparacion para el heap
	char** linea_1 = (char**)valor_1;
	char** linea_2 = (char**)valor_2;
	if(atoi(linea_1[5]) > atoi(linea_2[5])) return -1;
	else if(atoi(linea_1[5]) < atoi(linea_2[5])) return 1;
	else{
		if(strcmp(linea_1[0], linea_2[0]) > 0) return 1;
		else return -1;
	}
}

heap_t* top_k(char* cantidad, hash_t* hash){ //utilizamos un heap de minimos
	int cant = atoi(cantidad);
	if(cant < 1) return NULL;
	heap_t* heap = heap_crear(comparacion);
	if(!heap) return NULL;
	hash_iter_t* hash_iter = hash_iter_crear(hash);
	if(!hash_iter){
    heap_destruir(heap, NULL);
    return NULL;
  }
	size_t contador = 0; //para ir viendo si se pasa de la cantidad de lineas
	while (!hash_iter_al_final(hash_iter)){
    const char* clave = hash_iter_ver_actual(hash_iter);
		char* linea = hash_obtener(hash, clave);
		if(contador < cant){
			heap_encolar(heap, linea);
			contador++;
		}else if(comparacion(linea, heap_ver_max(heap)) < 0){
      heap_desencolar(heap);
      heap_encolar(heap, linea);
    }
		hash_iter_avanzar(hash_iter);
	}
  hash_iter_destruir(hash_iter);
  return heap;
}

void _prioridad_vuelos(heap_t* heap){ //imprime el heap
	char** linea = heap_desencolar(heap);
	if(!heap_esta_vacio(heap)) _prioridad_vuelos(heap);
	fprintf(stdout, "%s - %s\n", linea[5], linea[0]);
}

bool prioridad_vuelos(char* cantidad, hash_t* hash){ //imprime el heap
	heap_t* heap = top_k(cantidad, hash);
	if(!heap || heap_esta_vacio(heap)) return false;
  _prioridad_vuelos(heap);
  heap_destruir(heap, NULL);
  return true;
}

bool borrar(char* desde, char* hasta, abb_t* abb, hash_t* hash){
  abb_iter_t* abb_iter = abb_iter_in_crear(abb, desde, hasta, "asc");
  if(!abb_iter) return false;
  lista_t* lista = lista_crear();
  if(!lista) return false;
  while (!abb_iter_in_al_final(abb_iter)){
  	const char* clave_vuelo = strdup(abb_iter_in_ver_actual(abb_iter));
  	lista_insertar_ultimo(lista, (char*)clave_vuelo);
  	abb_iter_in_avanzar(abb_iter);
  }
  while(!lista_esta_vacia(lista)){
    char* clave = lista_borrar_primero(lista);
    char** vuelo = abb_borrar(abb, clave);
    if(!vuelo){
      return false;
    }
    char** temp = hash_borrar(hash, vuelo[0]);
    if(!temp){
     return false;
    }
    char* linea = join(vuelo, ' ');
    if(!linea) return false;
    fprintf(stdout, "%s\n", linea);
    free(linea);
    free(clave);
    free_strv(temp);
  }
  abb_iter_in_destruir(abb_iter);
  lista_destruir(lista, NULL);
  return true;
}

void interfaz(char* linea, hash_t* hash, abb_t* abb){
  char** subcadenas = split(linea, ' ');
  size_t tam = 0;
  bool resultado;
  for(int i=0;subcadenas[i];i++) tam++;
  char* opcion1 = "agregar_archivo";
  char* opcion2 = "ver_tablero";
  char* opcion3 = "info_vuelo";
  char* opcion4 = "prioridad_vuelos";
  char* opcion5 = "borrar";
  size_t len = strlen(subcadenas[tam-1]);
  if(subcadenas[tam-1][len-1] == '\n') subcadenas[tam-1][len-1] = 0; //Le saco el \n
  if(tam == 2 && (strcmp(subcadenas[0], opcion1)) == 0) resultado = agregar_archivo(subcadenas[1], hash, abb);
  else if(tam == 5 && (strcmp(subcadenas[0], opcion2)) == 0) resultado = ver_tablero(subcadenas[1], subcadenas[2], subcadenas[3], subcadenas[4], abb);
  else if(tam == 2 && (strcmp(subcadenas[0], opcion3)) == 0) resultado = info_vuelo(subcadenas[1], hash);
  else if(tam == 2 && (strcmp(subcadenas[0], opcion4)) == 0) resultado = prioridad_vuelos(subcadenas[1], hash);
  else if(tam == 3 && (strcmp(subcadenas[0], opcion5)) == 0) resultado = borrar(subcadenas[1], subcadenas[2], abb, hash);
  else resultado = false;
  if(!resultado){
    if(tam >= 1) fprintf(stderr,"Error en comando %s\n", subcadenas[0]);
  }else fprintf(stdout, "OK\n");
    free_strv(subcadenas);
}

void leer_entrada(FILE* entrada){
  abb_t* abb = abb_crear(strcmp, NULL);
  hash_t* hash = hash_crear(func_destruir);
  char* linea = NULL;
  size_t tam = 0;
  size_t archivo = getline(&linea , &tam, entrada);
  while(archivo != FIN){
    interfaz(linea, hash, abb);
    archivo = getline(&linea , &tam, entrada);
  }
  hash_destruir(hash);
  abb_destruir(abb);
  free(linea);
}

int main(int argc, char* argv[]){
  leer_entrada(stdin);
}
