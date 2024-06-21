#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <sapi/embed/php_embed.h>
#define PHP_PAM_VERSION "0.1.0"

static int le_pam_handle;

ZEND_BEGIN_ARG_INFO_EX(arginfo_get_user, 0, 0, 1)
ZEND_ARG_INFO(0, pamh)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_get_rhost, 0, 0, 1)
ZEND_ARG_INFO(0, pamh)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ask_question, 0, 0, 2)
ZEND_ARG_INFO(0, pamh)
ZEND_ARG_INFO(0, question)
ZEND_END_ARG_INFO()

PHP_FUNCTION(get_user)
{
  zval *zpam_resource;
  pam_handle_t *pamh;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &zpam_resource) == FAILURE) {
    RETURN_FALSE;
  }

  pamh = (pam_handle_t *)Z_RES_P(zpam_resource);
  if (!pamh) {
    RETURN_FALSE;
  }

  const char *user = NULL;
  int result       = pam_get_user(pamh, &user, NULL);
  if (result != PAM_SUCCESS || user == NULL) {
    RETURN_FALSE;
  }

  RETURN_STRING(user);
}

PHP_FUNCTION(get_rhost)
{
  zval *zpam_resource;
  pam_handle_t *pamh;
  const void *rhost;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &zpam_resource) == FAILURE) {
    RETURN_FALSE;
  }

  pamh = (pam_handle_t *)Z_RES_P(zpam_resource);
  if (!pamh) {
    RETURN_FALSE;
  }

  int result = pam_get_item(pamh, PAM_RHOST, &rhost);
  if (result != PAM_SUCCESS || rhost == NULL) {
    RETURN_FALSE;
  }

  RETURN_STRING((const char *)rhost);
}

int conversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
  struct pam_response *response = (struct pam_response *)calloc(num_msg, sizeof(struct pam_response));
  if (!response)
    return PAM_CONV_ERR;

  for (int i = 0; i < num_msg; i++) {
    switch (msg[i]->msg_style) {
    case PAM_PROMPT_ECHO_ON:
    case PAM_PROMPT_ECHO_OFF:
      printf("%s", msg[i]->msg);
      response[i].resp = (char *)malloc(PAM_MAX_RESP_SIZE);
      if (fgets(response[i].resp, PAM_MAX_RESP_SIZE, stdin) == NULL) {
        free(response);
        return PAM_CONV_ERR;
      }
      response[i].resp_retcode = 0;
      break;
    case PAM_ERROR_MSG:
      fprintf(stderr, "%s\n", msg[i]->msg);
      break;
    case PAM_TEXT_INFO:
      printf("%s\n", msg[i]->msg);
      break;
    default:
      fprintf(stderr, "Unknown message style %d\n", msg[i]->msg_style);
    }
  }

  *resp = response;
  return PAM_SUCCESS;
}

struct pam_conv conv = {conversation, NULL};

PHP_FUNCTION(ask_question)
{
  zval *zpam_resource;
  char *question;
  size_t question_len;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "zs", &zpam_resource, &question, &question_len) == FAILURE) {
    RETURN_FALSE;
  }

  pam_handle_t *pamh = (pam_handle_t *)Z_RES_P(zpam_resource);
  if (!pamh) {
    RETURN_FALSE;
  }

  const struct pam_message msg      = {PAM_PROMPT_ECHO_ON, // Echo the input
                                       question};
  const struct pam_message *msgs[1] = {&msg};
  struct pam_response *resp         = NULL;

  const struct pam_conv *conv = NULL;
  pam_get_item(pamh, PAM_CONV, (const void **)&conv);

  int retval = conv->conv(1, msgs, &resp, conv->appdata_ptr);
  if (retval != PAM_SUCCESS || !resp || !resp->resp) {
    if (resp) {
      if (resp->resp)
        free(resp->resp);
      free(resp);
    }
    RETURN_FALSE;
  }

  RETVAL_STRING(resp->resp);
  free(resp->resp);
  free(resp);
}

const zend_function_entry custom_functions[] = {PHP_FE(get_user, arginfo_get_user) PHP_FE(
    ask_question, arginfo_ask_question) PHP_FE(get_rhost, arginfo_get_rhost) PHP_FE_END};

int call_php_handler(pam_handle_t *pamh, const char *filename, const char *cfunction_name)
{
  int ret = PAM_AUTH_ERR;
  PHP_EMBED_START_BLOCK(0, NULL)
  zval retval, func_name;
  zend_file_handle file_handle;
  zend_string *php_filename;
  zval args[1];

  zend_stream_init_filename(&file_handle, filename);
  if (php_execute_script(&file_handle) == FAILURE) {
    return PAM_AUTH_ERR;
  }

  ZVAL_STRING(&func_name, cfunction_name);
  ZVAL_PTR(&args[0], pamh);

  zend_register_functions(NULL, custom_functions, NULL, MODULE_PERSISTENT);
  if (call_user_function(CG(function_table), NULL, &func_name, &retval, 1, args) == SUCCESS && retval.value.lval == 0) {
    ret = PAM_SUCCESS;
  } else {
    pam_syslog(NULL, LOG_ERR, "Failed to call PHP function %s", cfunction_name);
  }
  zend_unregister_functions(custom_functions, 1, NULL);

  zval_ptr_dtor(&func_name);
  zval_ptr_dtor(&args[0]);
  if (Z_TYPE(retval) != IS_UNDEF) {
    zval_ptr_dtor(&retval);
  }

  PHP_EMBED_END_BLOCK();
  return ret;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  const char *filename = NULL;

  if (argc > 0) {
    filename = argv[0];
  } else {
    pam_syslog(pamh, LOG_ERR, "No PHP file specified");
    return PAM_AUTH_ERR;
  }

  return call_php_handler(pamh, filename, "pam_authenticate");
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return PAM_SUCCESS;
}
