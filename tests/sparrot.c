#include "server/ssl/ssl_server.h"
#include "service/session.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util/logs.h"
#include "os/os.h"

const char* service = "sparrot";

const char* private_key =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC1DQSfaRQbFlQF\n"
    "ErQy31XknxaSjj+POmxsp8d6+1Oanl4K/u9VirnjsbyC7YB/GlyjkzMTRd20uUpJ\n"
    "t2MdA5I7orAxLSICwtdieTTaLq6/RQJuOAwk/re4vYJhV+N+QbtaX9wxcc+KOeGD\n"
    "Pamgdhw84sPDWAamzr7og35ecWtBiOrZzIZCKXEGi5Z2I28Vzc3vvcThaykD3MVZ\n"
    "JQKuwaqR4Os65/qLv32apm8zbBdpf2/gT73bmzUBYH54T7mYstGxPp/fgW5lmnJH\n"
    "2KPIqE+AW9U4A4Uy8Lcf8TOM0s/eSdakwHh0ZyszxkrZbwq0u0VYcKSdEvvCS9m+\n"
    "/SaSRAolAgMBAAECggEADqMzTPoPXJNFDrXw6K4CwT2VcxraC6fjDTPRXmGEZFtK\n"
    "aiQrUGSrW7vKbGFsQppsg/MKLZQ39P0IAwa3FijsQTMJgKhP+B4tkLIQ8lTU3vTi\n"
    "2ethG/qV+vMjX+BdrZ792TQn1r2HVHsSfuxU0vrJlKiWwCvVOdY8PAspg9QyojUf\n"
    "Mb6Kf0cV2LIcyU8nkSP0vdzvYkccgRnmq6IbQCfHBISx+WY0Aefx5QwSBAQZ1Sg0\n"
    "KuGMt6WBuIdWPVATXMUChde4sO4gLD7S4OgjJc+uuyGgOkx0G2hxnPrraHp+9F7m\n"
    "9MRFVJ5yTfMebZNNAcEbmARSIVTupk+t52Hrd54l7wKBgQDobISNrXeA+48QlvL4\n"
    "l0bYGHHGOQ9LfmGhnuRzSD5y4qdFQLrHjrCFmqizP81/ZIBF8WHDoOf20B6BGR1W\n"
    "WyvUlj8HTEOHY+1gIxsTuSEZjDEcSCpQYGAMAb38sRNd8RcuyWk1iPTyOp0eQU6i\n"
    "oiH3m1tH3CE+kATL6pgXv1/1BwKBgQDHanerxJJhsRK5jJKS11qvTxxkWgPxB9dJ\n"
    "0p0vy6K8LPTGXkcf/samZTbEhgKQ55hO9o2PGq4m7UA2mCt4Vmp+F0pc2Yy/llfc\n"
    "QrIDP1g4ygrp1f2qPGwLUWOJBiAOIx/9drylnAziZZWsNd2nlK7V6PVGD+m+ZHxV\n"
    "8AovAAlIcwKBgCxJwVhd3Y+uX6yBlvrXmKBEyClFZiy0gFYTuORet8eceNMxamXs\n"
    "QqayucKuPbIwrGCnhkGIv6rALY/cAUMbTFbN6mSxm4yI9gqDpf00IaKEnDcPaUC3\n"
    "nJWtH06vT0lvT0OTDYEt/77IfHfvadSDoIVWDSa8Q3lwZ/mTUjf0N4yxAoGBAMUG\n"
    "/JZJ8hdzzPxnV6qd/IuZroO4LIzJIn1eCtBecrtZ777PB5clM0DUX/hsL7zcFjBu\n"
    "ig8KXWc4omlOkCSlvTI38NEsYVQqR0To4Nk0jQYPGhlPxQGeEWJdV+raknFlYwYb\n"
    "euhl9pT5qZgs4IPi85uGQFCpCFmFDxQZxvYJIyLhAoGAEFAg177mB4KFSevGjZSr\n"
    "KJvgBOVPRBS3kp3ul22OEnm8okmzORhRqob3XAWV68Y01fq8EBSZWbg8Kp1dqeZh\n"
    "xBxG0o+9qQSQyN3c8R5cpR03SAlRtqsKvvgcTpUP7lLMOvLjsa+HMxZ4M22I4L2F\n"
    "3vDwn78806fOfkXXhZf0NVg=\n"
    "-----END PRIVATE KEY-----";

const char* public_key =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDbzCCAlegAwIBAgIUE35gdmkw5Wa9/VSUKItwtUIH7xEwDQYJKoZIhvcNAQEL\n"
    "BQAwRzELMAkGA1UEBhMCQlIxCzAJBgNVBAgMAlNQMRIwEAYDVQQHDAlTYW8gUGF1\n"
    "bG8xFzAVBgNVBAoMDm5lYmxpbmEgc2VydmVyMB4XDTI1MDkxOTIwMjgxOVoXDTI2\n"
    "MDkxOTIwMjgxOVowRzELMAkGA1UEBhMCQlIxCzAJBgNVBAgMAlNQMRIwEAYDVQQH\n"
    "DAlTYW8gUGF1bG8xFzAVBgNVBAoMDm5lYmxpbmEgc2VydmVyMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtQ0En2kUGxZUBRK0Mt9V5J8Wko4/jzpsbKfH\n"
    "evtTmp5eCv7vVYq547G8gu2Afxpco5MzE0XdtLlKSbdjHQOSO6KwMS0iAsLXYnk0\n"
    "2i6uv0UCbjgMJP63uL2CYVfjfkG7Wl/cMXHPijnhgz2poHYcPOLDw1gGps6+6IN+\n"
    "XnFrQYjq2cyGQilxBouWdiNvFc3N773E4WspA9zFWSUCrsGqkeDrOuf6i799mqZv\n"
    "M2wXaX9v4E+925s1AWB+eE+5mLLRsT6f34FuZZpyR9ijyKhPgFvVOAOFMvC3H/Ez\n"
    "jNLP3knWpMB4dGcrM8ZK2W8KtLtFWHCknRL7wkvZvv0mkkQKJQIDAQABo1MwUTAd\n"
    "BgNVHQ4EFgQUuCP6hROZ1UomkgLfBaGtpc4ZyMwwHwYDVR0jBBgwFoAUuCP6hROZ\n"
    "1UomkgLfBaGtpc4ZyMwwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC\n"
    "AQEAggXoGhBDw1bfGoNrZh3Fvsr607Kumcf6CmuqHj0LVCfjHmJG98a+CQ/n/I5b\n"
    "qHxWUZIshSkH634MvYdo/mgHj16v53rwvEM1XkoXwz4rrgqx/Ppf41UCinaC7TPh\n"
    "FA8xBc2PogyooROExtCgnnda3iEqUl90h8aJGvbDiHsldPYrPD3oFHGdeEGvTkwW\n"
    "O/RNL549x0p7XGIbgEwM2/zsjxipdK2/WqwLvnbUN4ej1FsQ33G5D/jeKs8s/z3S\n"
    "Cx/5ZM1g49J49g22BECacQOh8Jv72Sio/Q7ASmKICnKJOiD8+D3NzQrwGe6MEFit\n"
    "GJ/MWsOPtDo9dzr4d9qjLcrf7g==\n"
    "-----END CERTIFICATE-----";

typedef struct ParrotSession {
    Session session;
} ParrotSession;

static int parrot_on_recv(Session* session, CommunicationBuffer* c)
{
    (void) session;

    char* line;
    while ((commbuf_extract_line_from_recv_buffer(c, &line, "\r\n"))) {
        commbuf_add_text_to_send_buffer(c, line);
        free(line);
    }

    return 0;
}

static Session* parrot_session_create(SOCKET fd, void* data)
{
    (void) data;

    ParrotSession* psession = calloc(1, sizeof(ParrotSession));
    session_init(&psession->session, fd, parrot_on_recv, NULL);
    return (Session *) psession;
}

int main(int argc, char* argv[])
{
    if (argc == 2 && strcmp(argv[1], "-d") == 0)
        logs_verbose = true;
    os_handle_ctrl_c();

    SSLKey key = { .private_key = private_key, .public_key = public_key };
    Server* server = (Server *) ssl_server_create(23457, false, parrot_session_create, 0, &key);
    if (!server) {
        perror("ssl_create_socket");
        return EXIT_FAILURE;
    }

    server_run(server);
    server_destroy(server);
}
