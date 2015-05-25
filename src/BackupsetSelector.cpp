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

#include "BackupsetSelector.h"

 #if defined(HAVE_DIALOG)

#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include "Repository.h"
#include "dialog.h"

using namespace std;


BackupsetSelector::BackupsetSelector(Repository& repository, RuntimeConfig& config)
	: repository(repository), config(config)
{
  init_dialog(stdin, stdout);
}


BackupsetSelector::~BackupsetSelector()
{
  // end_dialog();
}


std::string BackupsetSelector::start()
{
	selectHost();
	return "sss";
}


void BackupsetSelector::selectHost()
{	
  string pattern("*_????" "-??" "-??_??????.sroot");

  vector<File> indexFiles = config.indexDir.listFiles(pattern);
  set<string> setNames; 

  for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
    File file(*it);
    string setName = file.fname.substr(0, file.fname.length() - 24);
    setNames.insert(setName);
  }

  for (set<string>::iterator it = setNames.begin(); it != setNames.end(); it++) {
    string setName(*it);
    cout << setName << endl;
  }

  // sort(indexFiles.begin(), indexFiles.end(), filePathComparator);

  dialog_yesno("Titel des Dialogs", "Soll ich nun wirklich oder doch lieber nicht?", 8, 70);


  int count = setNames.size();
  // char* items[] = {"1", "Erster Eintrag", "2","Zweiter Eintrag", "3","Dritter und letzter Eintrag"};
  const char** items = (const char**) malloc(count * 2 * sizeof(char*));

  int n = 0;
  for (set<string>::iterator it = setNames.begin(); it != setNames.end(); it++) {
    string setName(*it);
    items[n++] = "111";
    items[n++] = setName.c_str();
  }

  int sel = dialog_menu("Shaback recovery", "Select backup set to recover from:", 22, 76, 0, count, (char **) items);
  free(items);
}

 #endif
