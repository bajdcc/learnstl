#include "stdafx.h"
#include "Router.h"

int main()
{
    Router router;

    router.add_name("A");
    router.add_name("B");
    router.add_name("C");
    router.add_name("D");

    router.connect("A", "B", 10);
    router.connect("B", "D", 20);
    router.connect("A", "C", 15);
    router.connect("C", "D", 16);
    router.connect("B", "C", 12);

    router.run();

    return 0;
}