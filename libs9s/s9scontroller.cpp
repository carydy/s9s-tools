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
 * s9s-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s9s-tools. If not, see <http://www.gnu.org/licenses/>.
 */
#include "s9scontroller.h"

//#define DEBUG
//#define WARNING
#include "s9sdebug.h"

S9sController::S9sController() :
    S9sServer()
{
    m_properties["class_name"] = "CmonController";
}

S9sController::S9sController(
        const S9sController &orig) :
    S9sServer(orig)
{
}

S9sController::S9sController(
        const S9sVariantMap &properties) :
    S9sServer(properties)
{
    if (!m_properties.contains("class_name"))
        m_properties["class_name"] = "CmonController";
}

S9sController::~S9sController()
{
}

S9sController &
S9sController::operator=(
        const S9sVariantMap &rhs)
{
    setProperties(rhs);
    
    return *this;
}

