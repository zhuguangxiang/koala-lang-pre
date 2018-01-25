
#ifndef _KOALA_COMPILE_H_
#define _KOALA_COMPILE_H_

#include "koala.h"
#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

struct scope {
  struct list_head link;
  HashTable *stable;
  ItemTable *itable;
};

struct compiler {
  int count;
  char *package;
  Vector *stmts;
  ModuleObject *module;
  int avail_index;
  int scope;
  struct list_head scopes;
};

int init_compiler(struct compiler *cp);
int fini_compiler(struct compiler *cp);
int compile(struct compiler *cp);

#ifdef __cplusplus
}
#endif
#endif /* _KOALA_COMPILE_H_ */
