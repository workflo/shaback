/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2012 Florian Wolff (florian@donuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <panel.h>

#include "RestoreGui.h"

using namespace std;

RestoreGui::RestoreGui(RuntimeConfig& config, Repository& repository) :
  repository(repository), config(config)
{
}

RestoreGui::~RestoreGui()
{
  repository.unlock();
}

void RestoreGui::run()
{
  ::init_dialog(stdin, stdout);
  //::dialog_calendar("Datum", "b", 0, 40, -1, -1, -1);
  ::dialog_dselect("New repository directory", "/Users", 30, 80);
  ::end_dialog();
}

