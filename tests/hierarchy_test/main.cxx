
#include <iostream>
#include <string>
#include "log4cplus/hierarchy.h"
#include "log4cplus/helpers/loglog.h"

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

int
main()
{
    {

    cout << "Priorities:" << endl;
    PriorityList list = Priority::getAllPossiblePriorities();
    for(PriorityList::iterator it=list.begin(); it!=list.end(); ++it) {
        cout << "   " << it->toString() << endl;
    }
    Category cat = Category::getInstance("test");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test2");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest.a.b.c");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest.a");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest.a");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest.a.b.c");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest.a.b.c.d");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest.a.b.c");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest.a");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;

    cat = Category::getInstance("test.subtest");
    cout << "Category name: " << cat.getName()
         << " Parent = " << cat.getParent().getName() << endl;


    Category::shutdown();
    }
    cout << "Exiting main()..." << endl;
    return 0;
}

