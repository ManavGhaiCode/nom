#define _NOM_IMPLEMENTATION_
#include "nom.h"

int main( void ) {
    NOM_ERROR("%s", PATH("Foo", "Bar", "Test"));
    NOM_WARN("%s", PATH("Foo", "Bar", "Test"));
    NOM_INFO("%s", PATH("Foo", "Bar", "Test"));
}