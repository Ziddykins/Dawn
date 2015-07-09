#include "include/parse.h"

//All groupers from the regular expression
//string will be stored here globally.
char regex_group[15][2048];

char *to_lower (char str[MAX_MESSAGE_BUFFER]) {
    for (size_t i=0; i<strlen(str); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }
    return str;
}

char *xor_flip (char * password) { //password -> 64
    unsigned int key = 0xDEADBEBE;
    for (size_t i=0; i<strlen(password); i++)
        password[i] ^= key;
    return password;
}

void genSalt(char * salt, size_t len) {
    size_t pos;
    for(pos = 0; pos < len-1; pos++) {
        do {
            salt[pos] = (char)(rand() % 256 - 128); //char -> (-128...127); mod 2^x does not skew random distribution
        } while(salt[pos] != 0);
    }
    salt[pos] = '\0';
}

void hashPwd(unsigned char * digest, char const * salt, char const * password) {
    size_t concatlen = strnlen(salt, 16) + strnlen(password, 64) + 2;
    char * concat = malloc(concatlen);
    if(!concat) {
        perror(ERR "parse/hashPwd: malloc");
        exit(1);
    }
    snprintf(concat, concatlen, "!%s%s", password, salt);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, concat, strlen(concat));
    SHA256_Final(digest, &sha256);
    free(concat);
}

int hashcmp(unsigned char const * s1, unsigned char const * s2) {
    int i;
    for(i = 0; i < SHA256_DIGEST_LENGTH && s1 && *s1 && s2 && *s2 && *s1 == *s2; i++) {
        s1++;
        s2++;
    }
    return i == SHA256_DIGEST_LENGTH;
}

/*char *nultrm (char str[MAX_MESSAGE_BUFFER]) {
    size_t len = strlen(str);
    str[len+1] = '\0';
    return str;
}*/

int check_if_matches_regex (char *buf, const char *regular_expression) {
    pcre *regex_compiled;
    pcre_extra *pcre_optimized;
    int pcre_return;
    int pcre_error_offset;
    int substring_vec[30];
    const char *psubStrMatchStr;
    const char *pcre_error;

    regex_compiled = pcre_compile(regular_expression, PCRE_MULTILINE,
                                  &pcre_error, &pcre_error_offset, NULL);

    if (regex_compiled == NULL) {
        fprintf(stderr, ERR "parse/check_if_matches_regex: Could not compile regular expression: %s (%s)\n",
                regular_expression, pcre_error);
        exit(1);
    }

    pcre_optimized = pcre_study(regex_compiled, 0, &pcre_error);

    if (pcre_error != NULL) {
        fprintf(stderr, ERR "parse/check_if_matches_regex: Could not optimize regular expression: %s (%s)\n", regular_expression, pcre_error);
        exit(1);
    }

    pcre_return = pcre_exec(regex_compiled, pcre_optimized, buf,
                            (int)strlen(buf), 0, PCRE_NOTEOL, substring_vec, 30);
    pcre_free(regex_compiled);
    pcre_free(pcre_optimized);

    if (pcre_return > 0) {
        int j;
        for (j=0; j<pcre_return; j++) {
            pcre_get_substring(buf, substring_vec, pcre_return, j, &(psubStrMatchStr));
            strcpy(regex_group[j], psubStrMatchStr);
            pcre_free_substring(psubStrMatchStr);
        }
        //We've found a match
        return 1;
    } else {
        //We've not found a match
        return 0;
    }
}

void handle_login (char *nick, char *pass, char *real, char *ident) {
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "NICK %s\r\n", nick);
    addMsg(out, strlen(out));
    sprintf(out, "USER %s * * :%s\r\n", ident, real);
    addMsg(out, strlen(out));
    printf(INFO "Using password '%s'\n", pass); //TODO:quieting down warnings for now but this will be nickserv
}

void parse_private_message (struct Bot *b, struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, message->sender_nick);

    if (pindex == -1) {
        sprintf(out, "PRIVMSG %s :You do not have an account\r\n", message->sender_nick);
        addMsg(out, strlen(out));
        return;
    }

    if (check_if_matches_regex(message->message, ";set password (\\w+)")) {
        if (strcmp(message->sender_hostmask, b->players[pindex].hostmask) == 0) {
            hashPwd(b->players[pindex].pwd, b->players[pindex].salt, regex_group[1]);
            sprintf(out, "PRIVMSG %s :Your password has been set\r\n", message->sender_nick);
        }
        addMsg(out, strlen(out));
    } else if (check_if_matches_regex(message->message, ";login (\\w+)")) {
        if (b->players[pindex].auth_level < AL_USER) {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            hashPwd(hash, b->players[pindex].salt, regex_group[1]);
            if (hashcmp(hash, b->players[pindex].pwd)) {
                strcpy(b->players[pindex].hostmask, message->sender_hostmask);
                snprintf(out,  MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has been verified. (USER)\r\n", b->active_room, message->sender_nick);
                addMsg(out, strlen(out));
                b->players[pindex].auth_level = AL_USER;
                snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Password correct\r\n", message->sender_nick);
            } else {
                snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Incorrect password\r\n", message->sender_nick);
            }
        } else {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :I already recognize you, no need to log in\r\n", message->sender_nick);
        }
        addMsg(out, strlen(out));
    }
}

//DEPRECATED
int command_allowed (struct Bot *b, char * command, int pindex) { //command -> 16
    if (b->players[pindex].travel_timer.active) {
        char disallowed[2][16] = {"melee", "gmelee"}; //remember to change array limits
        for (int i=0; i<2; i++) {
            printf("recv '%s' iter '%s'\n", command, disallowed[i]);
            if (strcmp(command, disallowed[i]) == 0) return 0;
        }
    }
    return 1;
}

void parse_room_message (struct Bot *b, struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    if (strcmp(message->message, ";new") == 0) {
        init_new_character(b, message);
    }

    //Check if a user has an account and is logged in from the correct host
    if (check_if_matches_regex(message->message, "^;(.*)")) {
        int pindex = get_pindex(b, message->sender_nick);

        //store command to check if user can execute it in current state
        check_if_matches_regex(message->message, "^;(.*?)\\s");

        if (pindex == -1) {
            sprintf(out, "PRIVMSG %s :Please create a new account by issuing ';new'\r\n", message->receiver);
            addMsg(out, strlen(out));
            return;
        }

        if (dawn->players[pindex].auth_level &&
                strcmp(dawn->players[pindex].hostmask, message->sender_hostmask) != 0) {
            sprintf(out, "PRIVMSG %s :%s, I've updated your hostmask\r\n", message->receiver, message->sender_nick);
            strcpy(dawn->players[pindex].hostmask, message->sender_hostmask);
            addMsg(out, strlen(out));
        }

        if (strcmp(b->players[pindex].hostmask, message->sender_hostmask) != 0
                && b->players[pindex].auth_level != 1) {
            sprintf(out, "PRIVMSG %s :%s, I seem to recall you connecting from a different host. Please login "
                    "by sending me a private message containing: ;login <your_password>\r\n",
                    message->receiver, message->sender_nick);
            addMsg(out, strlen(out));
            return;
        }

        if (!command_allowed(b, regex_group[1], pindex)) {
            sprintf(out, "PRIVMSG %s :This command cannot be used at this time\r\n",  message->receiver);
            addMsg(out, strlen(out));
            return;
        }
    }
}
