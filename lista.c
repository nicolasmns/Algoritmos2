#include "lista.h"
#include <string.h>
#include <stdlib.h>

typedef struct nodo{
  struct nodo* siguiente;
  void* dato;
}nodo_t;

struct lista{
  nodo_t* primero;
  nodo_t* ultimo;
  size_t largo;
};

struct lista_iter{
  lista_t* lista;
  nodo_t* anterior;
  nodo_t* actual;
};

lista_t* lista_crear(void){
  lista_t* lista = malloc(sizeof(lista_t));
  if(lista == NULL) return NULL;
  lista->primero = NULL;
  lista->ultimo = NULL;
  lista->largo = 0;
  return lista;
}

nodo_t* nodo_crear(void* dato){
  nodo_t* nodo = malloc(sizeof(nodo_t));
  if(nodo == NULL) return NULL;
  nodo->dato = dato;
  nodo->siguiente = NULL;
  return nodo;
}

bool lista_esta_vacia(const lista_t *lista){
  return lista->largo == 0;
}

bool lista_insertar_primero(lista_t *lista, void *dato){
  nodo_t* nodo = nodo_crear(dato);
  if(!nodo) return false;
  if(lista_esta_vacia(lista)) lista->ultimo = nodo;
  nodo->siguiente = lista->primero;
  lista->primero = nodo;
  lista->largo++;
  return true;
}

bool lista_insertar_ultimo(lista_t *lista, void *dato){
  nodo_t* nodo = nodo_crear(dato);
  if(!nodo) return false;
  if(lista_esta_vacia(lista)) lista->primero = nodo;
  else lista->ultimo->siguiente = nodo;
  lista->ultimo = nodo;
  lista->largo++;
  return true;
}

void* lista_ver_primero(const lista_t *lista){
  if(lista_esta_vacia(lista) == true) return NULL;
  return lista->primero->dato;
}

void* lista_ver_ultimo(const lista_t* lista){
  if(lista_esta_vacia(lista) == true) return NULL;
  return lista->ultimo->dato;
}

size_t lista_largo(const lista_t *lista){
  return lista->largo;
}

lista_iter_t* lista_iter_crear(lista_t *lista){
  lista_iter_t* iter = malloc(sizeof(lista_iter_t));
  if(iter == NULL) return NULL;
  iter->lista = lista;
  iter->actual = lista->primero;
  iter->anterior = NULL;
  return iter;
}

void lista_destruir(lista_t *lista, void destruir_dato(void *)){
  while(!lista_esta_vacia(lista)){
    void* dato = lista_borrar_primero(lista);
    if(destruir_dato) destruir_dato(dato);
  }
  free(lista);
}

void *lista_borrar_primero(lista_t *lista){
  if (lista_esta_vacia(lista)) return NULL;
  nodo_t* nodo_aux = lista->primero;
  void* valor_aux = nodo_aux->dato;
  if(lista->largo == 1){
    lista->primero = NULL;
    lista->ultimo = NULL;
  }else lista->primero = lista->primero->siguiente;
  lista->largo -= 1;
  free(nodo_aux);
  return valor_aux;
}

bool lista_iter_avanzar(lista_iter_t *iter){
  if(iter->actual == NULL) return false;
  iter->anterior = iter->actual;
  iter->actual = iter->actual->siguiente;
  return true;
}

void* lista_iter_ver_actual(const lista_iter_t *iter){
  if(lista_esta_vacia(iter->lista)|| lista_iter_al_final(iter)) return NULL;
  return iter->actual->dato;
}

bool lista_iter_al_final(const lista_iter_t *iter){
  return iter->actual == NULL;
}

void lista_iter_destruir(lista_iter_t *iter){
  free(iter);
}

bool lista_iter_insertar(lista_iter_t* iter, void* dato){
  nodo_t* nodo = nodo_crear(dato);
  if(!nodo) return false;
  if(lista_esta_vacia(iter->lista)){
    iter->lista->primero = nodo;
    iter->lista->ultimo = nodo;
    iter->actual = nodo;
  }else if(iter->actual == iter->lista->primero){
    nodo->siguiente = iter->actual;
    iter->actual = nodo;
    iter->lista->primero = nodo;
  }else if(iter->anterior == iter->lista->ultimo){
    iter->anterior->siguiente = nodo;
    iter->actual = nodo;
    iter->lista->ultimo = iter->actual;
    iter->actual->siguiente = NULL;
  }else{
    iter->anterior->siguiente = nodo;
    nodo->siguiente = iter->actual;
    iter->actual = nodo;
  }
  iter->lista->largo++;
  return true;
}

void* lista_iter_borrar(lista_iter_t *iter){
  if(lista_esta_vacia(iter->lista) || iter->actual == NULL) return NULL;
  nodo_t* nodo_a_borrar = iter->actual;
  if(nodo_a_borrar == iter->lista->primero){
    iter->actual = iter->actual->siguiente;
    iter->lista->primero = iter->actual;
  }else if(iter->lista->largo == 1){
    iter->lista->primero = NULL;
    iter->lista->ultimo = NULL;
    iter->actual = NULL;
  }else if(iter->actual == iter->lista->ultimo){
    iter->actual = iter->actual->siguiente;
    iter->lista->ultimo = iter->anterior;
    iter->anterior->siguiente = NULL;
  }else{
    iter->actual = iter->actual->siguiente;
    iter->anterior->siguiente = iter->actual;
  }
  iter->lista->largo -= 1;
  void* valor_a_borrar = nodo_a_borrar->dato;
  free(nodo_a_borrar);
  return valor_a_borrar;
}

void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra){
  nodo_t* nodo_actual = lista->primero;
  while(nodo_actual != NULL){
    bool resultado = visitar(nodo_actual->dato, extra);
    if (!resultado) break;
    nodo_actual = nodo_actual->siguiente;
  }
}
