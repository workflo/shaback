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

#include <sys/types.h>
//#include <limits.h>

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
  end_dialog();
}


void freeItemList(char** items)
{
  for (int idx = 0; items[idx]; idx++) {
    free(items[idx]);
  }
  free(items);
}


std::string BackupsetSelector::start()
{
  while(true) {
  	if (!selectSet()) {
      return "";
    }
    if (!selectVersion()) {
      continue;
    }
  }
}


bool BackupsetSelector::selectSet()
{	
  string pattern("*_????" "-??" "-??_??????.sroot");

  vector<File> indexFiles = config.indexDir.listFiles(pattern);
  set<string> setNames; 

  for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
    File file(*it);
    string setName = file.fname.substr(0, file.fname.length() - 24);
    setNames.insert(setName);
  }

  if (setNames.size() == 0) {
    dialog_msgbox("Shaback recover", "\n\nNo backup set found to recover from.", 8, 50, 1);
    return false;
  } else if (setNames.size() == 1) {
    return true;
  }

  // for (set<string>::iterator it = setNames.begin(); it != setNames.end(); it++) {
  //   string setName(*it);
  //   cout << setName << endl;
  // }

  // sort(indexFiles.begin(), indexFiles.end(), filePathComparator);

  // dialog_yesno("Titel des Dialogs", "Soll ich nun wirklich oder doch lieber nicht?", 8, 70);


  int count = setNames.size();
  char** items = (char**) calloc(count + 1, 2 * sizeof(char*));

  int n = 0;
  for (set<string>::iterator it = setNames.begin(); it != setNames.end(); it++) {
    string setName(*it);
    
    items[n] = (char*) malloc(sizeof(char) * 10);
    sprintf(items[n], "%d", n/2);
    n++;

    items[n] = (char*) malloc(sizeof(char) * (1 + setName.size()));
    strcpy(items[n], setName.c_str());
    n++;
  }

  dlg_clr_result();
  int rc = dialog_menu("Shaback recovery", "Select backup set to recover from:", 0, 76, 0, count, (char **) items);
  if (rc == 0) {
    int sel = strtol(dialog_vars.input_result, 0, 10);
    setName = items[sel *2 +1];
    freeItemList(items);
    return true;
  } else {
    freeItemList(items);
    return false;
  }
}




bool BackupsetSelector::selectVersion()
{ 
  string pattern(setName);
  pattern += "_????" "-??" "-??_??????.sroot";

  vector<File> indexFiles = config.indexDir.listFiles(pattern);

  if (indexFiles.size() == 0) {
    dialog_msgbox("Shaback recover", "\n\nNo version found to recover from.", 8, 50, 1);
    return false;
  }

  // for (set<string>::iterator it = setNames.begin(); it != setNames.end(); it++) {
  //   string setName(*it);
  //   cout << setName << endl;
  // }

  sort(indexFiles.begin(), indexFiles.end(), filePathComparator);
  int count = indexFiles.size();
  char** items = (char**) calloc(count + 1, 2 * sizeof(char*));

  int n = 0;
  for (vector<File>::iterator it = indexFiles.begin(); it != indexFiles.end(); it++) {
    File version(*it);
    
    items[n] = (char*) malloc(sizeof(char) * 10);
    sprintf(items[n], "%d", n/2);
    n++;

    items[n] = (char*) malloc(sizeof(char) * (1 + version.fname.size()));
    strcpy(items[n], version.fname.c_str());
    n++;
  }

  dlg_clr_result();
  int rc = dialog_menu("Shaback recovery", "Select backup version to recover from:", 0, 76, 0, count, (char **) items);
  if (rc == 0) {
    int sel = strtol(dialog_vars.input_result, 0, 10);
    indexFile = indexFiles[sel]; 
    freeItemList(items);
    return true;
  } else {
    freeItemList(items);
    return false;
  }
}
 #endif
