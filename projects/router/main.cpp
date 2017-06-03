#include "stdafx.h"
#include "Router.h"

int main()
{
    Router router;

    router.add_name("A");
    router.add_name("B");
    router.add_name("C");
    router.add_name("D");
    router.add_name("E");

    router.connect("A", "B", 10);
    router.connect("B", "D", 50);
    router.connect("A", "C", 48);
    router.connect("C", "D", 8);
    router.connect("B", "C", 5);
    router.connect("B", "E", 10);
    router.connect("D", "E", 45);
    router.connect("A", "E", 5);

    router.run();

    return 0;
}