/*
 * Severalnines Tools
 * Copyright (C) 2016  Severalnines AB
 *
 * This file is part of s9s-tools.
 *
 * s9s-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar. If not, see <http://www.gnu.org/licenses/>.
 */
#include "ut_s9sregexp.h"

#include <cstdio>
#include <cstring>

#include "S9sRegExp"
#include "s9sregexp_p.h"

//#define DEBUG
//#define WARNING
#include "s9sdebug.h"

UtS9sRegExp::UtS9sRegExp()
{
    S9S_DEBUG("");
}

UtS9sRegExp::~UtS9sRegExp()
{
}

bool
UtS9sRegExp::runTest(const char *testName)
{
    bool retval = true;

    PERFORM_TEST(testCreate,        retval);
    PERFORM_TEST(test01,            retval);
    PERFORM_TEST(testMatched01,     retval);

    return retval;
}

/**
 * Testing the constructors.
 */
bool
UtS9sRegExp::testCreate()
{
    S9sRegExp regexp1;
    S9sRegExp regexp2("/some/");
    S9sRegExp regexp3("/other/ig");
    S9sRegExp regexp4("nice");
    S9sRegExp regexp5("/nice");

    S9S_COMPARE(regexp1.toString(), "");

    S9S_COMPARE(regexp2.toString(), "some");
    S9S_VERIFY(!regexp2.m_priv->m_ignoreCase);
    S9S_VERIFY(!regexp2.m_priv->m_global);
    
    S9S_COMPARE(regexp3.toString(), "other");
    S9S_VERIFY(regexp3.m_priv->m_ignoreCase);
    S9S_VERIFY(regexp3.m_priv->m_global);
    
    S9S_COMPARE(regexp4.toString(), "nice");
    S9S_VERIFY(!regexp4.m_priv->m_ignoreCase);
    S9S_VERIFY(!regexp4.m_priv->m_global);
    
    S9S_COMPARE(regexp5.toString(), "/nice");
    S9S_VERIFY(!regexp5.m_priv->m_ignoreCase);
    S9S_VERIFY(!regexp5.m_priv->m_global);

    return true;
}

/**
 * Testing the simplest feature, the == and the != operator.
 */
bool
UtS9sRegExp::test01()
{
    S9sRegExp  regexp;

    regexp = "n:[0-9]+";
    S9S_COMPARE(regexp.toString(), "n:[0-9]+");

    S9S_VERIFY(regexp != "some");
    S9S_VERIFY(regexp == "n:42");
    S9S_VERIFY(regexp == "n:4200");

    return true;
}

/**
 * Setting up the regexp with the assignment operator, testing the == operator
 * and the [] operator that returns substrings internally stored by the ==
 * operator. This is a great way to discover the match and then investigate the
 * details without any furter execution.
 */
bool
UtS9sRegExp::testMatched01()
{
    S9sRegExp  regexp;

    regexp = "n:([0-9]+)";

    S9S_VERIFY(regexp == "n:42");
    S9S_COMPARE(regexp[1], "42");

    S9S_VERIFY(regexp == "n:4200");
    S9S_COMPARE(regexp[1], "4200");

    return true;
}

S9S_UNIT_TEST_MAIN(UtS9sRegExp)


