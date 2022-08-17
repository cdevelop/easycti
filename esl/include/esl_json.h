/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#include "esl.h"
#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* cJSON Types: */
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6
	
#define cJSON_IsReference 256

/* The cJSON structure: */
typedef struct cJSON {
	struct cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==cJSON_String */
	int valueint;				/* The item's number, if type==cJSON_Number */
	double valuedouble;			/* The item's number, if type==cJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} cJSON;

typedef struct cJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} cJSON_Hooks;

/* Supply malloc, realloc and free functions to cJSON */
ESL_DECLARE(void) cJSON_InitHooks(cJSON_Hooks* hooks);


/* Supply a block of JSON, and this returns a cJSON object you can interrogate. Call cJSON_Delete when finished. */
ESL_DECLARE(cJSON *)cJSON_Parse(const char *value);
/* Render a cJSON entity to text for transfer/storage. Free the char* when finished. */
ESL_DECLARE(char *)cJSON_Print(cJSON *item);
/* Render a cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
ESL_DECLARE(char *)cJSON_PrintUnformatted(cJSON *item);
/* Delete a cJSON entity and all subentities. */
ESL_DECLARE(void)   cJSON_Delete(cJSON *c);

/* Returns the number of items in an array (or object). */
ESL_DECLARE(int)	  cJSON_GetArraySize(cJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
ESL_DECLARE(cJSON *)cJSON_GetArrayItem(cJSON *array,int item);
/* Get item "string" from object. Case insensitive. */
ESL_DECLARE(cJSON *)cJSON_GetObjectItem(cJSON *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds. */
ESL_DECLARE(const char *)cJSON_GetErrorPtr(void);
	
/* These calls create a cJSON item of the appropriate type. */
ESL_DECLARE(cJSON *)cJSON_CreateNull(void);
ESL_DECLARE(cJSON *)cJSON_CreateTrue(void);
ESL_DECLARE(cJSON *)cJSON_CreateFalse(void);
ESL_DECLARE(cJSON *)cJSON_CreateBool(int b);
ESL_DECLARE(cJSON *)cJSON_CreateNumber(double num);
ESL_DECLARE(cJSON *)cJSON_CreateString(const char *string);
ESL_DECLARE(cJSON *)cJSON_CreateArray(void);
ESL_DECLARE(cJSON *)cJSON_CreateObject(void);

/* These utilities create an Array of count items. */
ESL_DECLARE(cJSON *)cJSON_CreateIntArray(int *numbers,int count);
ESL_DECLARE(cJSON *)cJSON_CreateFloatArray(float *numbers,int count);
ESL_DECLARE(cJSON *)cJSON_CreateDoubleArray(double *numbers,int count);
ESL_DECLARE(cJSON *)cJSON_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
ESL_DECLARE(void) cJSON_AddItemToArray(cJSON *array, cJSON *item);
ESL_DECLARE(void)	cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing cJSON to a new cJSON, but don't want to corrupt your existing cJSON. */
ESL_DECLARE(void) cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);
ESL_DECLARE(void)	cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
ESL_DECLARE(cJSON *)cJSON_DetachItemFromArray(cJSON *array,int which);
ESL_DECLARE(void)   cJSON_DeleteItemFromArray(cJSON *array,int which);
ESL_DECLARE(cJSON *)cJSON_DetachItemFromObject(cJSON *object,const char *string);
ESL_DECLARE(void)   cJSON_DeleteItemFromObject(cJSON *object,const char *string);
	
/* Update array items. */
ESL_DECLARE(void) cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem);
ESL_DECLARE(void) cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);

#define cJSON_AddNullToObject(object,name)	cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)	cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddNumberToObject(object,name,n)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#ifdef __cplusplus
}
#endif

#endif

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet:
 */
