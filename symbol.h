
#ifndef _KOALA_SYMBOL_H_
#define _KOALA_SYMBOL_H_

#include "codeformat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYM_VAR     1
#define SYM_PROTO   2
#define SYM_CLASS   3
#define SYM_INTF    4
#define SYM_IPROTO  5
#define SYM_STABLE  6

#define ACCESS_PUBLIC   0
#define ACCESS_PRIVATE  1

typedef struct symboltable STable;

typedef struct symbol {
	HashNode hnode;
	idx_t name;
	int8 kind;
	int8 access;
	int8 konst;
	int8 refcnt;    /* for compiler */
	idx_t desc;     /* variable's type or func's proto */
	char *str;      /* -> name */
	TypeDesc *type; /* -> desc */
	int32 locvars;  /* for compiler's function */
	struct symbol *up;  /* for compiler's id */
	STable *stbl;   /* for compiler's import */
	void *code;     /* CodeObject */
	void *klazz;    /* Klass */
	void *block;    /* CodeBlock */
	idx_t index;    /* variable's index */
} Symbol;

struct symboltable {
	HashTable *htbl;
	AtomTable *atbl;
	int flag;
	idx_t next;
};

/* Exported APIs */
int STbl_Init(STable *stbl, AtomTable *atbl);
void STbl_Fini(STable *stbl);
STable *STbl_New(AtomTable *atbl);
void STbl_Free(STable *stbl);
Symbol *STbl_Add_Var(STable *stbl, char *name, TypeDesc *desc, bool konst);
Symbol *STbl_Add_Proto(STable *stbl, char *name, Proto *proto);
Symbol *STbl_Add_IProto(STable *stbl, char *name, Proto *proto);
#define STbl_Add_Class(stbl, name) STbl_Add_Symbol(stbl, name, SYM_CLASS, 0)
#define STbl_Add_Intf(stbl, name) STbl_Add_Symbol(stbl, name, SYM_INTF, 0)
Symbol *STbl_Add_Symbol(STable *stbl, char *name, int kind, bool konst);
Symbol *STbl_Get(STable *stbl, char *name);
typedef void (*symbolfunc)(Symbol *sym, void *arg);
void STbl_Traverse(STable *stbl, symbolfunc fn, void *arg);
void STbl_Show(STable *stbl, int detail);
void STbl_Delete(STable *stbl, Symbol *sym);
#define STbl_Count(stbl) HashTable_Count((stbl)->htbl)
int STbl_Update_Symbol(STable *stbl, Symbol *sym, TypeDesc *desc);

#ifdef __cplusplus
}
#endif
#endif /* _KOALA_SYMBOL_H_ */
