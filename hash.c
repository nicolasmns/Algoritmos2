#define _POSIX_C_SOURCE 200809L
#include "hash.h"
#include "lista.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#define CAPACIDAD_INICIAL 41
#define POSICION_INVALIDA -1
#define FACTOR_CARGA 2.5
#define FACTOR_DUPLICAR 2
#define FACTOR_DIVIDIR 2
#define FACTOR_REDUCIR 1

//-------------Estructuras-------------\\ .
struct hash{
  lista_t** listas;
  size_t cantidad;
  size_t capacidad;
  hash_destruir_dato_t func_destruct;
};

typedef struct campo_hash{
  char* clave;
  void* dato;
}campo_hash_t;

struct hash_iter {
  ssize_t pos;
  lista_iter_t* iter_lista;
  const hash_t* hash;
};

//-------------Funciones del Hash-------------\\ .
void destruir_campo(hash_t* hash,campo_hash_t* campo){
	free(campo->clave);
	free(campo);
}

size_t hashear(size_t capacidad, const char* clave){
  int suma = 0;
  size_t largo_clave = strlen(clave);
  for (int i = 0; i < largo_clave; i++){
    suma = suma * 7 + clave[i];
  }
  return ((size_t)suma) % capacidad;
}

campo_hash_t* crear_campo(const char *clave, void* dato) {
    if(!clave) return NULL;
    campo_hash_t* campo = malloc(sizeof(campo_hash_t));
    if(!campo) return NULL;
    char* clave_nueva= strdup(clave);
    campo->clave=clave_nueva;
    campo->dato=dato;
    return campo;
}

void eliminar_listas(hash_t* hash, size_t pos){
  for(int i=0; i<pos;i++){
    free(hash->listas[i]);
  }
}

hash_t* crear_listas(hash_t* hash){
  for(size_t pos = 0; pos < hash->capacidad; pos++){
    hash->listas[pos] = lista_crear();
    if(hash->listas[pos]==NULL){
      eliminar_listas(hash, pos);
      return NULL;
    }
  }
  return hash;

}

bool hash_redimensionar(hash_t* hash, size_t tam_nuevo){
  lista_t** listas_vieja = hash->listas;
  hash->listas = malloc(sizeof(lista_t*)*tam_nuevo);
	if (!hash->listas){
		hash->listas=listas_vieja;
		return false;
	};
  size_t vieja_capacidad = hash->capacidad;
  hash->capacidad = tam_nuevo;
  hash->cantidad = 0;
  hash = crear_listas(hash);
  for(int i = 0;i<vieja_capacidad;i++){
    while(!lista_esta_vacia(listas_vieja[i])){
      campo_hash_t* nodo_aux = lista_borrar_primero(listas_vieja[i]);
      hash_guardar(hash, nodo_aux->clave, nodo_aux->dato);
      destruir_campo(hash,nodo_aux);
    }
    free(listas_vieja[i]);
  }
  free(listas_vieja);
  return true;
}

hash_t* hash_crear(hash_destruir_dato_t destruir_dato){
  hash_t* hash = malloc(sizeof(hash_t));
	if (hash == NULL) return NULL;
  hash->capacidad = CAPACIDAD_INICIAL;
  hash->func_destruct = destruir_dato;
  hash->cantidad = 0;
	hash->listas = malloc(sizeof(lista_t*) * hash->capacidad);
  if (hash->listas == NULL){
		free(hash);
		return NULL;
	}
  hash = crear_listas(hash);
	return hash;
}

lista_iter_t* obtener_iterador_clave(const hash_t* hash, const char* clave, lista_t** listas ){
  	if(!hash || !clave) return NULL;
    size_t i = hashear(hash->capacidad,clave);
    lista_t* lista = hash->listas[i];
    lista_iter_t* lista_iter = lista_iter_crear(lista);
    if(!lista_iter) return NULL;
    if(listas) *listas=lista;
    while(!lista_iter_al_final(lista_iter) && strcmp(clave,((campo_hash_t*)lista_iter_ver_actual(lista_iter))->clave)){
        lista_iter_avanzar(lista_iter);
    }
    return lista_iter;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	if(!hash) return false;
  if(hash->cantidad/hash->capacidad >= FACTOR_CARGA){
    bool redimension = hash_redimensionar(hash, hash->capacidad*FACTOR_DUPLICAR);
    if(redimension == false) return false;
  }
  lista_t* lista = NULL;
  lista_iter_t* lista_iter=obtener_iterador_clave(hash, clave, &lista);
  if (lista_iter && !lista_iter_al_final(lista_iter)){
    campo_hash_t* nodo = lista_iter_ver_actual(lista_iter);
    if(hash->func_destruct) hash->func_destruct(nodo->dato);
    nodo->dato=dato;
    lista_iter_destruir(lista_iter);
    return true;
  }
  lista_iter_destruir(lista_iter);
  campo_hash_t* nodo = crear_campo(clave, dato);
  if(!nodo) return false;
  bool insertado=lista_insertar_ultimo(lista, nodo);
  if(!insertado){
    free(nodo->clave);
    free(nodo);
    return false;
  }
  hash->cantidad++;
  return insertado;
}

void* hash_borrar(hash_t *hash, const char *clave){
	if (!hash_pertenece(hash,clave)) return NULL;
	size_t pos = hashear(hash->capacidad,clave);
	lista_iter_t* iter=lista_iter_crear(hash->listas[pos]);
	campo_hash_t* nodo;
	while(!lista_iter_al_final(iter)){
		nodo = lista_iter_ver_actual(iter);
		if (strcmp(nodo->clave,clave) == 0){
			void* valor=nodo->dato;
			lista_iter_borrar(iter);
			lista_iter_destruir(iter);
			hash->cantidad--;
      free(nodo->clave);
      if(hash->cantidad/hash->capacidad < FACTOR_REDUCIR){
        bool redimension = hash_redimensionar(hash, hash->capacidad/FACTOR_DIVIDIR);
        if(!redimension) return NULL;
      }
      free(nodo);
			return valor;
		}
		lista_iter_avanzar(iter);
	}
	lista_iter_destruir(iter);
	return NULL;
}

campo_hash_t* hash_obtener_campo(const hash_t *hash, const char *clave){
	if (!hash) return NULL;
	size_t pos = hashear(hash->capacidad,clave);
	if(!lista_esta_vacia(hash->listas[pos])){
		lista_iter_t* iter = lista_iter_crear(hash->listas[pos]);
		if (!iter) return NULL;
		campo_hash_t* nodo;
		while (!lista_iter_al_final(iter)){
			nodo=lista_iter_ver_actual(iter);
			if (strcmp(nodo->clave,clave) == 0){
				lista_iter_destruir(iter);
				return nodo;
			}
			lista_iter_avanzar(iter);
		}
		lista_iter_destruir(iter);
	}
	return NULL;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
	return hash_obtener_campo(hash,clave) != NULL;
}

void *hash_obtener(const hash_t *hash, const char *clave){
	campo_hash_t* nodo = hash_obtener_campo(hash,clave);
	if (nodo != NULL) return nodo->dato;
	return NULL;
}

size_t hash_cantidad(const hash_t *hash){
	return hash->cantidad;
}

void hash_destruir(hash_t *hash){
  for(size_t i=0;i<hash->capacidad;i++){
    lista_t* lista = hash->listas[i];
    if(!lista) continue;
    while(!lista_esta_vacia(lista)){
      campo_hash_t* nodo = lista_borrar_primero(lista);
      if(hash->func_destruct != NULL) hash->func_destruct(nodo->dato);
      free(nodo->clave);
      free(nodo);
    }
    free(lista);
  }
  free(hash->listas);
  free(hash);
}

//-------------Funciones del iterador-------------\\ .

//Busca el siguiente posicion valida disponible del hash
ssize_t buscar_siguiente_libre(hash_iter_t* iter){
  for(iter->pos+=1; iter->pos<iter->hash->capacidad;iter->pos++){
    if(!lista_esta_vacia(iter->hash->listas[iter->pos])) return iter->pos;
  }
  return POSICION_INVALIDA;
}

hash_iter_t *hash_iter_crear(const hash_t *hash){
	hash_iter_t* iter=malloc(sizeof(hash_iter_t));
	if (!iter) return NULL;
	iter->hash = hash;
  iter->pos = -1;
  iter->pos = buscar_siguiente_libre(iter);
  iter->iter_lista = NULL;
  if(iter->pos != POSICION_INVALIDA) iter->iter_lista=lista_iter_crear(iter->hash->listas[iter->pos]);
  return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
	if (hash_iter_al_final(iter)) return false;
	if (!lista_iter_al_final(iter->iter_lista)) lista_iter_avanzar(iter->iter_lista);
  if(lista_iter_al_final(iter->iter_lista)){
    free(iter->iter_lista);
    ssize_t nueva_pos = buscar_siguiente_libre(iter);
    iter->iter_lista = NULL;
    if(nueva_pos != POSICION_INVALIDA) iter->iter_lista = lista_iter_crear(iter->hash->listas[nueva_pos]);
  }
  return true;
}

bool hash_iter_al_final(const hash_iter_t *iter){
  return iter->iter_lista == NULL;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
  if(!iter->iter_lista || hash_iter_al_final(iter)) return NULL;
  campo_hash_t* nodo_aux = lista_iter_ver_actual(iter->iter_lista);
  return nodo_aux->clave;
}

void hash_iter_destruir(hash_iter_t* iter){
	lista_iter_destruir(iter->iter_lista);
	free(iter);
}
