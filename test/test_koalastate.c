
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stringobject.h"
#include "koalastate.h"
#include "intobject.h"

void test_koalastate(void)
{
  Package *pkg = Package_New("lang");
  Package_Add_Var(pkg, "Foo", &Int_Type, 0);
  Package_Add_Var(pkg, "Bar", &Int_Type, 0);
  Koala_Install_Package("github.com/home/zgx/koala", pkg);
  Koala_Set_Object(pkg, "Foo", Integer_New(100));
  Object *ob = Koala_Get_Object(pkg, "Foo");
  assert(ob);
  assert(100 == Integer_ToCInt(ob));
  Koala_Set_Object(pkg, "Foo", String_New("hello"));
  ob = Koala_Get_Object(pkg, "Foo");
  assert(ob);
  assert(!strcmp("hello", String_RawString(ob)));
  Koala_Install_Package("github.com/", pkg);
  pkg = Package_New("io");
  Koala_Install_Package("github.com/home/zgx/koala/", pkg);
  Koala_Install_Package("github.com/", pkg);
  Koala_Install_Package("github.com/home/", pkg);
  Koala_Show_Packages();
}

int main(int argc, char *argv[])
{
  AtomString_Init();
  Init_String_Klass();
  Init_Integer_Klass();
  Init_Package_Klass();
  Koala_Initialize();
  test_koalastate();
  Koala_Finalize();
  Fini_Package_Klass();
  Fini_Integer_Klass();
  Fini_String_Klass();
  AtomString_Fini();
  return 0;
}
