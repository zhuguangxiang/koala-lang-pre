/*
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "hashfunc.h"
#include "mem.h"
#include "log.h"
#include "stringobject.h"
#include "codeobject.h"
#include "tupleobject.h"

static void klass_mark(Object *ob)
{
  OB_ASSERT_KLASS(ob, Klass_Klass);
  //FIXME
}

static void klass_free(Object *ob)
{
  OB_ASSERT_KLASS(ob, Klass_Klass);
  //FXIME
}

Klass Klass_Klass = {
  OBJECT_HEAD_INIT(&Klass_Klass)
  .name = "Klass",
  .basesize = sizeof(Klass),
  .ob_mark  = klass_mark,
  .ob_free  = klass_free,
};

Klass Any_Klass = {
  OBJECT_HEAD_INIT(&Klass_Klass)
  .name = "Any",
};

Klass Trait_Klass = {
  OBJECT_HEAD_INIT(&Klass_Klass)
  .name = "Trait",
  .basesize = sizeof(Klass),
  .ob_mark  = klass_mark,
  .ob_free  = klass_free,
};

int Klass_Add_CFunctions(Klass *klazz, FuncDef *funcs)
{
  int res;
  FuncDef *f = funcs;
  Object *meth;
  Vector *pdesc;
  Vector *rdesc;
  TypeDesc *proto;

  while (f->name) {
    rdesc = String_To_TypeDescList(f->rdesc);
    pdesc = String_To_TypeDescList(f->pdesc);
    proto = TypeDesc_New_Proto(pdesc, rdesc);
    meth = CLCode_New(f->fn, proto);
    res = Klass_Add_Method(klazz, f->name, meth);
    assert(!res);
    ++f;
  }
  return 0;
}

static void object_mark(Object *ob)
{
  //FIXME
  UNUSED_PARAMETER(ob);
}

static int objectsize(Klass *klazz)
{
  /* Base */
  LRONode *node = Vector_Get_Last(&klazz->lro);
  int size = node->offset;

  /* Self */
  size += klazz->itemsize;

  Log_Debug("class(trait) '%s' has %d fields", klazz->name, size);

  return sizeof(Object) + sizeof(Object *) * size;
}

static Object *object_alloc(Klass *klazz)
{
  Object *ob = gc_alloc(objectsize(klazz));
  Init_Object_Head(ob, klazz);
  return ob;
}

static void object_free(Object *ob)
{
  Log_Debug("object of '%s' is freed", OB_KLASS(ob)->name);
  gc_free(ob);
}

static uint32 object_hash(Object *ob)
{
  return hash_uint32((intptr_t)ob, 32);
}

static int object_equal(Object *ob1, Object *ob2)
{
  return ob1 == ob2;
}

static Object *object_tostring(Object *ob)
{
  char buf[64];
  snprintf(buf, 63, "%s@%x", OB_KLASS(ob)->name, (uint16)(intptr_t)ob);
  return String_New(buf);
}

static LRONode *LRONode_New(int offset, Klass *klazz)
{
  LRONode *node = mm_alloc(sizeof(LRONode));
  node->offset = offset;
  node->klazz  = klazz;
  return node;
}

static int lro_find(Vector *lros, Klass *lro)
{
  LRONode *node;
  Vector_ForEach(node, lros) {
    if (node->klazz == lro)
      return 1;
  }
  return 0;
}

static int build_one_lro(Klass *klazz, Klass *base, int offset)
{
  LRONode *node;
  Vector_ForEach(node, &base->lro) {
    if (!lro_find(&klazz->lro, node->klazz)) {
      Vector_Append(&klazz->lro, LRONode_New(offset, node->klazz));
      offset += node->klazz->itemsize;
    }
  }

  if (!lro_find(&klazz->lro, base)) {
    Vector_Append(&klazz->lro, LRONode_New(offset, base));
    offset += base->itemsize;
  }

  return offset;
}

static void lro_build(Klass *klazz, Klass *base, Vector *traits)
{
  int offset = 0;
  if (base)
    offset = build_one_lro(klazz, base, offset);
  Klass *line;
  Vector_ForEach(line, traits) {
    /* must be Trait_Klass, means a trait not a class */
    OB_ASSERT_KLASS(line, Trait_Klass);
    offset = build_one_lro(klazz, line, offset);
  }

  /* add self to the last */
  Vector_Append(&klazz->lro, LRONode_New(offset, klazz));
}

static void lro_debug(Klass *klazz)
{
  puts("++++++++++++++++line-order++++++++++++++++");
  LRONode *node;
  Vector_ForEach_Reverse(node, &klazz->lro)
    printf("  %s ->", node->klazz->name);
  puts("  Any");
  puts("++++++++++++++++++++++++++++++++++++++++++");
}

Klass *Klass_New(char *name, Klass *base, Vector *traits, Klass *type)
{
  Klass *klazz = gc_alloc(sizeof(Klass));
  Init_Object_Head(klazz, type);
  klazz->name = strdup(name);
  klazz->basesize = sizeof(Object);
  klazz->itemsize = 0;

  klazz->ob_mark  = object_mark;
  klazz->ob_alloc = object_alloc;
  klazz->ob_free  = object_free,
  klazz->ob_hash  = object_hash;
  klazz->ob_equal = object_equal;
  klazz->ob_str   = object_tostring;

  Vector_Init(&klazz->lro);
  lro_build(klazz, base, traits);
  lro_debug(klazz);

  return klazz;
}

static void free_lronode_func(void *item, void *arg)
{
  mm_free(item);
}

static void free_member_func(HashNode *hnode, void *arg)
{
  MemberDef *m = container_of(hnode, MemberDef, hnode);
  switch (m->kind) {
    case MEMBER_VAR: {
      Log_Debug("field '%s' is freed", m->name);
      break;
    }
    case MEMBER_CODE: {
      Log_Debug("method '%s' is freed", m->name);
      CodeObject_Free(m->code);
      break;
    }
    default: {
      assert(0);
      break;
    }
  }
  Member_Free(m);
}

void Klass_Free(Klass *klazz)
{
  Log_Debug("Klass '%s' is freed", klazz->name);
  Vector_Fini(&klazz->lro, free_lronode_func, NULL);
  if (klazz->table)
    HashTable_Free(klazz->table, free_member_func, NULL);
  free(klazz->name);
  gc_free(klazz);
}

static HashTable *__get_table(Klass *klazz)
{
  if (!klazz->table) {
    klazz->table = HashTable_New((ht_hashfunc)Member_Hash,
                                 (ht_equalfunc)Member_Equal);
  }
  return klazz->table;
}

int Klass_Add_Field(Klass *klazz, char *name, TypeDesc *desc)
{
  assert(OB_KLASS(klazz) == &Klass_Klass ||
         OB_KLASS(klazz) == &Trait_Klass);

  MemberDef *member = Member_Var_New(name, desc, 0);
  int res = HashTable_Insert(__get_table(klazz), &member->hnode);
  if (!res) {
    member->offset = klazz->itemsize++;
    return 0;
  } else {
    Member_Free(member);
    return -1;
  }
}

int Klass_Add_Method(Klass *klazz, char *name, Object *code)
{
  assert(OB_KLASS(klazz) == &Klass_Klass ||
         OB_KLASS(klazz) == &Trait_Klass);
  OB_ASSERT_KLASS(code, Code_Klass);

  CodeObject *co = (CodeObject *)code;
  MemberDef *member = Member_Code_New(name, co->proto);
  int res = HashTable_Insert(__get_table(klazz), &member->hnode);
  if (!res) {
    member->code = code;
    if (IS_CLCODE(code))
      co->kl.consts = klazz->consts;
    return 0;
  } else {
    Member_Free(member);
    return -1;
  }
}

int Klass_Add_Proto(Klass *klazz, char *name, TypeDesc *proto)
{
  assert(OB_KLASS(klazz) == &Klass_Klass ||
         OB_KLASS(klazz) == &Trait_Klass);

  MemberDef *member = Member_Proto_New(name, proto);
  int res = HashTable_Insert(__get_table(klazz), &member->hnode);
  if (!res) {
    return 0;
  } else {
    Member_Free(member);
    return -1;
  }
}

static MemberDef *__get_member(Klass *klazz, char *name)
{
  MemberDef key = {.name = name};
  HashNode *hnode = HashTable_Find(__get_table(klazz), &key);
  return hnode ? container_of(hnode, MemberDef, hnode) : NULL;
}

static int __get_lronode_index(Vector *lros, Klass *klazz)
{
  LRONode *item;
  Vector_ForEach_Reverse(item, lros) {
    if (item->klazz == klazz)
      return i;
  }
  return -1;
}

static int __get_field_index(Object *ob, char *name, Klass *klazz)
{
  assert(OB_KLASS(OB_KLASS(ob)) == &Klass_Klass ||
         OB_KLASS(OB_KLASS(ob)) == &Trait_Klass);

  Klass *base = OB_KLASS(ob);
  LRONode *lronode;
  MemberDef *member;
  int lro_index = __get_lronode_index(&base->lro, klazz);
  char *dot = strchr(name, '.');
  char *fieldname = name;

  if (dot) {
    assert(!memcmp(name, "super", 5));
    lro_index = lro_index - 1;
    fieldname = dot + 1;
  }

  while (lro_index >= 0) {
    lronode = Vector_Get(&base->lro, lro_index);
    assert(lronode);
    member = __get_member(lronode->klazz, fieldname);
    if (member)
      break;
    --lro_index;
  }
  assert(member);
  assert(member->kind == MEMBER_VAR);
  return lronode->offset + member->offset;
}

Object *Object_Get_Value(Object *ob, char *name, Klass *klazz)
{
  int index = __get_field_index(ob, name, klazz);
  Object **value = (Object **)(ob + 1);
  Log_Info("get_value: klass '%s', name '%s', index %d",
           klazz->name, name, index);
  return value[index];
}

Object *Object_Set_Value(Object *ob, char *name, Klass *klazz, Object *val)
{
  int index = __get_field_index(ob, name, klazz);
  Object **value = (Object **)(ob + 1);
  Log_Info("set_value: klass '%s', name '%s', index %d",
           klazz->name, name, index);
  Object *old = value[index];
  value[index] = val;
  return old;
}

Object *Object_Get_Method(Object *ob, char *name, Klass *klazz)
{
  assert(OB_KLASS(OB_KLASS(ob)) == &Klass_Klass ||
         OB_KLASS(OB_KLASS(ob)) == &Trait_Klass);
#if 0
  Object *base;
  MemberDef *member;
  char *dot = strchr(name, '.');
  if (dot) {
    assert(!memcmp(name, "super", 5));
    base = ob->ob_down;
    assert(base);
    member = __get_member(OB_KLASS(base), dot + 1);
    assert(member && member->kind == MEMBER_CODE);
    *rob = base;
    return member->code;
  } else {
    if (!strcmp(name, "__init__")) {
      //__init__ method is allowed to be searched only in current class
      member = __get_member(OB_KLASS(ob), "__init__");
      *rob = ob;
      return member->code;
    } else {
      base = ob;
      while (base) {
        member = __get_member(OB_KLASS(base), name);
        if (member) {
          *rob = base;
          return member->code;
        }
        base = base->ob_down;
      }
      return NULL;
    }
  }
#endif
  return NULL;
}

MemberDef *Member_New(int kind, char *name, TypeDesc *desc, int konst)
{
  MemberDef *member = mm_alloc(sizeof(MemberDef));
  assert(member);
  Init_HashNode(&member->hnode, member);
  member->kind = kind;
  member->name = name;
  member->desc = desc;
  member->konst = konst;
  return member;
}

void Member_Free(MemberDef *m)
{
  mm_free(m);
}

uint32 Member_Hash(MemberDef *m)
{
  return hash_string(m->name);
}

int Member_Equal(MemberDef *m1, MemberDef *m2)
{
  return !strcmp(m1->name, m2->name);
}
