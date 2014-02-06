/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pwd.h>

struct passwd *
getpwnam (const char *name)
{
    if (strcmp(name, "testuser") != 0)
        return 0;

    static struct passwd user_passwd = {0};
    user_passwd.pw_name = "testuser";
    user_passwd.pw_uid = 12345;
    user_passwd.pw_gid = 12345;

    return &user_passwd;
}

uid_t getuid ()
{
    return 12345;
}
