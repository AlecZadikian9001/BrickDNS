//
//  sha256.c
//  AgreementFinderServer
//
//  Created by Alec Zadikian on 6/7/14.
//  Copyright (c) 2014 AlecZ. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>
#include "general.h"

char* hash(char *str, char* key){
    size_t inputLen = strlen(str);
    size_t keyLen = strlen(key);
    char* ret = emalloc(65*sizeof(char));
    unsigned char* hash = HMAC(EVP_sha256(), key, keyLen, (unsigned char*)str, inputLen, NULL, NULL);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(ret + (i * 2), "%02x", hash[i]);
    }
    ret[65] = '\0';
    return ret;
}