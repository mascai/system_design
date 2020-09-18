//
// Created by alex on 05.09.2020.
//


#include "test_runner.h"


void Assert(bool cond, const std::string& hint)
{
    AssertEqual(cond, true, hint);
}


TestRunner::TestRunner()
{
    fail_count = 0;
}


TestRunner::~TestRunner()
{
    if (fail_count > 0)
    {
        std::cerr << fail_count << " tests failed. Terminate";
        exit(1);
    }
}