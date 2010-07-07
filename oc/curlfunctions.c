/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */


#include "ocinternal.h"
#include "ocdebug.h"
#include "ocdata.h"
#include "occontent.h"

#include "rc.h"

static char* combinecredentials(const char* user, const char* pwd);


/* Set various general curl flags */
int
ocset_curl_flags(CURL* curl,  OCstate* state)
{
    CURLcode cstat = CURLE_OK;
    struct OCcurlflags* flags = &state->curlflags;
#ifdef CURLOPT_ENCODING
    if (flags->compress) {
	cstat = curl_easy_setopt(curl, CURLOPT_ENCODING, 'deflate, gzip');
	if(cstat != CURLE_OK) goto fail;
    }
#endif
    if (flags->cookiejar || flags->cookiefile) {
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);
	if (cstat != CURLE_OK) goto fail;
    }
    if (flags->cookiejar) {
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIEJAR, flags->cookiejar);
	if (cstat != CURLE_OK) goto fail;
    }
    if (flags->cookiefile) {
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIEFILE, flags->cookiefile);
	if (cstat != CURLE_OK) goto fail;
    }
    if (flags->verbose) {
	cstat = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	if (cstat != CURLE_OK) goto fail;
    }

    /* Following are always set */
    cstat = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    return OC_NOERR;
fail:
    return OC_ECURL;
}

int
ocset_proxy(CURL* curl, OCstate* state)
{
    CURLcode cstat;
    struct OCproxy *proxy = &state->proxy;
    struct OCcredentials *creds = &state->creds;
    cstat = curl_easy_setopt(curl, CURLOPT_PROXY, proxy->host);
    if (cstat != CURLE_OK)
            return OC_ECURL;
    cstat = curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy->port);
    if (cstat != CURLE_OK)
            return OC_ECURL;
    if (creds->username) {
        char *combined = combinecredentials(creds->username,creds->password);
        if (!combined) return OC_ENOMEM;
        cstat = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, combined);
        free(combined);
        if (cstat != CURLE_OK) return OC_ECURL;
#ifdef CURLOPT_PROXYAUTH
        cstat = curl_easy_setopt(curl, CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);
        if(cstat != CURLE_OK) goto fail;
#endif
    }
    return OC_NOERR;
}

int
ocset_ssl(CURL* curl, OCstate* state)
{
    CURLcode cstat = CURLE_OK;
    struct OCSSL* ssl = &state->ssl;
    long verify = (ssl->validate?1L:0L);
    cstat=curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify);
    if (cstat != CURLE_OK) goto fail;
    cstat=curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify);
    if (cstat != CURLE_OK) goto fail;
    if(verify) {
        if(ssl->certificate) {
            cstat = curl_easy_setopt(curl, CURLOPT_SSLCERT, ssl->certificate);
            if(cstat != CURLE_OK) goto fail;
        }
        if(ssl->key) {
            cstat = curl_easy_setopt(curl, CURLOPT_SSLKEY, ssl->key);
            if(cstat != CURLE_OK) goto fail;
        }
        if(ssl->cainfo) {
            cstat = curl_easy_setopt(curl, CURLOPT_CAINFO, ssl->cainfo);
            if(cstat != CURLE_OK) goto fail;
        }
        if(ssl->capath) {
            cstat = curl_easy_setopt(curl, CURLOPT_CAPATH, ssl->capath);
            if(cstat != CURLE_OK) goto fail;
        }
    }    
    return OC_NOERR;

fail:
    return OC_ECURL;
}

/* This is called with arguments while the other functions in this file are
 * used with global values read from the.dodsrc file. The reason is that
 * we may have multiple password sources.
 */
int
ocset_user_password(CURL* curl, const char *userC, const char *passwordC)
{
    CURLcode cstat;
    char* combined = NULL;

    if(userC == NULL && passwordC == NULL) return OC_NOERR;
    if(userC == NULL) userC = "";
    if(passwordC == NULL) passwordC = "";

    combined = combinecredentials(userC,passwordC);
    if (!combined) return OC_ENOMEM;
    cstat = curl_easy_setopt(curl, CURLOPT_USERPWD, combined);
    if (cstat != CURLE_OK) goto done;
    cstat = curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long) CURLAUTH_ANY);
    if (cstat != CURLE_OK) goto done;

done:
    if(combined != NULL) free(combined);
    return (cstat == CURLE_OK?OC_NOERR:OC_ECURL);
}


static char*
combinecredentials(const char* user, const char* pwd)
{
    int userPassSize = strlen(user) + strlen(pwd) + 2;
    char *userPassword = malloc(sizeof(char) * userPassSize);
    if (!userPassword) {
        oc_log(LOGERR, "Out of Memory\n");
	return NULL;
    }
    strcpy(userPassword, user);
    strcat(userPassword, ":");
    strcat(userPassword, pwd);
    return userPassword;
}
