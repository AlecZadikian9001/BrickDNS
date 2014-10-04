//
//  wordsDatabase.h
//  BrickDNS
//
//  Created by Alec Zadikian on 10/4/14.
//  Copyright (c) 2014 AlecZ. All rights reserved.
//

#ifndef __BrickDNS__wordsDatabase__
#define __BrickDNS__wordsDatabase__

#include <stdio.h>
#include "general.h"
#include "database.h"

int loadWordList(sqlite3* db, int* longestWord);

#endif /* defined(__BrickDNS__wordsDatabase__) */
