#ifndef LISTA_H
#define LISTA_H
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

typedef struct lista lista_t;

typedef struct lista_iter lista_iter_t;

/* ******************************************************************
 *                    PRIMITIVAS DE LA LISTA
 * *****************************************************************/
//Pre: -
//Post: Devuelve una nueva lista vacia
lista_t* lista_crear(void);

//Pre: La lista fue creada
//Post: Devuelve verdadero si la lista esta vacia, false en caso contrario
bool lista_esta_vacia(const lista_t *lista);

//Pre: La lista fue creada
//Post:Se agrego un nuevo elemento en la primera posicion de la lista.
bool lista_insertar_primero(lista_t *lista, void *dato);

//Pre: La lista fue creada
//Post:Se agrego un nuevo elemento en la ultima posicion de la lista.
bool lista_insertar_ultimo(lista_t *lista, void *dato);

//Pre: La lista fue creada y no esta vacia
//Post: Se borra el primer elemento de la lista
void *lista_borrar_primero(lista_t *lista);

//Pre: La lista fue creada y no esta vacia
//Post: devuelve la primera posicion de la lista
void *lista_ver_primero(const lista_t *lista);

//Pre: La lista fue creada y no esta vacia
//Post: devuelve la ultima posicion de la lista
void *lista_ver_ultimo(const lista_t* lista);

//Pre: La lista fue creada
//Post: Devuelve el largo de la lista
size_t lista_largo(const lista_t *lista);

//Pre: La lista fue creada
//Post: Se eliminan todos los elementos de la lista y la lista
void lista_destruir(lista_t *lista, void destruir_dato(void *));

/* ******************************************************************
 *                 PRIMITIVAS DEL ITERADOR INTERNO
 * *****************************************************************/
//Pre: La lista fue creada
//Post: Se recorre toda la lista y se aplica la funcion visitar en cada dato
void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra);

/* ******************************************************************
 *                 PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/
//Pre: La lista fue creada
//Post: Se crea el iterador
lista_iter_t* lista_iter_crear(lista_t *lista);

//Pre: La lista y el iterador fueron creados
//Post: El iterador avanza al siguiente nodo
bool lista_iter_avanzar(lista_iter_t *iter);

//Pre: La lista y el iterador fueron creados y la lista tiene elementos
//Post: se devuelve el elemento al que apunta el iterador
void* lista_iter_ver_actual(const lista_iter_t *iter);

//Pre: La lista y el iterador fueron creados
//Post: La lista devuelve true si el iterador llego al final, false en caso contrario
bool lista_iter_al_final(const lista_iter_t *iter);

//Pre: El iterador fue creado
//Post: Se destruye el iterador
void lista_iter_destruir(lista_iter_t *iter);

//Pre: La lista y el iterador fueron creados
//Post: Se inserta un elemento en la posicion del iterador
bool lista_iter_insertar(lista_iter_t *iter, void *dato);

//Pre: La lista y el iterador fueron creados
//Post: Se borra el elemento en la posicion del iterador
void* lista_iter_borrar(lista_iter_t *iter);

#endif
