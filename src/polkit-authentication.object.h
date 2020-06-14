#ifndef OBJECT__H__POLKIT_AUTHENTICATION
#define OBJECT__H__POLKIT_AUTHENTICATION

typedef struct AuthenticationData AuthenticationData;

PolkitAuthenticationData * authentication_data_new();

void authentication_data_destroy(PolkitAuthenticationData * data);


#endif
